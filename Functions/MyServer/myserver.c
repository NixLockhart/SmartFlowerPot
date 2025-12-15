/**
 ****************************************************************************************************
 * @file        myserver.c
 * @author      NixStudio(NixLockhart)
 * @version     V2.0
 * @date        2025-12-14
 * @brief       自定义服务器通信模块实现 - 支持动态配置
 ****************************************************************************************************
 * @attention
 *
 * 平台: 正点原子 STM32F103开发板
 * 服务器: 自定义TCP服务器 111.228.6.160:8003
 * 协议: JSON over TCP
 *
 * V2.0 更新:
 * - 添加动态传感器配置支持
 * - 添加动态控制项配置支持
 * - 动态JSON数据构建
 *
 ****************************************************************************************************
 */

#include "myserver.h"
#include "atk_mw8266d.h"
#include "atk_mw8266d_uart.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "bump.h"
#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************/
/* 全局变量 */

uint8_t g_my_wifi_status = MY_WIFI_DISCONNECTED;        /* WiFi连接状态 */
uint8_t g_my_server_status = MY_SERVER_DISCONNECTED;    /* 服务器连接状态 */
my_received_cmd_t g_received_cmd = {0};                 /* 接收到的命令 */
my_device_config_t g_device_config = {0};               /* 设备动态配置 (V2.0新增) */

/******************************************************************************************/
/* 私有变量 */

static uint32_t s_heartbeat_timer = 0;      /* 心跳计时器 */
static uint32_t s_msg_seq = 0;              /* 消息序列号 */
/* 接收到的命令结构 - 增加字符串ID存储 */
static char s_cmd_id_str[64];               /* 命令ID字符串 (用于ACK响应) - UUID长度为36字符 */
static char s_send_buf[512];                /* 发送缓冲区 - 增大以容纳完整的ACK消息 */
static char s_recv_buf[512];                /* 接收缓冲区 - 增大以容纳完整的控制命令 */

/******************************************************************************************/
/* 私有函数声明 */

static uint8_t send_json_message(const char *json);
static uint8_t parse_json_command(const char *json);
static int find_json_int(const char *json, const char *key);
static char* find_json_string(const char *json, const char *key, char *out, uint8_t max_len);

/******************************************************************************************/
/* 初始化与连接 */

/**
 * @brief  初始化WiFi模块
 */
uint8_t myserver_wifi_init(void)
{
    uint8_t ret;

    ret = atk_mw8266d_init(115200);
    if (ret != 0)
    {
        printf("[MyServer] WiFi module init failed!\r\n");
        g_my_wifi_status = MY_WIFI_ERROR;
        return 1;
    }

    printf("[MyServer] WiFi module init OK\r\n");
    return 0;
}

/**
 * @brief  连接WiFi路由器
 */
uint8_t myserver_wifi_connect(void)
{
    uint8_t ret = 0;
    char ip_buf[16];

    printf("[MyServer] Connecting to WiFi: %s\r\n", MY_WIFI_SSID);

    ret  = atk_mw8266d_restore();                                   /* 恢复出厂设置 */
    ret += atk_mw8266d_at_test();                                   /* AT测试 */
    ret += atk_mw8266d_set_mode(1);                                 /* Station模式 */
    ret += atk_mw8266d_sw_reset();                                  /* 软件复位 */
    ret += atk_mw8266d_ate_config(0);                               /* 关闭回显 */
    ret += atk_mw8266d_join_ap(MY_WIFI_SSID, MY_WIFI_PWD);          /* 连接WiFi */
    ret += atk_mw8266d_get_ip(ip_buf);                              /* 获取IP地址 */

    if (ret != 0)
    {
        printf("[MyServer] WiFi connect failed!\r\n");
        g_my_wifi_status = MY_WIFI_ERROR;
        return ret;
    }

    printf("[MyServer] WiFi connected! IP: %s\r\n", ip_buf);
    g_my_wifi_status = MY_WIFI_CONNECTED;
    atk_mw8266d_uart_rx_restart();

    return 0;
}

/**
 * @brief  连接到自定义服务器
 */
