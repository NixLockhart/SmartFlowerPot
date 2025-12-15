/**
 * @file    sensor_manager.h
 * @brief   传感器数据管理模块
 * @version 2.0
 */

#ifndef __SENSOR_MANAGER_H
#define __SENSOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* 传感器类型 */
typedef enum {
    SENSOR_SOIL_MOISTURE = 0,   /* 土壤湿度 */
    SENSOR_TEMPERATURE,         /* 环境温度 (x10) */
    SENSOR_HUMIDITY,            /* 空气湿度 */
    SENSOR_LIGHT,               /* 光照强度 */
    SENSOR_WATER_LEVEL,         /* 水位 */
    SENSOR_MAX
} sensor_type_t;

/* 传感器键名 */
#define SENSOR_KEY_SOIL     "soil"
#define SENSOR_KEY_TEMP     "temp"
#define SENSOR_KEY_HUMI     "humi"
#define SENSOR_KEY_LIGHT    "light"
#define SENSOR_KEY_WATER    "water"

/* 传感器数据 */
typedef struct {
    int32_t value;              /* 当前值 */
    int32_t min_value;          /* 最小值 (记录) */
    int32_t max_value;          /* 最大值 (记录) */
    uint32_t last_update;       /* 最后更新时间 */
    bool valid;                 /* 数据是否有效 */
} sensor_data_t;

/* 传感器读取函数类型 */
typedef int32_t (*sensor_reader_t)(void);

/**
 * @brief  初始化传感器管理器
 */
void sensor_manager_init(void);

/**
 * @brief  注册传感器读取函数
 * @param  type: 传感器类型
 * @param  reader: 读取函数
 */
void sensor_manager_register_reader(sensor_type_t type, sensor_reader_t reader);

/**
 * @brief  更新传感器数据
 * @param  type: 传感器类型
 * @param  value: 新值
 */
void sensor_manager_update(sensor_type_t type, int32_t value);

/**
 * @brief  读取并更新所有传感器
 */
void sensor_manager_read_all(void);

/**
 * @brief  获取传感器数据
 * @param  type: 传感器类型
 * @retval 传感器数据指针 (只读)
 */
const sensor_data_t* sensor_manager_get(sensor_type_t type);

/**
 * @brief  获取传感器当前值
 * @param  type: 传感器类型
 * @retval 当前值
 */
int32_t sensor_manager_get_value(sensor_type_t type);

/**
 * @brief  获取所有传感器数据用于上报
 * @param  keys: 输出键名数组
 * @param  values: 输出值数组
 * @retval 有效传感器数量
 */
uint8_t sensor_manager_get_all(const char **keys, int32_t *values);

/**
 * @brief  重置统计数据 (最小/最大值)
 */
void sensor_manager_reset_stats(void);

/**
 * @brief  检查传感器数据是否过期
 * @param  type: 传感器类型
 * @param  timeout_ms: 超时时间 (毫秒)
 * @retval true: 过期, false: 有效
 */
bool sensor_manager_is_stale(sensor_type_t type, uint32_t timeout_ms);

/**
 * @brief  键名转类型
 * @param  key: 键名
 * @retval 类型, -1表示无效
 */
int sensor_manager_key_to_type(const char *key);

/**
 * @brief  类型转键名
 * @param  type: 类型
 * @retval 键名, NULL表示无效
 */
const char* sensor_manager_type_to_key(sensor_type_t type);

#endif /* __SENSOR_MANAGER_H */
