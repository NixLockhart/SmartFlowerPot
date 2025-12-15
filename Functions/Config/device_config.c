/**
 * @file    device_config.c
 * @brief   设备配置模块实现
 * @version 2.0
 */

#include "device_config.h"
#include "../Protocol/json_parser.h"
#include "../../HARDWARE/24CXX/24cxx.h"
#include <string.h>
#include <stdio.h>

/* EEPROM存储地址 */
#define CONFIG_EEPROM_ADDR      0x0000
#define CONFIG_MAGIC            0x53465032  /* "SFP2" */

/* 存储结构 (带校验) */
typedef struct {
    uint32_t magic;
    device_config_t config;
    uint32_t checksum;
} config_storage_t;

/* 默认配置 */
static const device_config_t s_default_config = {
    .device_id = "SFP_DEVICE_001",
    .device_name = "Smart Flower Pot",
    .user_id = "",
    .server_ip = "192.168.1.100",
    .server_port = 8003,
    .report_interval = 30,
    .heartbeat_interval = 10,
    .work_mode = 0,  /* 自动模式 */
    .thresholds = {
        .soil_moisture_low = 30,
        .soil_moisture_high = 70,
        .temperature_low = 150,   /* 15.0°C */
        .temperature_high = 300,  /* 30.0°C */
        .humidity_low = 40,
        .humidity_high = 80,
        .light_low = 200,
        .light_high = 800
    },
    .config_version = 0
};

/* 当前配置 */
static device_config_t s_config;
static config_change_callback_t s_change_cb = NULL;

/* 计算校验和 */
static uint32_t calc_checksum(const device_config_t *config)
{
    const uint8_t *ptr = (const uint8_t *)config;
    uint32_t sum = 0;

    for (size_t i = 0; i < sizeof(device_config_t); i++) {
        sum += ptr[i];
    }

    return sum ^ CONFIG_MAGIC;
}

/* 通知配置变更 */
static void notify_config_change(void)
{
    if (s_change_cb) {
        s_change_cb(&s_config);
    }
}

/* ===== 公共接口 ===== */

void device_config_init(void)
{
    /* 尝试从EEPROM加载配置 */
    if (device_config_load() != 0) {
        /* 加载失败，使用默认配置 */
        device_config_reset_default();
    }
}

const device_config_t* device_config_get(void)
{
    return &s_config;
}

void device_config_set_callback(config_change_callback_t callback)
{
    s_change_cb = callback;
}

int device_config_update_from_json(const char *json)
{
    char str_buf[64];
    int int_val;
    bool changed = false;

    if (!json) {
        return -1;
    }

    /* 解析设备名称 */
    if (json_get_string(json, "name", str_buf, sizeof(str_buf))) {
        if (strcmp(s_config.device_name, str_buf) != 0) {
            strncpy(s_config.device_name, str_buf, sizeof(s_config.device_name) - 1);
            changed = true;
        }
    }

    /* 解析上报间隔 */
    int_val = json_get_int(json, "ri", -1);
    if (int_val > 0 && int_val != s_config.report_interval) {
        s_config.report_interval = (uint16_t)int_val;
        changed = true;
    }

    /* 解析心跳间隔 */
    int_val = json_get_int(json, "hb", -1);
    if (int_val > 0 && int_val != s_config.heartbeat_interval) {
        s_config.heartbeat_interval = (uint16_t)int_val;
        changed = true;
    }

    /* 解析工作模式 */
    int_val = json_get_int(json, "mode", -1);
    if (int_val >= 0 && int_val != s_config.work_mode) {
        s_config.work_mode = (uint8_t)int_val;
        changed = true;
    }

    /* 解析阈值 - 土壤湿度 */
    int_val = json_get_int(json, "sm_l", -1);
    if (int_val >= 0) {
        s_config.thresholds.soil_moisture_low = (int16_t)int_val;
        changed = true;
    }
    int_val = json_get_int(json, "sm_h", -1);
    if (int_val >= 0) {
        s_config.thresholds.soil_moisture_high = (int16_t)int_val;
        changed = true;
    }

    /* 解析阈值 - 温度 */
    int_val = json_get_int(json, "t_l", -32768);
    if (int_val != -32768) {
        s_config.thresholds.temperature_low = (int16_t)int_val;
        changed = true;
    }
    int_val = json_get_int(json, "t_h", -32768);
    if (int_val != -32768) {
        s_config.thresholds.temperature_high = (int16_t)int_val;
        changed = true;
    }

    /* 解析阈值 - 湿度 */
    int_val = json_get_int(json, "h_l", -1);
    if (int_val >= 0) {
        s_config.thresholds.humidity_low = (int16_t)int_val;
        changed = true;
    }
    int_val = json_get_int(json, "h_h", -1);
    if (int_val >= 0) {
        s_config.thresholds.humidity_high = (int16_t)int_val;
        changed = true;
    }

    /* 解析阈值 - 光照 */
    int_val = json_get_int(json, "l_l", -1);
    if (int_val >= 0) {
        s_config.thresholds.light_low = (int16_t)int_val;
        changed = true;
    }
    int_val = json_get_int(json, "l_h", -1);
    if (int_val >= 0) {
        s_config.thresholds.light_high = (int16_t)int_val;
        changed = true;
    }

    /* 解析配置版本 */
    int_val = json_get_int(json, "ver", -1);
    if (int_val > 0) {
        s_config.config_version = (uint32_t)int_val;
    }

    if (changed) {
        /* 保存到EEPROM */
        device_config_save();

        /* 通知变更 */
        notify_config_change();
    }

    return 0;
}