uint8_t myserver_connect(void)
{
    uint8_t ret;

    if (g_my_wifi_status != MY_WIFI_CONNECTED)
    {
        printf("[MyServer] WiFi not connected!\r\n");
        return 1;
    }

    printf("[MyServer] Connecting to server %s:%s\r\n", MY_SERVER_IP, MY_SERVER_PORT);

    /* 连接TCP服务器 */
    ret = atk_mw8266d_connect_tcp_server(MY_SERVER_IP, MY_SERVER_PORT);
    if (ret != 0)
    {
        printf("[MyServer] Server connect failed!\r\n");
        g_my_server_status = MY_SERVER_DISCONNECTED;
        return 1;
    }

    /* 进入透传模式 */
    ret = atk_mw8266d_enter_unvarnished();
    if (ret != 0)
    {
        printf("[MyServer] Enter unvarnished mode failed!\r\n");
        g_my_server_status = MY_SERVER_DISCONNECTED;
        return 1;
    }

    printf("[MyServer] Server connected!\r\n");
    g_my_server_status = MY_SERVER_CONNECTED;

    /* 发送注册消息 */
    delay_ms(100);
    myserver_send_register();

    return 0;
}

/**
 * @brief  断开服务器连接
 */
uint8_t myserver_disconnect(void)
{
    atk_mw8266d_exit_unvarnished();
    delay_ms(100);
    atk_mw8266d_send_at_cmd("AT+CIPCLOSE", "OK", 500);

    g_my_server_status = MY_SERVER_DISCONNECTED;
    printf("[MyServer] Server disconnected\r\n");

    return 0;
}

/**
 * @brief  重新连接服务器
 */
uint8_t myserver_reconnect(void)
{
    myserver_disconnect();
    delay_ms(500);
    return myserver_connect();
}

/******************************************************************************************/
/* 数据发送 */

/**
 * @brief  发送设备注册信息 (V2.0协议格式)
 * @note   用户ID用于将设备绑定到指定用户账号
 */
uint8_t myserver_send_register(void)
{
    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"reg","p":{"d":"设备ID","u":"用户ID","ver":"固件版本"}} */
    snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"reg\",\"p\":{\"d\":\"%s\",\"u\":\"%s\",\"ver\":\"2.0\"}}\n",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000), MY_DEVICE_ID, MY_USER_ID);

    return send_json_message(s_send_buf);
}

/**
 * @brief  发送心跳包 (V2.0协议格式)
 */
uint8_t myserver_send_heartbeat(void)
{
    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"hb","d":"设备ID","p":{}} */
    snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"hb\",\"d\":\"%s\",\"p\":{}}\n",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000), MY_DEVICE_ID);

    return send_json_message(s_send_buf);
}

/**
 * @brief  发送传感器数据 (V2.0协议格式)
 */
uint8_t myserver_send_sensor_data(my_sensor_data_t *data)
{
    if (data == NULL) return 1;

    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"dat","d":"设备ID","p":{传感器数据}} */
    snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"dat\",\"d\":\"%s\","
        "\"p\":{\"temp\":%d,\"humi\":%d,\"soil\":%d,\"light\":%d}}\n",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000), MY_DEVICE_ID,
        data->temperature, data->humidity, data->soil_humidity, data->light_intensity);

    return send_json_message(s_send_buf);
}

/**
 * @brief  发送设备状态 (V2.0协议格式)
 */
uint8_t myserver_send_device_status(my_device_status_t *status)
{
    if (status == NULL) return 1;

    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"sta","d":"设备ID","p":{状态数据}} */
    snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"sta\",\"d\":\"%s\","
        "\"p\":{\"mode\":%d,\"light\":%d,\"water\":%d,\"fan\":%d}}\n",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000), MY_DEVICE_ID,
        status->mode, status->light_status, status->water_status, status->fan_status);

    return send_json_message(s_send_buf);
}

/**
 * @brief  发送命令确认 (V2.0协议格式)
 * @note   使用原始命令的字符串ID作为ref字段，以便服务器匹配
 */
uint8_t myserver_send_ack(uint32_t cmd_id, uint8_t success)
{
    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"ack","d":"设备ID","p":{"ref":"原始命令ID","ok":1}} */
    /* 使用保存的字符串ID作为ref字段 */
    snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"ack\",\"d\":\"%s\","
        "\"p\":{\"ref\":\"%s\",\"ok\":%d}}\n",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000), MY_DEVICE_ID,
        s_cmd_id_str, success ? 1 : 0);

    return send_json_message(s_send_buf);
}

/******************************************************************************************/
/* 数据接收 */

/**
 * @brief  接收并解析服务器消息
 */
