/**
 * @file    threshold_engine.h
 * @brief   阈值自动控制引擎
 * @version 2.0
 */

#ifndef __THRESHOLD_ENGINE_H
#define __THRESHOLD_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include "device_config.h"
#include "sensor_manager.h"
#include "control_manager.h"

/* 控制动作 */
typedef enum {
    ACTION_NONE = 0,
    ACTION_TURN_ON,
    ACTION_TURN_OFF
} control_action_t;

/* 触发记录 */
typedef struct {
    control_item_t control;
    control_action_t action;
    sensor_type_t trigger_sensor;
    int32_t trigger_value;
    int32_t threshold_value;
} trigger_record_t;

/* 触发回调 */
typedef void (*trigger_callback_t)(const trigger_record_t *record);

/**
 * @brief  初始化阈值引擎
 */
void threshold_engine_init(void);

/**
 * @brief  设置触发回调
 * @param  callback: 回调函数
 */
void threshold_engine_set_callback(trigger_callback_t callback);

/**
 * @brief  执行阈值检查
 * @note   应在自动模式下定期调用
 */
void threshold_engine_process(void);

/**
 * @brief  检查单个传感器阈值
 * @param  sensor: 传感器类型
 * @param  value: 当前值
 * @param  low: 下限
 * @param  high: 上限
 * @param  on_low_action: 低于下限时的动作
 * @param  on_high_action: 高于上限时的动作
 * @retval 触发的动作
 */
control_action_t threshold_engine_check(sensor_type_t sensor, int32_t value,
                                         int32_t low, int32_t high,
                                         control_action_t on_low_action,
                                         control_action_t on_high_action);

/**
 * @brief  获取最后一次触发记录
 * @retval 触发记录指针
 */
const trigger_record_t* threshold_engine_get_last_trigger(void);

/**
 * @brief  重置触发记录
 */
void threshold_engine_reset(void);

/**
 * @brief  启用/禁用引擎
 * @param  enable: true=启用, false=禁用
 */
void threshold_engine_enable(bool enable);

/**
 * @brief  检查引擎是否启用
 * @retval true=启用, false=禁用
 */
bool threshold_engine_is_enabled(void);

#endif /* __THRESHOLD_ENGINE_H */
