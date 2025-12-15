/**
 * @file    threshold_engine.c
 * @brief   阈值自动控制引擎实现
 * @version 2.0
 */

#include "threshold_engine.h"
#include <string.h>

/* 内部状态 */
static struct {
    bool enabled;
    trigger_record_t last_trigger;
    trigger_callback_t callback;
} s_engine = {0};

/* 记录并执行触发动作 */
static void execute_trigger(control_item_t ctrl, control_action_t action,
                            sensor_type_t sensor, int32_t value, int32_t threshold)
{
    if (action == ACTION_NONE) {
        return;
    }

    uint8_t state = (action == ACTION_TURN_ON) ? 1 : 0;

    /* 检查是否需要改变状态 */
    if (control_manager_get(ctrl) == state) {
        return;  /* 状态已经是目标状态 */
    }

    /* 记录触发 */
    s_engine.last_trigger.control = ctrl;
    s_engine.last_trigger.action = action;
    s_engine.last_trigger.trigger_sensor = sensor;
    s_engine.last_trigger.trigger_value = value;
    s_engine.last_trigger.threshold_value = threshold;

    /* 执行控制 */
    control_manager_set(ctrl, state);

    /* 回调通知 */
    if (s_engine.callback) {
        s_engine.callback(&s_engine.last_trigger);
    }
}

/* ===== 公共接口 ===== */

void threshold_engine_init(void)
{
    memset(&s_engine, 0, sizeof(s_engine));
    s_engine.enabled = true;  /* 默认启用 */
}

void threshold_engine_set_callback(trigger_callback_t callback)
{
    s_engine.callback = callback;
}

void threshold_engine_process(void)
{
    const device_config_t *config;
    const threshold_config_t *th;
    int32_t value;

    if (!s_engine.enabled) {
        return;
    }

    config = device_config_get();
    if (!config || config->work_mode != 0) {
        return;  /* 非自动模式 */
    }

    th = &config->thresholds;

    /* 检查土壤湿度 -> 控制水泵 */
    value = sensor_manager_get_value(SENSOR_SOIL_MOISTURE);
    if (value < th->soil_moisture_low) {
        /* 土壤太干，开水泵 */
        execute_trigger(CTRL_PUMP, ACTION_TURN_ON,
                        SENSOR_SOIL_MOISTURE, value, th->soil_moisture_low);
    }
    else if (value > th->soil_moisture_high) {
        /* 土壤够湿，关水泵 */
        execute_trigger(CTRL_PUMP, ACTION_TURN_OFF,
                        SENSOR_SOIL_MOISTURE, value, th->soil_moisture_high);
    }

    /* 检查温度 -> 控制加热器和风扇 */
    value = sensor_manager_get_value(SENSOR_TEMPERATURE);
    if (value < th->temperature_low) {
        /* 温度太低，开加热器 */
        execute_trigger(CTRL_HEATER, ACTION_TURN_ON,
                        SENSOR_TEMPERATURE, value, th->temperature_low);
        /* 关风扇 */
        execute_trigger(CTRL_FAN, ACTION_TURN_OFF,
                        SENSOR_TEMPERATURE, value, th->temperature_low);
    }
    else if (value > th->temperature_high) {
        /* 温度太高，开风扇 */
        execute_trigger(CTRL_FAN, ACTION_TURN_ON,
                        SENSOR_TEMPERATURE, value, th->temperature_high);
        /* 关加热器 */
        execute_trigger(CTRL_HEATER, ACTION_TURN_OFF,
                        SENSOR_TEMPERATURE, value, th->temperature_high);
    }
    else {
        /* 温度正常范围，可以关闭加热器和风扇 */
        int32_t mid = (th->temperature_low + th->temperature_high) / 2;
        if (value > mid - 20 && value < mid + 20) {
            /* 在中间区域，关闭调节设备 */
            if (control_manager_get(CTRL_HEATER)) {
                execute_trigger(CTRL_HEATER, ACTION_TURN_OFF,
                                SENSOR_TEMPERATURE, value, mid);
            }
            if (control_manager_get(CTRL_FAN)) {
                execute_trigger(CTRL_FAN, ACTION_TURN_OFF,
                                SENSOR_TEMPERATURE, value, mid);
            }
        }
    }

    /* 检查光照 -> 控制补光灯 */
    value = sensor_manager_get_value(SENSOR_LIGHT);
    if (value < th->light_low) {
        /* 光照不足，开补光灯 */
        execute_trigger(CTRL_LIGHT, ACTION_TURN_ON,
                        SENSOR_LIGHT, value, th->light_low);
    }
    else if (value > th->light_high) {
        /* 光照充足，关补光灯 */
        execute_trigger(CTRL_LIGHT, ACTION_TURN_OFF,
                        SENSOR_LIGHT, value, th->light_high);
    }
}

control_action_t threshold_engine_check(sensor_type_t sensor, int32_t value,
                                         int32_t low, int32_t high,
                                         control_action_t on_low_action,
                                         control_action_t on_high_action)
{
    if (value < low) {
        return on_low_action;
    }
    else if (value > high) {
        return on_high_action;
    }

    return ACTION_NONE;
}

const trigger_record_t* threshold_engine_get_last_trigger(void)
{
    return &s_engine.last_trigger;
}

void threshold_engine_reset(void)
{
    memset(&s_engine.last_trigger, 0, sizeof(trigger_record_t));
}

void threshold_engine_enable(bool enable)
{
    s_engine.enabled = enable;
}

bool threshold_engine_is_enabled(void)
{
    return s_engine.enabled;
}