my_cmd_type_t myserver_receive_command(void)
{
    uint8_t *buf;

    if (g_my_server_status != MY_SERVER_CONNECTED)
    {
        return CMD_NONE;
    }

    buf = atk_mw8266d_uart_rx_get_frame();
    if (buf == NULL)
    {
        return CMD_NONE;
    }

    /* 复制到接收缓冲区 */
    strncpy(s_recv_buf, (char *)buf, sizeof(s_recv_buf) - 1);
    s_recv_buf[sizeof(s_recv_buf) - 1] = '\0';

    printf("[MyServer] Received: %s\r\n", s_recv_buf);

    /* 解析JSON命令 */
    if (parse_json_command(s_recv_buf) == 0)
    {
        atk_mw8266d_uart_rx_restart();
        return g_received_cmd.type;
    }

    atk_mw8266d_uart_rx_restart();
    return CMD_NONE;
}

/**
 * @brief  获取接收到的阈值设置
 */
uint8_t myserver_get_threshold(my_threshold_t *threshold)
{
    if (threshold == NULL) return 1;

    memcpy(threshold, &g_received_cmd.threshold, sizeof(my_threshold_t));
    return 0;
}

/******************************************************************************************/
/* 状态检查 */

/**
 * @brief  检查WiFi连接状态
 */
uint8_t myserver_check_wifi(void)
{
    return g_my_wifi_status;
}

/**
 * @brief  检查服务器连接状态
 */
uint8_t myserver_check_server(void)
{
    return g_my_server_status;
}

/******************************************************************************************/
/* 主处理函数 */

/**
 * @brief  服务器通信处理 (在主循环中周期调用)
 */
void myserver_process(void)
{
    /* 心跳处理 - 每10秒发送一次 */
    s_heartbeat_timer++;
    if (s_heartbeat_timer >= 200)  /* 200 * 50ms = 10s */
    {
        s_heartbeat_timer = 0;
        if (g_my_server_status == MY_SERVER_CONNECTED)
        {
            myserver_send_heartbeat();
        }
    }
}

/******************************************************************************************/
/* 私有函数实现 */

/**
 * @brief  发送JSON消息
 */
static uint8_t send_json_message(const char *json)
{
    if (g_my_server_status != MY_SERVER_CONNECTED)
    {
        printf("[MyServer] Not connected, cannot send!\r\n");
        return 1;
    }

    atk_mw8266d_uart_printf("%s", json);
    printf("[MyServer] Sent: %s", json);

    return 0;
}

/**
 * @brief  解析JSON命令 (V2.0协议格式)
 * @note   V2.0协议使用精简字段名: t=类型, p=载荷, k=控制项, s=状态
 *         控制命令格式: {"v":"1.0","id":"xxx","ts":123,"t":"ctl","d":"SFP_001","p":{"k":"light","s":1}}
 */
