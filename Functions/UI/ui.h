/**
 ****************************************************************************************************
 * @file        ui.h
 * @author      NixStudio(NixLockhart)
 * @version     V1.3
 * @date        2025-06-08
 * @lastupdate  2025-12-16
 * @brief       UI模块
 ****************************************************************************************************
 * @attention
 *
 * 平台: 正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#ifndef __UI_H
#define __UI_H

#include "sys.h"
#include "lvgl/lvgl.h"
#include "key.h"

/*------------------------------------------------------------------------------
 * UI颜色宏定义 - 统一管理所有UI颜色配置
 *----------------------------------------------------------------------------*/

/* 屏幕背景颜色 */
#define UI_COLOR_BG_SCREEN          0xF0F8FF    /* 更淡的浅蓝 - 主屏幕背景 */
#define UI_COLOR_BG_TITLE           0x1E90FF    /* 较深的天蓝色 - 标题栏/提示栏背景 */

/* 主界面传感器容器背景颜色 */
#define UI_COLOR_BG_TEMP            0xFF7F7F    /* 红色 - 温度容器背景 */
#define UI_COLOR_BG_HUMI            0x00AA00    /* 绿色 - 湿度容器背景 */
#define UI_COLOR_BG_SOIL            0xFFD700    /* 金色 - 土壤湿度容器背景 */
#define UI_COLOR_BG_LIGHT           0x00BFFF    /* 天蓝色 - 光照容器背景 */

/* 阈值设置界面容器背景颜色 */
#define UI_COLOR_BG_UPPER           0xADD8E6    /* 浅蓝色 - 上限设置区域背景 */
#define UI_COLOR_BG_LOWER           0xFFFFE0    /* 浅奶油黄 - 下限设置区域背景 */

/* 手动控制界面容器背景颜色 */
#define UI_COLOR_BG_WATER           0x87CEFA    /* 浅天蓝色 - 水泵控制背景 */
#define UI_COLOR_BG_LED             0xFFD700    /* 金色 - 补光灯控制背景 */
#define UI_COLOR_BG_FAN             0x98FB98    /* 浅绿色 - 风扇控制背景 */

/* 弹窗背景颜色 */
#define UI_COLOR_BG_POPUP           0x4682B4    /* 深蓝色 - 弹窗背景 */

/* 状态指示颜色 */
#define UI_COLOR_STATUS_ON          0x32CD32    /* 明亮绿色 - ON状态 */
#define UI_COLOR_STATUS_OFF         0x808080    /* 中灰色 - OFF状态 */

/* 边框颜色 */
#define UI_COLOR_BORDER_SELECTED    0xFF4500    /* 橙红色 - 选中边框 */

/* 告警颜色 */
#define UI_COLOR_WARN_HIGH          0xFF4500    /* 橙红色 - 高温/超上限告警 */
#define UI_COLOR_WARN_LOW           0x1E90FF    /* 天蓝色 - 低温/低于下限告警 */

//屏幕类型枚举
typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_MANUAL,
    SCREEN_LIMIT
} screen_t;

//阈值结构体
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

//全局变量声明
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

//外部函数声明（来自其他模块）
extern uint8_t execute(void);  /* WiFi模块函数 */

//UI函数声明
void UI_Init(void);
void create_main_screen(void);
void update_main_screen(void);
void create_menu_screen(void);
void update_menu_screen(void);
void create_limit_screen(void);
void update_limit_screen(void);
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