void device_config_set_device_id(const char *device_id)
{
    if (device_id) {
        strncpy(s_config.device_id, device_id, sizeof(s_config.device_id) - 1);
    }
}

void device_config_set_user_id(const char *user_id)
{
    if (user_id) {
        strncpy(s_config.user_id, user_id, sizeof(s_config.user_id) - 1);
    }
}

void device_config_set_server(const char *ip, uint16_t port)
{
    if (ip) {
        strncpy(s_config.server_ip, ip, sizeof(s_config.server_ip) - 1);
    }
    s_config.server_port = port;
}

void device_config_set_work_mode(uint8_t mode)
{
    if (mode != s_config.work_mode) {
        s_config.work_mode = mode;
        notify_config_change();
    }
}

void device_config_set_thresholds(const threshold_config_t *thresholds)
{
    if (thresholds) {
        memcpy(&s_config.thresholds, thresholds, sizeof(threshold_config_t));
        notify_config_change();
    }
}

int device_config_save(void)
{
    config_storage_t storage;
    uint8_t *ptr;

    storage.magic = CONFIG_MAGIC;
    memcpy(&storage.config, &s_config, sizeof(device_config_t));
    storage.checksum = calc_checksum(&s_config);

    /* 写入EEPROM */
    ptr = (uint8_t *)&storage;
    for (size_t i = 0; i < sizeof(config_storage_t); i++) {
        AT24CXX_WriteOneByte(CONFIG_EEPROM_ADDR + i, ptr[i]);
    }

    return 0;
}

int device_config_load(void)
{
    config_storage_t storage;
    uint8_t *ptr;
    uint32_t checksum;

    /* 从EEPROM读取 */
    ptr = (uint8_t *)&storage;
    for (size_t i = 0; i < sizeof(config_storage_t); i++) {
        ptr[i] = AT24CXX_ReadOneByte(CONFIG_EEPROM_ADDR + i);
    }

    /* 检查魔数 */
    if (storage.magic != CONFIG_MAGIC) {
        return -1;
    }

    /* 校验配置 */
    checksum = calc_checksum(&storage.config);
    if (checksum != storage.checksum) {
        return -2;
    }

    /* 复制配置 */
    memcpy(&s_config, &storage.config, sizeof(device_config_t));

    return 0;
}

void device_config_reset_default(void)
{
    memcpy(&s_config, &s_default_config, sizeof(device_config_t));
    notify_config_change();
}

bool device_config_is_valid(void)
{
    /* 检查必要字段 */
    if (s_config.device_id[0] == '\0') {
        return false;
    }

    if (s_config.server_ip[0] == '\0') {
        return false;
    }

    if (s_config.server_port == 0) {
        return false;
    }

    return true;
}