static uint8_t parse_json_command(const char *json)
{
    char type_buf[16];
    char key_buf[16];
    int state;

    /* 清空命令结构 */
    memset(&g_received_cmd, 0, sizeof(g_received_cmd));

    /* V2.0协议: 获取消息类型 "t" */
    if (find_json_string(json, "t", type_buf, sizeof(type_buf)) == NULL)
    {
        /* 尝试旧协议格式 "type" (向后兼容) */
        if (find_json_string(json, "type", type_buf, sizeof(type_buf)) == NULL)
        {
            return 1;
        }
    }

    /* 获取消息ID "id" (用于ACK响应) - 保存完整的字符串ID */
    memset(s_cmd_id_str, 0, sizeof(s_cmd_id_str));
    if (find_json_string(json, "id", s_cmd_id_str, sizeof(s_cmd_id_str)) != NULL)
    {
        g_received_cmd.cmd_id = (uint32_t)atoi(s_cmd_id_str);
    }

    /* 解析V2.0协议控制命令: t="ctl" */
    if (strcmp(type_buf, "ctl") == 0)
    {
        /* 获取控制项 "k" 和状态 "s" - 这些字段在 payload "p" 内部 */
        /* find_json_string 会在整个 JSON 中搜索，应该能找到嵌套的字段 */
        if (find_json_string(json, "k", key_buf, sizeof(key_buf)) != NULL)
        {
            state = find_json_int(json, "s");

            printf("[MyServer] Parsed control: key=%s, state=%d\r\n", key_buf, state);

            /* 根据控制项和状态设置命令类型 */
            if (strcmp(key_buf, "light") == 0)
            {
                g_received_cmd.type = (state == 1) ? CMD_LIGHT_ON : CMD_LIGHT_OFF;
            }
            else if (strcmp(key_buf, "water") == 0)
            {
                g_received_cmd.type = (state == 1) ? CMD_WATER_ON : CMD_WATER_OFF;
            }
            else if (strcmp(key_buf, "fan") == 0)
            {
                g_received_cmd.type = (state == 1) ? CMD_FAN_ON : CMD_FAN_OFF;
            }
            else if (strcmp(key_buf, "mode") == 0)
            {
                g_received_cmd.type = (state == 1) ? CMD_MODE_MANUAL : CMD_MODE_AUTO;
            }
            else
            {
                printf("[MyServer] Unknown control key: %s\r\n", key_buf);
                return 1;  /* 未知控制项 */
            }
        }
        else
        {
            return 1;  /* 缺少控制项 */
        }
    }
    /* 解析V2.0协议功能操作: t="act" */
    else if (strcmp(type_buf, "act") == 0)
    {
        if (find_json_string(json, "k", key_buf, sizeof(key_buf)) != NULL)
        {
            if (strcmp(key_buf, "get_status") == 0)
                g_received_cmd.type = CMD_GET_STATUS;
            else if (strcmp(key_buf, "reboot") == 0)
                g_received_cmd.type = CMD_REBOOT;
            else
                return 1;
        }
    }
    /* 解析V2.0协议配置同步: t="cfg" */
    else if (strcmp(type_buf, "cfg") == 0)
    {
        /* 阈值配置在payload中 */
        g_received_cmd.type = CMD_SET_THRESHOLD;
        /* 尝试解析阈值数据 (简化处理，服务器会发送完整配置) */
        g_received_cmd.threshold.temp_upper = find_json_int(json, "temp_upper");
        g_received_cmd.threshold.temp_lower = find_json_int(json, "temp_lower");
        g_received_cmd.threshold.humi_upper = find_json_int(json, "humi_upper");
        g_received_cmd.threshold.humi_lower = find_json_int(json, "humi_lower");
        g_received_cmd.threshold.soil_upper = find_json_int(json, "soil_upper");
        g_received_cmd.threshold.soil_lower = find_json_int(json, "soil_lower");
        g_received_cmd.threshold.light_upper = find_json_int(json, "light_upper");
        g_received_cmd.threshold.light_lower = find_json_int(json, "light_lower");
    }
    /* 解析心跳响应: t="hb_ok" */
    else if (strcmp(type_buf, "hb_ok") == 0)
    {
        /* 心跳响应，无需特殊处理 */
        return 0;
    }
    /* 解析注册响应: t="reg_ok" */
    else if (strcmp(type_buf, "reg_ok") == 0)
    {
        printf("[MyServer] Registration confirmed by server\r\n");
        return 0;
    }
    /* 解析错误响应: t="err" 或 t="reg_err" */
    else if (strcmp(type_buf, "err") == 0 || strcmp(type_buf, "reg_err") == 0)
    {
        printf("[MyServer] Server error received\r\n");
        return 0;
    }
    else
    {
        return 1;
    }

    return 0;
}

/**
 * @brief  从JSON中查找整数值 (简易解析)
 */
static int find_json_int(const char *json, const char *key)
{
    char search[32];
    char *pos;

    snprintf(search, sizeof(search), "\"%s\":", key);
    pos = strstr(json, search);
    if (pos == NULL) return -1;

    pos += strlen(search);
    while (*pos == ' ') pos++;

    return atoi(pos);
}

/**
 * @brief  从JSON中查找字符串值
 */
static char* find_json_string(const char *json, const char *key, char *out, uint8_t max_len)
{
    char search[32];
    char *pos, *end;
    uint8_t len;

    snprintf(search, sizeof(search), "\"%s\":", key);
    pos = strstr(json, search);
    if (pos == NULL) return NULL;

    pos += strlen(search);

    /* 跳过冒号后的空格 */
    while (*pos == ' ') pos++;

    /* 检查是否是字符串值（以引号开始） */
    if (*pos != '"') return NULL;
    pos++;  /* 跳过开始的引号 */

    end = strchr(pos, '"');
    if (end == NULL) return NULL;

    len = end - pos;
    if (len >= max_len) len = max_len - 1;

    strncpy(out, pos, len);
    out[len] = '\0';

    return out;
}

/******************************************************************************************/
/* 无线控制处理 */

/**
 * @brief  处理服务器下发的无线控制命令
 */
