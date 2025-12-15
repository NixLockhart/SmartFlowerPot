/**
 * @file    control_manager.h
 * @brief   控制状态管理模块
 * @version 2.0
 */

#ifndef __CONTROL_MANAGER_H
#define __CONTROL_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* 控制项定义 */
typedef enum {
    CTRL_PUMP = 0,      /* 水泵 */
    CTRL_FAN,           /* 风扇 */
    CTRL_LIGHT,         /* 补光灯 */
    CTRL_HEATER,        /* 加热器 */
    CTRL_MAX
} control_item_t;

/* 控制项键名 */
#define CTRL_KEY_PUMP       "pump"
#define CTRL_KEY_FAN        "fan"
#define CTRL_KEY_LIGHT      "light"
#define CTRL_KEY_HEATER     "heater"

/* 控制执行回调 */
typedef void (*control_executor_t)(control_item_t item, uint8_t state);

/**
 * @brief  初始化控制管理器
 */
void control_manager_init(void);

/**
 * @brief  设置控制执行回调
 * @param  executor: 执行函数
 */
void control_manager_set_executor(control_executor_t executor);

/**
 * @brief  根据键名设置控制状态
 * @param  key: 控制项键名
 * @param  state: 状态值
 * @retval 0: 成功, -1: 未知键名
 */
int control_manager_set_by_key(const char *key, uint8_t state);

/**
 * @brief  根据枚举设置控制状态
 * @param  item: 控制项
 * @param  state: 状态值
 */
void control_manager_set(control_item_t item, uint8_t state);

/**
 * @brief  获取控制状态
 * @param  item: 控制项
 * @retval 状态值
 */
uint8_t control_manager_get(control_item_t item);

/**
 * @brief  根据键名获取控制状态
 * @param  key: 控制项键名
 * @retval 状态值, -1表示未知键名
 */
int control_manager_get_by_key(const char *key);

/**
 * @brief  获取所有控制项状态
 * @param  keys: 输出键名数组
 * @param  states: 输出状态数组
 * @retval 控制项数量
 */
uint8_t control_manager_get_all(const char **keys, uint8_t *states);

/**
 * @brief  切换控制状态
 * @param  item: 控制项
 */
void control_manager_toggle(control_item_t item);

/**
 * @brief  全部关闭
 */
void control_manager_all_off(void);

/**
 * @brief  键名转枚举
 * @param  key: 键名
 * @retval 枚举值, -1表示无效
 */
int control_manager_key_to_item(const char *key);

/**
 * @brief  枚举转键名
 * @param  item: 枚举值
 * @retval 键名, NULL表示无效
 */
const char* control_manager_item_to_key(control_item_t item);

#endif /* __CONTROL_MANAGER_H */
