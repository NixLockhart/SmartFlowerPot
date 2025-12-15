/**
 * @file    control_manager.c
 * @brief   控制状态管理模块实现
 * @version 2.0
 */

#include "control_manager.h"
#include <string.h>

/* 键名映射表 */
static const char* s_key_names[CTRL_MAX] = {
    CTRL_KEY_PUMP,
    CTRL_KEY_FAN,
    CTRL_KEY_LIGHT,
    CTRL_KEY_HEATER
};

/* 控制状态 */
static uint8_t s_states[CTRL_MAX] = {0};

/* 执行回调 */
static control_executor_t s_executor = NULL;

/* ===== 公共接口 ===== */

void control_manager_init(void)
{
    memset(s_states, 0, sizeof(s_states));
    s_executor = NULL;
}

void control_manager_set_executor(control_executor_t executor)
{
    s_executor = executor;
}

int control_manager_set_by_key(const char *key, uint8_t state)
{
    int item = control_manager_key_to_item(key);

    if (item < 0) {
        return -1;
    }

    control_manager_set((control_item_t)item, state);
    return 0;
}

void control_manager_set(control_item_t item, uint8_t state)
{
    if (item >= CTRL_MAX) {
        return;
    }

    /* 更新状态 */
    s_states[item] = state;

    /* 调用执行回调 */
    if (s_executor) {
        s_executor(item, state);
    }
}

uint8_t control_manager_get(control_item_t item)
{
    if (item >= CTRL_MAX) {
        return 0;
    }

    return s_states[item];
}

int control_manager_get_by_key(const char *key)
{
    int item = control_manager_key_to_item(key);

    if (item < 0) {
        return -1;
    }

    return s_states[item];
}

uint8_t control_manager_get_all(const char **keys, uint8_t *states)
{
    for (uint8_t i = 0; i < CTRL_MAX; i++) {
        if (keys) {
            keys[i] = s_key_names[i];
        }
        if (states) {
            states[i] = s_states[i];
        }
    }

    return CTRL_MAX;
}

void control_manager_toggle(control_item_t item)
{
    if (item >= CTRL_MAX) {
        return;
    }

    control_manager_set(item, s_states[item] ? 0 : 1);
}

void control_manager_all_off(void)
{
    for (uint8_t i = 0; i < CTRL_MAX; i++) {
        control_manager_set((control_item_t)i, 0);
    }
}

int control_manager_key_to_item(const char *key)
{
    if (!key) {
        return -1;
    }

    for (uint8_t i = 0; i < CTRL_MAX; i++) {
        if (strcmp(key, s_key_names[i]) == 0) {
            return i;
        }
    }

    return -1;
}

const char* control_manager_item_to_key(control_item_t item)
{
    if (item >= CTRL_MAX) {
        return NULL;
    }

    return s_key_names[item];
}