void myserver_wireless_control(void)
{
    my_cmd_type_t cmd = myserver_receive_command();

    /* 自动模式下，设备控制命令被忽略，只响应模式切换和状态查询 */
    if (!mode && cmd != CMD_NONE) {
        /* 自动模式下允许的命令 */
        if (cmd != CMD_MODE_AUTO && cmd != CMD_MODE_MANUAL &&
            cmd != CMD_GET_STATUS && cmd != CMD_SET_THRESHOLD && cmd != CMD_REBOOT) {
            /* 其他控制命令在自动模式下忽略，但仍发送ACK */
            myserver_send_ack(g_received_cmd.cmd_id, 0);  /* 0表示未执行 */
            return;
        }
    }

    switch (cmd) {
        case CMD_LIGHT_ON:
            light_status = 1;
            LED1 = 0;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Light ON", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_LIGHT_OFF:
            light_status = 0;
            LED1 = 1;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Light OFF", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_WATER_ON:
            water_status = 1;
            BUMP_ON;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Water ON", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_WATER_OFF:
            water_status = 0;
            BUMP_OFF;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Water OFF", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_FAN_ON:
            fun_status = 1;
            FUN_ON;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Fan ON", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_FAN_OFF:
            fun_status = 0;
            FUN_OFF;
            if (get_current_screen() == SCREEN_MANUAL) {
                update_manual_screen();
            }
            create_popup();
            show_popup("Fan OFF", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_MODE_AUTO:
            mode = 0;
            if (get_current_screen() == SCREEN_MAIN) {
                update_main_screen();
            } else if (get_current_screen() == SCREEN_MENU) {
                update_menu_screen();
            }
            create_popup();
            show_popup("Auto Mode", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_MODE_MANUAL:
            mode = 1;
            if (get_current_screen() == SCREEN_MAIN) {
                update_main_screen();
            } else if (get_current_screen() == SCREEN_MENU) {
                update_menu_screen();
            }
            create_popup();
            show_popup("Manual Mode", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_SET_THRESHOLD:
            /* 更新阈值设置 */
            lim_value.temp_upper = g_received_cmd.threshold.temp_upper;
            lim_value.temp_lower = g_received_cmd.threshold.temp_lower;
            lim_value.humi_upper = g_received_cmd.threshold.humi_upper;
            lim_value.humi_lower = g_received_cmd.threshold.humi_lower;
            lim_value.shumi_upper = g_received_cmd.threshold.soil_upper;
            lim_value.shumi_lower = g_received_cmd.threshold.soil_lower;
            lim_value.light_upper = g_received_cmd.threshold.light_upper;
            lim_value.light_lower = g_received_cmd.threshold.light_lower;
            create_popup();
            show_popup("Threshold Updated", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_GET_STATUS:
            /* 发送当前设备状态 */
            {
                my_device_status_t status;
                status.mode = mode;
                status.light_status = light_status;
                status.water_status = water_status;
                status.fan_status = fun_status;
                myserver_send_device_status(&status);
            }
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            break;

        case CMD_REBOOT:
            create_popup();
            show_popup("Rebooting...", 2000);
            myserver_send_ack(g_received_cmd.cmd_id, 1);
            delay_ms(500);
            NVIC_SystemReset();
            break;

        case CMD_NONE:
        default:
            break;
    }
}

/******************************************************************************************/
/* 动态配置相关函数 (V2.0新增) */

/**
 * @brief  初始化默认配置
 */
void myserver_init_default_config(void)
{
    memset(&g_device_config, 0, sizeof(g_device_config));
    g_device_config.config_loaded = 0;
    printf("[MyServer] Default config initialized\r\n");
}

/**
 * @brief  注册传感器配置
 */
uint8_t myserver_register_sensor(const char *key, uint8_t data_type, void *value_ptr)
{
    if (g_device_config.sensor_count >= MY_MAX_SENSORS) {
        printf("[MyServer] Sensor config full!\r\n");
        return 1;
    }

    my_sensor_config_t *sensor = &g_device_config.sensors[g_device_config.sensor_count];
    strncpy(sensor->key, key, MY_MAX_KEY_LEN - 1);
    sensor->key[MY_MAX_KEY_LEN - 1] = '\0';
    sensor->data_type = data_type;
    sensor->value_ptr = value_ptr;
    sensor->enabled = 1;

    g_device_config.sensor_count++;
    printf("[MyServer] Registered sensor: %s\r\n", key);
    return 0;
}

/**
 * @brief  注册控制项配置
 */
uint8_t myserver_register_control(const char *key, const char *cmd_on, const char *cmd_off,
                                   uint8_t *status_ptr, void (*action_on)(void), void (*action_off)(void))
{
    if (g_device_config.control_count >= MY_MAX_CONTROLS) {
        printf("[MyServer] Control config full!\r\n");
        return 1;
    }

    my_control_config_t *ctrl = &g_device_config.controls[g_device_config.control_count];
    strncpy(ctrl->key, key, MY_MAX_KEY_LEN - 1);
    ctrl->key[MY_MAX_KEY_LEN - 1] = '\0';
    strncpy(ctrl->cmd_on, cmd_on, MY_MAX_CMD_LEN - 1);
    ctrl->cmd_on[MY_MAX_CMD_LEN - 1] = '\0';
    strncpy(ctrl->cmd_off, cmd_off, MY_MAX_CMD_LEN - 1);
    ctrl->cmd_off[MY_MAX_CMD_LEN - 1] = '\0';
    ctrl->status_ptr = status_ptr;
    ctrl->action_on = action_on;
    ctrl->action_off = action_off;

    g_device_config.control_count++;
    printf("[MyServer] Registered control: %s\r\n", key);
    return 0;
}

/**
 * @brief  动态发送传感器数据 (根据配置, V2.0协议格式)
 */
uint8_t myserver_send_sensor_data_dynamic(void)
{
    int i;
    int offset;
    int first = 1;

    if (g_device_config.sensor_count == 0) {
        printf("[MyServer] No sensors registered!\r\n");
        return 1;
    }

    /* V2.0协议: {"v":"1.0","id":"xxx","ts":123,"t":"dat","d":"设备ID","p":{传感器数据}} */
    offset = snprintf(s_send_buf, sizeof(s_send_buf),
        "{\"v\":\"1.0\",\"id\":\"%lu\",\"ts\":%lu,\"t\":\"%s\",\"d\":\"%s\",\"p\":{",
        (unsigned long)s_msg_seq++, (unsigned long)(s_msg_seq * 1000),
        MSG_TYPE_DAT, MY_DEVICE_ID);

    /* 动态构建传感器数据JSON */
    for (i = 0; i < g_device_config.sensor_count; i++) {
        my_sensor_config_t *sensor = &g_device_config.sensors[i];
        if (!sensor->enabled || sensor->value_ptr == NULL) continue;

        if (!first) {
            offset += snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset, ",");
        }
        first = 0;

        switch (sensor->data_type) {
            case MY_DATA_TYPE_INT:
                offset += snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset,
                    "\"%s\":%d", sensor->key, *(uint8_t*)sensor->value_ptr);
                break;
            case MY_DATA_TYPE_FLOAT:
                offset += snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset,
                    "\"%s\":%.1f", sensor->key, *(float*)sensor->value_ptr);
                break;
            case MY_DATA_TYPE_BOOL:
                offset += snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset,
                    "\"%s\":%d", sensor->key, *(uint8_t*)sensor->value_ptr ? 1 : 0);
                break;
            default:
                offset += snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset,
                    "\"%s\":%d", sensor->key, *(uint8_t*)sensor->value_ptr);
                break;
        }
    }

    snprintf(s_send_buf + offset, sizeof(s_send_buf) - offset, "}}\n");

    return send_json_message(s_send_buf);
}

