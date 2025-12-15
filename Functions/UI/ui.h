/**
 ****************************************************************************************************
 * @file        ui.h
 * @author      NixStudio(NixLockhart)
 * @version     V1.0
 * @date        2025-06-08
 * @brief       UI模块
 ****************************************************************************************************
 * @attention
 *
 * 平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#ifndef __UI_H
#define __UI_H

#include "sys.h"
#include "lvgl/lvgl.h"
#include "key.h"

// 屏幕类型枚举
typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_MANUAL,
    SCREEN_LIMIT
} screen_t;

// 限制值结构体
typedef struct{
    uint8_t temp_upper;
    uint8_t temp_lower;
    uint8_t humi_upper;
    uint8_t humi_lower;
    uint8_t shumi_upper;
    uint8_t shumi_lower;
    uint8_t light_upper;
    uint8_t light_lower;
} limits;

// 全局变量声明
extern limits lim_value;
extern uint8_t temp;
extern uint8_t humi;
extern uint8_t soil_humi;
extern uint8_t light_intensity;
extern uint8_t mode;
extern uint8_t light_status;
extern uint8_t water_status;
extern uint8_t fun_status;
extern uint8_t wifi_sta;
extern uint8_t atkcld_sta;

// 外部函数声明（来自其他模块）
extern uint8_t execute(void);  // WiFi模块函数

// UI函数声明
void UI_Init(void);
void create_main_screen(void);
void update_main_screen(void);
void create_menu_screen(void);
void update_menu_screen(void);
void create_limit_screen(void);
void update_limit_value(void);
void create_manual_screen(void);
void update_manual_screen(void);
void create_popup(void);
void show_popup(const char *message, uint32_t duration_ms);
void hide_popup(lv_timer_t *timer);
void hide_and_destroy_popup(lv_timer_t *timer);
void UI_Switch(uint8_t key);
void Warn_function(void);
void wireless_control(void);
void destroy_all_children(lv_obj_t *parent);
void destory_active_screen(void);
void handle_menu_selection(int index);
screen_t get_current_screen(void);
void set_current_screen(screen_t screen);

#endif

