/**
 * @file    sensor_manager.c
 * @brief   传感器数据管理模块实现
 * @version 2.0
 */

#include "sensor_manager.h"
#include <string.h>
#include <limits.h>

/* 外部时间函数 */
extern uint32_t get_tick_ms(void);

/* 键名映射表 */
static const char* s_key_names[SENSOR_MAX] = {
    SENSOR_KEY_SOIL,
    SENSOR_KEY_TEMP,
    SENSOR_KEY_HUMI,
    SENSOR_KEY_LIGHT,
    SENSOR_KEY_WATER
};

/* 传感器数据 */
static sensor_data_t s_sensors[SENSOR_MAX];

/* 读取函数 */
static sensor_reader_t s_readers[SENSOR_MAX] = {NULL};

/* ===== 公共接口 ===== */

void sensor_manager_init(void)
{
    memset(s_sensors, 0, sizeof(s_sensors));
    memset(s_readers, 0, sizeof(s_readers));

    /* 初始化统计值 */
    for (uint8_t i = 0; i < SENSOR_MAX; i++) {
        s_sensors[i].min_value = INT32_MAX;
        s_sensors[i].max_value = INT32_MIN;
        s_sensors[i].valid = false;
    }
}

void sensor_manager_register_reader(sensor_type_t type, sensor_reader_t reader)
{
    if (type < SENSOR_MAX) {
        s_readers[type] = reader;
    }
}

void sensor_manager_update(sensor_type_t type, int32_t value)
{
    if (type >= SENSOR_MAX) {
        return;
    }

    sensor_data_t *sensor = &s_sensors[type];

    sensor->value = value;
    sensor->last_update = get_tick_ms();
    sensor->valid = true;

    /* 更新统计 */
    if (value < sensor->min_value) {
        sensor->min_value = value;
    }
    if (value > sensor->max_value) {
        sensor->max_value = value;
    }
}

void sensor_manager_read_all(void)
{
    for (uint8_t i = 0; i < SENSOR_MAX; i++) {
        if (s_readers[i]) {
            int32_t value = s_readers[i]();
            sensor_manager_update((sensor_type_t)i, value);
        }
    }
}

const sensor_data_t* sensor_manager_get(sensor_type_t type)
{
    if (type >= SENSOR_MAX) {
        return NULL;
    }

    return &s_sensors[type];
}

int32_t sensor_manager_get_value(sensor_type_t type)
{
    if (type >= SENSOR_MAX) {
        return 0;
    }

    return s_sensors[type].value;
}

uint8_t sensor_manager_get_all(const char **keys, int32_t *values)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < SENSOR_MAX; i++) {
        if (s_sensors[i].valid) {
            if (keys) {
                keys[count] = s_key_names[i];
            }
            if (values) {
                values[count] = s_sensors[i].value;
            }
            count++;
        }
    }

    return count;
}

void sensor_manager_reset_stats(void)
{
    for (uint8_t i = 0; i < SENSOR_MAX; i++) {
        s_sensors[i].min_value = INT32_MAX;
        s_sensors[i].max_value = INT32_MIN;
    }
}

bool sensor_manager_is_stale(sensor_type_t type, uint32_t timeout_ms)
{
    if (type >= SENSOR_MAX) {
        return true;
    }

    if (!s_sensors[type].valid) {
        return true;
    }

    uint32_t now = get_tick_ms();
    return (now - s_sensors[type].last_update) > timeout_ms;
}

int sensor_manager_key_to_type(const char *key)
{
    if (!key) {
        return -1;
    }

    for (uint8_t i = 0; i < SENSOR_MAX; i++) {
        if (strcmp(key, s_key_names[i]) == 0) {
            return i;
        }
    }

    return -1;
}

const char* sensor_manager_type_to_key(sensor_type_t type)
{
    if (type >= SENSOR_MAX) {
        return NULL;
    }

    return s_key_names[type];
}