/**
 * @brief  动态处理控制命令 (根据配置)
 * @note   此函数用于处理通过 myserver_register_control() 注册的自定义控制项
 *         当前版本使用硬编码的 myserver_wireless_control() 处理内置控制项,
 *         此函数可用于扩展自定义控制项的处理
 */
uint8_t myserver_handle_command_dynamic(const char *cmd)
{
    int i;

    if (cmd == NULL) return 1;

    for (i = 0; i < g_device_config.control_count; i++) {
        my_control_config_t *ctrl = &g_device_config.controls[i];

        /* 检查开启命令 */
        if (strcmp(cmd, ctrl->cmd_on) == 0) {
            if (ctrl->action_on) ctrl->action_on();
            if (ctrl->status_ptr) *ctrl->status_ptr = 1;
            printf("[MyServer] Dynamic cmd: %s ON\r\n", ctrl->key);
            return 0;
        }
        /* 检查关闭命令 */
        else if (strcmp(cmd, ctrl->cmd_off) == 0) {
            if (ctrl->action_off) ctrl->action_off();
            if (ctrl->status_ptr) *ctrl->status_ptr = 0;
            printf("[MyServer] Dynamic cmd: %s OFF\r\n", ctrl->key);
            return 0;
        }
    }

    return 1;  /* 未找到匹配命令 */
}
