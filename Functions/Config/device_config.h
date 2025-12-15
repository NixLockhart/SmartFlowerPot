/**
 * @file    device_config.h
 * @brief   设备配置模块
 * @version 2.0
 */

#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* 阈值配置 */
typedef struct {
    int16_t soil_moisture_low;      /* 土壤湿度下限 */
    int16_t soil_moisture_high;     /* 土壤湿度上限 */
    int16_t temperature_low;        /* 温度下限 (x10) */
    int16_t temperature_high;       /* 温度上限 (x10) */
    int16_t humidity_low;           /* 空气湿度下限 */
    int16_t humidity_high;          /* 空气湿度上限 */
    int16_t light_low;              /* 光照下限 */
    int16_t light_high;             /* 光照上限 */
} threshold_config_t;

/* 设备配置 */
typedef struct {
    char device_id[32];             /* 设备ID */
    char device_name[64];           /* 设备名称 */
    char user_id[32];               /* 绑定的用户ID */
    char server_ip[32];             /* 服务器IP */
    uint16_t server_port;           /* 服务器端口 */
    uint16_t report_interval;       /* 数据上报间隔 (秒) */
    uint16_t heartbeat_interval;    /* 心跳间隔 (秒) */
    uint8_t work_mode;              /* 工作模式: 0=自动, 1=手动 */
    threshold_config_t thresholds;  /* 阈值配置 */
    uint32_t config_version;        /* 配置版本号 */
} device_config_t;

/* 配置变更回调 */
typedef void (*config_change_callback_t)(const device_config_t *config);

/**
 * @brief  初始化配置模块
 */
void device_config_init(void);

/**
 * @brief  获取当前配置
 * @retval 配置指针 (只读)
 */
const device_config_t* device_config_get(void);

/**
 * @brief  设置配置变更回调
 * @param  callback: 回调函数
 */
void device_config_set_callback(config_change_callback_t callback);

/**
 * @brief  从JSON更新配置
 * @param  json: 配置JSON字符串
 * @retval 0: 成功, 其他: 失败
 */
int device_config_update_from_json(const char *json);

/**
 * @brief  设置设备ID
 * @param  device_id: 设备ID
 */
void device_config_set_device_id(const char *device_id);

/**
 * @brief  设置用户ID
 * @param  user_id: 用户ID
 */
void device_config_set_user_id(const char *user_id);

/**
 * @brief  设置服务器地址
 * @param  ip: 服务器IP
 * @param  port: 服务器端口
 */
void device_config_set_server(const char *ip, uint16_t port);

/**
 * @brief  设置工作模式
 * @param  mode: 0=自动, 1=手动
 */
void device_config_set_work_mode(uint8_t mode);

/**
 * @brief  设置阈值
 * @param  thresholds: 阈值配置
 */
void device_config_set_thresholds(const threshold_config_t *thresholds);

/**
 * @brief  保存配置到Flash
 * @retval 0: 成功, 其他: 失败
 */
int device_config_save(void);

/**
 * @brief  从Flash加载配置
 * @retval 0: 成功, 其他: 失败
 */
int device_config_load(void);

/**
 * @brief  恢复默认配置
 */
void device_config_reset_default(void);

/**
 * @brief  检查配置是否有效
 * @retval true: 有效, false: 无效
 */
bool device_config_is_valid(void);

#endif /* __DEVICE_CONFIG_H */
