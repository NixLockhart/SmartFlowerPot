/**
 * @file    app_main.c
 * @brief   应用主模块实现
 * @version 2.0
 */

#include "app_main.h"
#include "../Config/device_config.h"
#include "../Config/sensor_manager.h"
#include "../Config/control_manager.h"
#include "../Config/threshold_engine.h"
#include "../Server/server_client.h"
#include "../Server/msg_handler.h"
#include "../Server/data_reporter.h"
#include "../WiFi/wifi.h"
#include "../../HARDWARE/DHT11/dht11.h"
#include "../../HARDWARE/BUMP/bump.h"
#include <string.h>

/* 传感器读取间隔 (毫秒) */
#define SENSOR_READ_INTERVAL    1000

/* 阈值检查间隔 (毫秒) */
#define THRESHOLD_CHECK_INTERVAL 2000

/* 外部时间函数 */
extern uint32_t get_tick_ms(void);

/* 内部状态 */
static struct {
    uint32_t last_sensor_read;
    uint32_t last_threshold_check;
    bool initialized;
} s_app = {0};

/* 硬件控制执行函数 */
static void hardware_control_executor(control_item_t item, uint8_t state)
{
    switch (item) {
        case CTRL_PUMP:
            /* 控制水泵 */
            if (state) {
                BUMP_ON();  /* 假设这是水泵控制 */
            } else {
                BUMP_OFF();
            }
            break;

        case CTRL_FAN:
            /* 控制风扇 - 需要根据实际硬件实现 */
            break;

        case CTRL_LIGHT:
            /* 控制补光灯 - 需要根据实际硬件实现 */
            break;

        case CTRL_HEATER:
            /* 控制加热器 - 需要根据实际硬件实现 */
            break;

        default:
            break;
    }
}

/* 服务器状态回调 */
static void server_state_callback(client_state_t old_state, client_state_t new_state)
{
    /* 可以在这里更新UI显示连接状态 */
    if (new_state == CLIENT_STATE_REGISTERED) {
        /* 连接成功，立即上报一次数据 */
        data_reporter_force_report();
    }
}

/* 控制命令回调 */
static void control_command_callback(const char *key, uint8_t state)
{
    /* 执行控制命令 */
    control_manager_set_by_key(key, state);
}

/* 配置更新回调 */
static void config_update_callback(const char *config_json)
{
    /* 更新本地配置 */
    device_config_update_from_json(config_json);

    /* 更新数据上报间隔 */
    const device_config_t *config = device_config_get();
    data_reporter_set_interval(config->report_interval);
}

/* 传感器读取函数 */
static int32_t read_soil_moisture(void)
{
    /* 读取土壤湿度 ADC 值并转换为百分比 */
    /* 需要根据实际硬件实现 */
    return 50;  /* 示例值 */
}

static int32_t read_temperature(void)
{
    /* 读取 DHT11 温度 (x10) */
    uint8_t temp = 0, humi = 0;
    if (DHT11_Read_Data(&temp, &humi) == 0) {
        return temp * 10;
    }
    return 250;  /* 默认 25.0°C */
}

static int32_t read_humidity(void)
{
    /* 读取 DHT11 湿度 */
    uint8_t temp = 0, humi = 0;
    if (DHT11_Read_Data(&temp, &humi) == 0) {
        return humi;
    }
    return 50;  /* 默认 50% */
}

static int32_t read_light(void)
{
    /* 读取光照传感器 */
    /* 需要根据实际硬件实现 */
    return 500;  /* 示例值 */
}

/* ===== 公共接口 ===== */

void app_init(void)
{
    const device_config_t *config;
    client_config_t client_cfg;

    if (s_app.initialized) {
        return;
    }

    /* 初始化配置模块 (从EEPROM加载) */
    device_config_init();
    config = device_config_get();

    /* 初始化传感器管理器 */
    sensor_manager_init();

    /* 初始化控制管理器 */
    control_manager_init();
    control_manager_set_executor(hardware_control_executor);

    /* 初始化阈值引擎 */
    threshold_engine_init();

    /* 初始化消息处理器 */
    msg_handler_init();
    msg_handler_set_device_id(config->device_id);
    msg_handler_set_control_callback(control_command_callback);
    msg_handler_set_config_callback(config_update_callback);

    /* 配置服务器客户端 */
    memset(&client_cfg, 0, sizeof(client_cfg));
    strncpy(client_cfg.server_ip, config->server_ip, sizeof(client_cfg.server_ip) - 1);
    client_cfg.server_port = config->server_port;
    strncpy(client_cfg.device_id, config->device_id, sizeof(client_cfg.device_id) - 1);
    strncpy(client_cfg.user_id, config->user_id, sizeof(client_cfg.user_id) - 1);
    strcpy(client_cfg.firmware_version, "2.0");
    client_cfg.reconnect_interval = 10;
    client_cfg.max_reconnect_attempts = 0;  /* 无限重连 */

    /* 初始化服务器客户端 */
    server_client_init(&client_cfg);
    server_client_set_state_callback(server_state_callback);

    /* 初始化数据上报器 */
    data_reporter_init();
    data_reporter_set_interval(config->report_interval);

    s_app.initialized = true;
}

void app_process(void)
{
    uint32_t now = get_tick_ms();

    if (!s_app.initialized) {
        return;
    }

    /* 处理服务器通信 */
    server_client_process();

    /* 定期读取传感器 */
    if (now - s_app.last_sensor_read >= SENSOR_READ_INTERVAL) {
        s_app.last_sensor_read = now;
        sensor_manager_read_all();
    }

    /* 处理数据上报 */
    data_reporter_process();

    /* 定期检查阈值 (仅自动模式) */
    if (now - s_app.last_threshold_check >= THRESHOLD_CHECK_INTERVAL) {
        s_app.last_threshold_check = now;
        threshold_engine_process();
    }
}

void app_register_sensors(void)
{
    /* 注册传感器读取函数 */
    sensor_manager_register_reader(SENSOR_SOIL_MOISTURE, read_soil_moisture);
    sensor_manager_register_reader(SENSOR_TEMPERATURE, read_temperature);
    sensor_manager_register_reader(SENSOR_HUMIDITY, read_humidity);
    sensor_manager_register_reader(SENSOR_LIGHT, read_light);
}

int app_connect_server(void)
{
    return server_client_connect();
}

void app_disconnect_server(void)
{
    server_client_disconnect();
}

bool app_is_server_ready(void)
{
    return server_client_is_ready();
}

void app_report_data(void)
{
    data_reporter_force_report();
}

void app_set_mode(uint8_t mode)
{
    device_config_set_work_mode(mode);

    /* 切换到手动模式时禁用阈值引擎 */
    threshold_engine_enable(mode == 0);
}

uint8_t app_get_mode(void)
{
    const device_config_t *config = device_config_get();
    return config ? config->work_mode : 0;
}

int app_manual_control(const char *key, uint8_t state)
{
    /* 手动模式下才允许手动控制 */
    if (app_get_mode() == 0) {
        return -1;  /* 自动模式不允许手动控制 */
    }

    return control_manager_set_by_key(key, state);
}
