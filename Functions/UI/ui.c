/**
 ****************************************************************************************************
 * @file        ui.c
 * @author      NixStudio(NixLockhart)
 * @version     V1.1
 * @date        2025-06-08
 * @lastupdate  2025-12-15
 * @brief       UI模块实现
 ****************************************************************************************************
 * @attention
 *
 * 平台: 正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#include "ui.h"
#include "led.h"
#include "bump.h"
#include "ts.h"
#include "delay.h"

/* 全局变量定义 */
limits lim_value;
uint8_t temp = 25;
uint8_t humi = 60;
uint8_t soil_humi = 40;
uint8_t light_intensity = 80;
uint8_t mode = 0;
uint8_t light_status = 0;
uint8_t water_status = 0;
uint8_t fun_status = 0;
uint8_t wifi_sta = 0;
uint8_t atkcld_sta = 0;

/* 静态变量 */
static screen_t current_screen = SCREEN_MAIN;
static uint8_t menu_index = 0;
static uint8_t menu_item = 4;
static uint8_t limit_page = 1;
static uint8_t limit_index = 0;   /* 阈值设置页面当前选中项 (0=上限, 1=下限) */
static uint8_t manual_index = 0;  /* 手动控制页面当前选中项 (0=水泵, 1=补光灯, 2=风扇) */

/* 手动控制页面容器对象 */
static lv_obj_t *water_container;
static lv_obj_t *light_container;
static lv_obj_t *fan_container;

/* 阈值设置页面容器对象 */
static lv_obj_t *upper_container;
static lv_obj_t *lower_container;

/* 屏幕对象 */
static lv_obj_t *scr_main;
static lv_obj_t *scr_menu;
static lv_obj_t *scr_manual;
static lv_obj_t *scr_limit;

/* 标签对象 */
static lv_obj_t *label_temp;
static lv_obj_t *label_humi;
static lv_obj_t *label_soil_humi;
static lv_obj_t *label_light;
static lv_obj_t *label_mode;
static lv_obj_t *label_menu;
static lv_obj_t *label_light_status;
static lv_obj_t *label_water_status;
static lv_obj_t *label_fan_status;
static lv_obj_t *label_lower_value;
static lv_obj_t *label_upper_value;

/* 弹窗容器对象 */
static lv_obj_t *popup_container;
static lv_obj_t *popup_label;
static lv_timer_t *popup_timer;  /* 弹窗定时器 */

/* 阈值边界 */
static uint8_t temp_max = 50;
static uint8_t temp_min = 0;
static uint8_t shumi_max = 65;
static uint8_t shumi_min = 40;
static uint8_t light_max = 100;
static uint8_t light_min = 30;

/**
 * @brief  UI初始化
 * @note   初始化阈值默认值
 * @retval 无
 */
void UI_Init(void) {
    lim_value.temp_upper = 30;
    lim_value.temp_lower = 10;
    lim_value.humi_upper = 70;
    lim_value.humi_lower = 40;
    lim_value.shumi_upper = 65;
    lim_value.shumi_lower = 40;
    lim_value.light_upper = 90;
    lim_value.light_lower = 50;
}

/* 前向声明 */
static void cleanup_popup(void);

/**
 * @brief  删除所有子对象
 * @param  parent: 父对象指针
 * @retval 无
 */
void destroy_all_children(lv_obj_t *parent) {
    lv_obj_t *child = lv_obj_get_child(parent, NULL);
    while (child) {
        lv_obj_del(child);
        child = lv_obj_get_child(parent, NULL);
    }
}

/**
 * @brief  销毁当前活动屏幕
 * @note   销毁前先清理弹窗定时器，防止内存访问错误
 * @retval 无
 */
void destory_active_screen() {
    /* 先清理弹窗，防止定时器访问已销毁的对象 */
    cleanup_popup();

    lv_obj_t *active_screen = lv_scr_act();
    destroy_all_children(active_screen);
}

/**
 * @brief  创建主屏幕
 * @note   显示温度、湿度、土壤湿度、光照强度四个传感器数据
 * @retval 无
 */
void create_main_screen(void) {
    scr_main = lv_scr_act();
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(UI_COLOR_BG_SCREEN), 0);  /* 设置为浅蓝色 */
    lv_obj_set_style_bg_opa(scr_main, LV_OPA_COVER, 0);

    /* 创建标题栏 */
    lv_obj_t *title_container = lv_obj_create(scr_main);
    lv_obj_set_size(title_container, 320, 40);
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(title_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);  /* 深蓝色背景 */
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Smart Flower Pot");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    /* 温度容器 - 左上角 */
    lv_obj_t *temp_container = lv_obj_create(scr_main);
    lv_obj_set_size(temp_container, 140, 60);
    lv_obj_align(temp_container, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_obj_set_style_bg_color(temp_container, lv_color_hex(UI_COLOR_BG_TEMP), 0);  /* 红色背景 */
    lv_obj_set_style_bg_opa(temp_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(temp_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *temp_label = lv_label_create(temp_container);
    lv_label_set_text(temp_label, "Temp");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_align(temp_label, LV_ALIGN_TOP_MID, 0, -5);

    label_temp = lv_label_create(temp_container);
    lv_label_set_text(label_temp, "25 C");
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
    lv_obj_align(label_temp, LV_ALIGN_BOTTOM_MID, 0, 5);

    /* 湿度容器 - 右上角 */
    lv_obj_t *humi_container = lv_obj_create(scr_main);
    lv_obj_set_size(humi_container, 140, 60);
    lv_obj_align(humi_container, LV_ALIGN_TOP_RIGHT, -10, 50);
    lv_obj_set_style_bg_color(humi_container, lv_color_hex(UI_COLOR_BG_HUMI), 0);  /* 绿色背景 */
    lv_obj_set_style_bg_opa(humi_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(humi_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *humi_label = lv_label_create(humi_container);
    lv_label_set_text(humi_label, "Humi");
    lv_obj_set_style_text_color(humi_label, lv_color_white(), 0);
    lv_obj_align(humi_label, LV_ALIGN_TOP_MID, 0, -5);

    label_humi = lv_label_create(humi_container);
    lv_label_set_text(label_humi, "60 %");
    lv_obj_set_style_text_color(label_humi, lv_color_white(), 0);
    lv_obj_align(label_humi, LV_ALIGN_BOTTOM_MID, 0, 5);

    /* 土壤湿度容器 - 左下角 */
    lv_obj_t *soil_humi_container = lv_obj_create(scr_main);
    lv_obj_set_size(soil_humi_container, 140, 60);
    lv_obj_align(soil_humi_container, LV_ALIGN_BOTTOM_LEFT, 10, -50);
    lv_obj_set_style_bg_color(soil_humi_container, lv_color_hex(UI_COLOR_BG_SOIL), 0);  /* 金色背景 */
    lv_obj_set_style_bg_opa(soil_humi_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(soil_humi_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *soil_humi_label = lv_label_create(soil_humi_container);
    lv_label_set_text(soil_humi_label, "Soil Humi");
    lv_obj_set_style_text_color(soil_humi_label, lv_color_white(), 0);
    lv_obj_align(soil_humi_label, LV_ALIGN_TOP_MID, 0, -5);

    label_soil_humi = lv_label_create(soil_humi_container);
    lv_label_set_text(label_soil_humi, "40 %");
    lv_obj_set_style_text_color(label_soil_humi, lv_color_white(), 0);
    lv_obj_align(label_soil_humi, LV_ALIGN_BOTTOM_MID, 0, 5);

    /* 光照强度容器 - 右下角 */
    lv_obj_t *light_container = lv_obj_create(scr_main);
    lv_obj_set_size(light_container, 140, 60);
    lv_obj_align(light_container, LV_ALIGN_BOTTOM_RIGHT, -10, -50);
    lv_obj_set_style_bg_color(light_container, lv_color_hex(UI_COLOR_BG_LIGHT), 0);  /* 天蓝色背景 */
    lv_obj_set_style_bg_opa(light_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(light_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *light_label = lv_label_create(light_container);
    lv_label_set_text(light_label, "Light");
    lv_obj_set_style_text_color(light_label, lv_color_white(), 0);
    lv_obj_align(light_label, LV_ALIGN_TOP_MID, 0, -5);

    label_light = lv_label_create(light_container);
    lv_label_set_text(label_light, "80 %");
    lv_obj_set_style_text_color(label_light, lv_color_white(), 0);
    lv_obj_align(label_light, LV_ALIGN_BOTTOM_MID, 0, 5);

    /* 底部模式显示栏 */
    lv_obj_t *mode_container = lv_obj_create(scr_main);
    lv_obj_set_size(mode_container, 320, 30);
    lv_obj_align(mode_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(mode_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);  /* 深蓝色背景 */
    lv_obj_set_style_bg_opa(mode_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(mode_container, LV_OBJ_FLAG_SCROLLABLE);

    label_mode = lv_label_create(mode_container);
    lv_label_set_text(label_mode, "Mode: Auto");
    lv_obj_set_style_text_color(label_mode, lv_color_black(), 0);
    lv_obj_align(label_mode, LV_ALIGN_BOTTOM_MID, 0, 6);
}

/**
 * @brief  更新主屏幕数据
 * @note   刷新传感器数值和模式显示
 * @retval 无
 */
void update_main_screen() {
    lv_label_set_text_fmt(label_temp, "%d C", temp);
    lv_label_set_text_fmt(label_humi, "%d %%", humi);
    lv_label_set_text_fmt(label_soil_humi,"%d %%", soil_humi);
    lv_label_set_text_fmt(label_light, "%d %%", light_intensity);
    lv_label_set_text_fmt(label_mode, mode==1 ?"Mode: Manual":"Mode: Auto");
}

/**
 * @brief  创建菜单屏幕
 * @note   显示功能选项列表
 * @retval 无
 */
void create_menu_screen(void) {
    scr_menu = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_menu, lv_color_hex(UI_COLOR_BG_SCREEN), 0);  /* 浅蓝色背景 */
    lv_obj_set_style_bg_opa(scr_menu, LV_OPA_COVER, 0);

    /* 创建标题栏 */
    lv_obj_t *title_container = lv_obj_create(scr_menu);
    lv_obj_set_size(title_container, 320, 40);
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(title_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Menu");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    /* 菜单选项列表 */
    label_menu = lv_label_create(scr_menu);
    if(mode)
        lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode\n  5. Manual Control");
    else
        lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode");
    lv_obj_set_style_text_color(label_menu, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_menu, &lv_font_montserrat_16, 0);
    lv_obj_align(label_menu, LV_ALIGN_TOP_LEFT, 10, 60);

    /* 底部操作提示栏 */
    lv_obj_t *tip_container = lv_obj_create(scr_menu);
    lv_obj_set_size(tip_container, 320, 30);
    lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *mode_label = lv_label_create(tip_container);
    lv_label_set_text(mode_label, "KEY_UP:Confrim\tKEY0: Down\tKEY1:Up");
    lv_obj_set_style_text_color(mode_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);
    lv_obj_align(mode_label, LV_ALIGN_BOTTOM_MID, 0, 6);
}

/**
 * @brief  更新菜单屏幕
 * @note   刷新菜单选项高亮显示
 * @retval 无
 */
void update_menu_screen() {
    /* 同步菜单项数量（手动模式5项，自动模式4项） */
    menu_item = mode ? 5 : 4;

    /* 确保menu_index不超出范围 */
    if (menu_index >= menu_item) {
        menu_index = 0;
    }

    switch (menu_index) {
        case 0:
            if(mode)
                lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode\n  5. Manual Control");
            else
                lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode");
            break;
        case 1:
            if(mode)
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n> 2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode\n  5. Manual Control");
            else
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n> 2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode");
            break;
        case 2:
            if(mode)
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n  2. Set Humi Limits\n> 3. Set Light Limits\n  4. Set Mode\n  5. Manual Control");
            else
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n  2. Set Humi Limits\n> 3. Set Light Limits\n  4. Set Mode");
            break;
        case 3:
            if(mode)
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n> 4. Set Mode\n  5. Manual Control");
            else
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n> 4. Set Mode");
            break;
        case 4:
            if(mode)
                lv_label_set_text(label_menu, "  1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode\n> 5. Manual Control");
            break;
    }
}

/**
 * @brief  创建阈值设置屏幕
 * @note   根据limit_page显示不同参数的阈值设置界面
 * @retval 无
 */
void create_limit_screen() {
    scr_limit = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_limit, lv_color_hex(UI_COLOR_BG_SCREEN), 0);  /* 浅蓝色背景 */
    lv_obj_set_style_bg_opa(scr_limit, LV_OPA_COVER, 0);

    /* 创建标题栏 */
    lv_obj_t *title_container = lv_obj_create(scr_limit);
    lv_obj_set_size(title_container, 320, 40);
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(title_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, limit_page == 1 ? "Temperature Limits" :
                                     (limit_page == 2 ? "Soil Humidity Limit" : "Light Intensity Limits"));
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    /* 重置选中项索引 */
    limit_index = 0;

    /* 上限设置区域 */
    upper_container = lv_obj_create(scr_limit);
    lv_obj_set_size(upper_container, 280, 40);
    lv_obj_align(upper_container, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_color(upper_container, lv_color_hex(UI_COLOR_BG_UPPER), 0);  /* 浅蓝色背景 */
    lv_obj_set_style_bg_opa(upper_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(upper_container, 3, 0);  /* 选中时边框 */
    lv_obj_set_style_border_color(upper_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);  /* 红色边框表示选中 */
    lv_obj_clear_flag(upper_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_upper = lv_label_create(upper_container);
    lv_label_set_text(label_upper, "Upper");
    lv_obj_set_style_text_color(label_upper, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_upper, &lv_font_montserrat_16, 0);
    lv_obj_align(label_upper, LV_ALIGN_LEFT_MID, 10, 0);

    label_upper_value = lv_label_create(upper_container);
    lv_label_set_text_fmt(label_upper_value, limit_page == 1 ? "%d C" : "%d %%",
                          limit_page == 1 ? lim_value.temp_upper :
                          (limit_page == 2 ? lim_value.shumi_upper : lim_value.light_upper));
    lv_obj_set_style_text_color(label_upper_value, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_upper_value, &lv_font_montserrat_16, 0);
    lv_obj_align(label_upper_value, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 下限设置区域 */
    lower_container = lv_obj_create(scr_limit);
    lv_obj_set_size(lower_container, 280, 40);
    lv_obj_align(lower_container, LV_ALIGN_TOP_MID, 0, 96);
    lv_obj_set_style_bg_color(lower_container, lv_color_hex(UI_COLOR_BG_LOWER), 0);  /* 浅黄色背景 */
    lv_obj_set_style_bg_opa(lower_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(lower_container, 0, 0);  /* 未选中无边框 */
    lv_obj_clear_flag(lower_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_lower = lv_label_create(lower_container);
    lv_label_set_text(label_lower, "Lower");
    lv_obj_set_style_text_color(label_lower, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_lower, &lv_font_montserrat_16, 0);
    lv_obj_align(label_lower, LV_ALIGN_LEFT_MID, 10, 0);

    label_lower_value = lv_label_create(lower_container);
    lv_label_set_text_fmt(label_lower_value, limit_page == 1 ? "%d C" : "%d %%",
                          limit_page == 1 ? lim_value.temp_lower :
                          (limit_page == 2 ? lim_value.shumi_lower : lim_value.light_lower));
    lv_obj_set_style_text_color(label_lower_value, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_lower_value, &lv_font_montserrat_16, 0);
    lv_obj_align(label_lower_value, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 底部操作提示栏 */
    lv_obj_t *tip_container = lv_obj_create(scr_limit);
    lv_obj_set_size(tip_container, 320, 30);
    lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *tip_label = lv_label_create(tip_container);
    lv_label_set_text(tip_label, "KEY_UP:Add\tKEY0:Down\tKEY1:Up");
    lv_obj_set_style_text_color(tip_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(tip_label, &lv_font_montserrat_14, 0);
    lv_obj_align(tip_label, LV_ALIGN_BOTTOM_MID, 0, 6);
}

/**
 * @brief  更新阈值设置屏幕数据
 * @note   刷新数值显示和选中高亮
 * @retval 无
 */
void update_limit_screen() {
    /* 更新数值显示 */
    lv_label_set_text_fmt(label_upper_value,limit_page==1?"%d C":"%d %%",limit_page==1?lim_value.temp_upper:(limit_page==2?lim_value.shumi_upper:lim_value.light_upper));
    lv_label_set_text_fmt(label_lower_value,limit_page==1?"%d C":"%d %%",limit_page==1?lim_value.temp_lower:(limit_page==2?lim_value.shumi_lower:lim_value.light_lower));

    /* 更新选中项高亮边框 */
    lv_obj_set_style_border_width(upper_container, limit_index == 0 ? 3 : 0, 0);
    lv_obj_set_style_border_width(lower_container, limit_index == 1 ? 3 : 0, 0);

    if (limit_index == 0) {
        lv_obj_set_style_border_color(upper_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);
    } else {
        lv_obj_set_style_border_color(lower_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);
    }
}

/**
 * @brief  创建手动控制屏幕
 * @note   显示水泵、补光灯、风扇的手动控制界面（三栏纵向排列）
 * @retval 无
 */
void create_manual_screen(void) {
    scr_manual = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_manual, lv_color_hex(UI_COLOR_BG_SCREEN), 0);  /* 浅蓝色背景 */
    lv_obj_set_style_bg_opa(scr_manual, LV_OPA_COVER, 0);

    /* 创建标题栏 */
    lv_obj_t *title_container = lv_obj_create(scr_manual);
    lv_obj_set_size(title_container, 320, 40);
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(title_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Manual Operation");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    /* 重置选中项索引 */
    manual_index = 0;

    /* 水泵控制区域 - 第一栏 */
    water_container = lv_obj_create(scr_manual);
    lv_obj_set_size(water_container, 280, 40);
    lv_obj_align(water_container, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_color(water_container, lv_color_hex(UI_COLOR_BG_WATER), 0);  /* 蓝色背景 */
    lv_obj_set_style_bg_opa(water_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(water_container, 3, 0);  /* 选中时边框 */
    lv_obj_set_style_border_color(water_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);  /* 红色边框表示选中 */
    lv_obj_clear_flag(water_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_water = lv_label_create(water_container);
    lv_label_set_text(label_water, "Water");
    lv_obj_set_style_text_color(label_water, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_water, &lv_font_montserrat_16, 0);
    lv_obj_align(label_water, LV_ALIGN_LEFT_MID, 10, 0);

    label_water_status = lv_label_create(water_container);
    lv_label_set_text(label_water_status, water_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_water_status, water_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);
    lv_obj_set_style_text_font(label_water_status, &lv_font_montserrat_16, 0);
    lv_obj_align(label_water_status, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 补光灯控制区域 - 第二栏 */
    light_container = lv_obj_create(scr_manual);
    lv_obj_set_size(light_container, 280, 40);
    lv_obj_align(light_container, LV_ALIGN_TOP_MID, 0, 96);
    lv_obj_set_style_bg_color(light_container, lv_color_hex(UI_COLOR_BG_LED), 0);  /* 黄色背景 */
    lv_obj_set_style_bg_opa(light_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(light_container, 0, 0);  /* 未选中无边框 */
    lv_obj_clear_flag(light_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_light = lv_label_create(light_container);
    lv_label_set_text(label_light, "Light");
    lv_obj_set_style_text_color(label_light, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_light, &lv_font_montserrat_16, 0);
    lv_obj_align(label_light, LV_ALIGN_LEFT_MID, 10, 0);

    label_light_status = lv_label_create(light_container);
    lv_label_set_text(label_light_status, light_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_light_status, light_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);
    lv_obj_set_style_text_font(label_light_status, &lv_font_montserrat_16, 0);
    lv_obj_align(label_light_status, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 风扇控制区域 - 第三栏 */
    fan_container = lv_obj_create(scr_manual);
    lv_obj_set_size(fan_container, 280, 40);
    lv_obj_align(fan_container, LV_ALIGN_TOP_MID, 0, 144);
    lv_obj_set_style_bg_color(fan_container, lv_color_hex(UI_COLOR_BG_FAN), 0);  /* 浅绿色背景 */
    lv_obj_set_style_bg_opa(fan_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(fan_container, 0, 0);  /* 未选中无边框 */
    lv_obj_clear_flag(fan_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_fan = lv_label_create(fan_container);
    lv_label_set_text(label_fan, "Fan");
    lv_obj_set_style_text_color(label_fan, lv_color_black(), 0);
    lv_obj_set_style_text_font(label_fan, &lv_font_montserrat_16, 0);
    lv_obj_align(label_fan, LV_ALIGN_LEFT_MID, 10, 0);

    label_fan_status = lv_label_create(fan_container);
    lv_label_set_text(label_fan_status, fun_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_fan_status, fun_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);
    lv_obj_set_style_text_font(label_fan_status, &lv_font_montserrat_16, 0);
    lv_obj_align(label_fan_status, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 底部操作提示栏 */
    lv_obj_t *tip_container = lv_obj_create(scr_manual);
    lv_obj_set_size(tip_container, 320, 30);
    lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(UI_COLOR_BG_TITLE), 0);
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *tip_label = lv_label_create(tip_container);
    lv_label_set_text(tip_label, "KEY_UP:Switch\tKEY0:Down\tKEY1:Up");
    lv_obj_set_style_text_color(tip_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(tip_label, &lv_font_montserrat_14, 0);
    lv_obj_align(tip_label, LV_ALIGN_BOTTOM_MID, 0, 6);
}

/**
 * @brief  更新手动控制屏幕状态
 * @note   刷新三个控制项的状态显示和选中高亮
 * @retval 无
 */
void update_manual_screen() {
    /* 更新水泵状态 */
    lv_label_set_text(label_water_status, water_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_water_status, water_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);

    /* 更新补光灯状态 */
    lv_label_set_text(label_light_status, light_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_light_status, light_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);

    /* 更新风扇状态 */
    lv_label_set_text(label_fan_status, fun_status ? "ON" : "OFF");
    lv_obj_set_style_text_color(label_fan_status, fun_status ? lv_color_hex(UI_COLOR_STATUS_ON) : lv_color_hex(UI_COLOR_STATUS_OFF), 0);

    /* 更新选中项高亮边框 */
    lv_obj_set_style_border_width(water_container, manual_index == 0 ? 3 : 0, 0);
    lv_obj_set_style_border_width(light_container, manual_index == 1 ? 3 : 0, 0);
    lv_obj_set_style_border_width(fan_container, manual_index == 2 ? 3 : 0, 0);

    if (manual_index == 0) {
        lv_obj_set_style_border_color(water_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);
    } else if (manual_index == 1) {
        lv_obj_set_style_border_color(light_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);
    } else {
        lv_obj_set_style_border_color(fan_container, lv_color_hex(UI_COLOR_BORDER_SELECTED), 0);
    }
}

/**
 * @brief  隐藏并销毁弹窗（定时器回调）
 * @param  timer: LVGL定时器指针
 * @retval 无
 */
void hide_and_destroy_popup(lv_timer_t *timer) {
    if (popup_container) {
        lv_obj_del(popup_container);
        popup_container = NULL;
        popup_label = NULL;
    }
    if (popup_timer) {
        lv_timer_del(popup_timer);
        popup_timer = NULL;
    }
}

/**
 * @brief  清理弹窗（切换界面前调用）
 * @note   安全清理弹窗和定时器，防止切换界面后定时器访问无效内存
 * @retval 无
 */
static void cleanup_popup(void) {
    /* 先删除定时器，防止回调访问无效内存 */
    if (popup_timer) {
        lv_timer_del(popup_timer);
        popup_timer = NULL;
    }
    /* 清空指针（容器会随屏幕一起被销毁） */
    popup_container = NULL;
    popup_label = NULL;
}

/**
 * @brief  创建弹窗容器
 * @retval 无
 */
void create_popup(void) {
    if (popup_container) {
        return;
    }
    /* 创建弹窗容器 */
    popup_container = lv_obj_create(current_screen==SCREEN_MAIN?scr_main:(current_screen==SCREEN_MENU)?scr_menu:(current_screen==SCREEN_MANUAL)?scr_manual:scr_limit);
    lv_obj_set_size(popup_container, 200, 40);
    lv_obj_align(popup_container, LV_ALIGN_BOTTOM_RIGHT, -10, -10);  /* 屏幕右下角 */
    lv_obj_set_style_bg_color(popup_container, lv_color_hex(UI_COLOR_BG_POPUP), 0);
    lv_obj_set_style_bg_opa(popup_container, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(popup_container, 8, 0);  /* 圆角样式 */
    lv_obj_clear_flag(popup_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(popup_container, LV_OBJ_FLAG_HIDDEN);  /* 默认隐藏 */

    /* 创建弹窗文本标签 */
    popup_label = lv_label_create(popup_container);
    lv_label_set_text(popup_label, "");
    lv_obj_set_style_text_color(popup_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(popup_label, &lv_font_montserrat_14, 0);
    lv_obj_align(popup_label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief  隐藏弹窗（定时器回调）
 * @param  timer: LVGL定时器指针
 * @retval 无
 */
void hide_popup(lv_timer_t *timer) {
    lv_obj_add_flag(popup_container, LV_OBJ_FLAG_HIDDEN);
    lv_timer_del(timer);
}

/**
 * @brief  显示弹窗
 * @param  message: 弹窗显示的消息文本
 * @param  duration_ms: 弹窗显示时长（毫秒）
 * @retval 无
 */
void show_popup(const char *message, uint32_t duration_ms) {
    lv_label_set_text(popup_label, message);
    lv_obj_clear_flag(popup_container, LV_OBJ_FLAG_HIDDEN);
    /* 设置定时器自动隐藏弹窗 */
    popup_timer = lv_timer_create(hide_and_destroy_popup, duration_ms, NULL);
    lv_timer_set_repeat_count(popup_timer, 1);
}

/**
 * @brief  处理菜单选项选择
 * @param  index: 菜单选项索引 (0-4)
 * @retval 无
 */
void handle_menu_selection(int index) {
    switch (index) {
        case 0:
            /* 进入温度阈值设置界面 */
            limit_page=1;
            destory_active_screen();
            create_limit_screen();
            current_screen = SCREEN_LIMIT;
            lv_scr_load(scr_limit);
            break;
        case 1:
            /* 进入土壤湿度阈值设置界面 */
            limit_page=2;
            destory_active_screen();
            create_limit_screen();
            current_screen = SCREEN_LIMIT;
            lv_scr_load(scr_limit);
            break;
        case 2:
            /* 进入光照强度阈值设置界面 */
            limit_page=3;
            destory_active_screen();
            create_limit_screen();
            current_screen = SCREEN_LIMIT;
            lv_scr_load(scr_limit);
            break;
        case 3:
            /* 切换工作模式（自动/手动） */
            mode=!mode;
            menu_item=mode?5:4;
            menu_index=0;
            destory_active_screen();
            create_menu_screen();
            current_screen = SCREEN_MENU;
            lv_scr_load(scr_menu);
            create_popup();
            show_popup(mode?"Manual Mode":"Auto Mode", 3000);
            break;
        case 4:
            /* 进入手动控制界面 */
            destory_active_screen();
            create_manual_screen();
            current_screen = SCREEN_MANUAL;
            lv_scr_load(scr_manual);
            break;
    }
}

/**
 * @brief  UI切换逻辑
 * @note   根据当前屏幕和按键处理界面切换
 * @param  key: 按键值 (KEY0_PRES, KEY1_PRES, WKUP_PRES, 10=TPAD)
 * @retval 无
 */
void UI_Switch(uint8_t key){
    if(current_screen==SCREEN_MAIN){
        /* 主界面按键处理 */
        switch(key){
            case WKUP_PRES:
                /* 按下KEY_UP进入菜单 */
                destory_active_screen();
                create_menu_screen();
                current_screen = SCREEN_MENU;
                lv_scr_load(scr_menu);
                break;
            case KEY0_PRES:
                break;
            case KEY1_PRES:
                break;
            case 10:
                break;
        }
    }else if(current_screen==SCREEN_MENU){
        /* 菜单界面按键处理 */
        switch(key){
            case WKUP_PRES:
                /* 按下KEY_UP确认当前选项 */
                handle_menu_selection(menu_index);
                menu_index=0;
                break;
            case KEY0_PRES:
                /* 按下KEY0菜单下移 */
                menu_index = (menu_index + 1) % menu_item;
                update_menu_screen();
                break;
            case KEY1_PRES:
                /* 按下KEY1菜单上移 */
                menu_index = (menu_index - 1 + menu_item) % menu_item;
                update_menu_screen();
                break;
            case 10:
                /* 按下TPAD返回主界面 */
                menu_index=0;
                destory_active_screen();
                create_main_screen();
                lv_scr_load(scr_main);
                current_screen = SCREEN_MAIN;
        }
    }else if(current_screen==SCREEN_MANUAL){
        /* 手动控制界面按键处理 */
        switch(key){
            case WKUP_PRES:
                /* 按下KEY_UP切换当前选中项的开关状态 */
                if(manual_index == 0){
                    /* 水泵控制 */
                    if(water_status){
                        water_status = 0;
                        BUMP_OFF;
                    } else {
                        water_status = 1;
                        BUMP_ON;
                    }
                } else if(manual_index == 1){
                    /* 补光灯控制 */
                    if(light_status){
                        light_status = 0;
                        LED1 = 1;
                    } else {
                        light_status = 1;
                        LED1 = 0;
                    }
                } else {
                    /* 风扇控制 */
                    if(fun_status){
                        fun_status = 0;
                        FUN_OFF;
                    } else {
                        fun_status = 1;
                        FUN_ON;
                    }
                }
                update_manual_screen();
                break;
            case KEY0_PRES:
                /* 按下KEY0选择下移 */
                manual_index = (manual_index + 1) % 3;
                update_manual_screen();
                break;
            case KEY1_PRES:
                /* 按下KEY1选择上移 */
                manual_index = (manual_index + 2) % 3;  /* +2 等同于 -1 mod 3 */
                update_manual_screen();
                break;
            case 10:
                /* 按下TPAD返回菜单界面 */
                manual_index = 0;
                destory_active_screen();
                create_menu_screen();
                lv_scr_load(scr_menu);
                current_screen = SCREEN_MENU;
        }
    }else if(current_screen==SCREEN_LIMIT){
        /* 阈值设置界面按键处理 */
        switch(key){
            case WKUP_PRES:
                /* 按下KEY_UP增加当前选中项的值 */
                if(limit_index == 0){
                    /* 增加上限值 */
                    if(limit_page==1){
                        lim_value.temp_upper=(lim_value.temp_upper+2)<=temp_max?lim_value.temp_upper+2:lim_value.temp_lower;
                    }else if(limit_page==2){
                        lim_value.shumi_upper=(lim_value.shumi_upper+10)<=shumi_max?lim_value.shumi_upper+10:lim_value.shumi_lower;
                    }else{
                        lim_value.light_upper=(lim_value.light_upper+10)<=light_max?lim_value.light_upper+10:lim_value.light_lower;
                    }
                }else{
                    /* 增加下限值 */
                    if(limit_page==1){
                        lim_value.temp_lower=(lim_value.temp_lower+2)<=lim_value.temp_upper?lim_value.temp_lower+2:temp_min;
                    }else if(limit_page==2){
                        lim_value.shumi_lower=(lim_value.shumi_lower+10)<=lim_value.shumi_upper?lim_value.shumi_lower+10:shumi_min;
                    }else{
                        lim_value.light_lower=(lim_value.light_lower+10)<=lim_value.light_upper?lim_value.light_lower+10:light_min;
                    }
                }
                update_limit_screen();
                break;
            case KEY0_PRES:
                /* 按下KEY0选择下移 */
                limit_index = (limit_index + 1) % 2;
                update_limit_screen();
                break;
            case KEY1_PRES:
                /* 按下KEY1选择上移 */
                limit_index = (limit_index + 1) % 2;  /* 只有两项，+1即可 */
                update_limit_screen();
                break;
            case 10:
                /* 按下TPAD返回菜单界面 */
                limit_index = 0;
                destory_active_screen();
                create_menu_screen();
                lv_scr_load(scr_menu);
                current_screen = SCREEN_MENU;
                break;
        }
    }
}

/**
 * @brief  告警与自动控制功能
 * @note   根据传感器数据和阈值进行告警显示和自动控制
 * @retval 无
 */
void Warn_function(void){
    if(current_screen!=SCREEN_MAIN)
        return;

    /* 温度告警处理 */
    if(temp>lim_value.temp_upper){
        /* 温度超上限，开启风扇，文字变红 */
				if(!mode&&!fun_status){
            FUN_ON;
            fun_status=1;
        }
        lv_obj_set_style_text_color(label_temp, lv_color_hex(UI_COLOR_WARN_HIGH), 0);
    }else if(temp<lim_value.temp_lower){
        /* 温度低于下限，关闭风扇，文字变蓝 */
        if(!mode&&fun_status){
            FUN_OFF;
            fun_status=0;
        }
        lv_obj_set_style_text_color(label_temp, lv_color_hex(UI_COLOR_WARN_LOW), 0);
    }else{
        /* 温度正常，关闭风扇，文字白色 */
        if(!mode&&fun_status){
            FUN_OFF;
            fun_status=0;
        }
        lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
    }

    /* 空气湿度告警处理 */
    if(humi>lim_value.humi_upper){
        lv_obj_set_style_text_color(label_humi, lv_color_hex(UI_COLOR_WARN_HIGH), 0);
    }else if(humi<lim_value.humi_lower){
        lv_obj_set_style_text_color(label_humi, lv_color_hex(UI_COLOR_WARN_LOW), 0);
    }else{
        lv_obj_set_style_text_color(label_humi, lv_color_white(), 0);
    }

    /* 土壤湿度告警处理及自动浇水 */
    if(soil_humi>lim_value.shumi_upper){
        /* 土壤湿度超上限，自动模式下关闭水泵 */
        if(!mode&&water_status){
            BUMP_OFF;
            water_status=0;
        }
        lv_obj_set_style_text_color(label_soil_humi, lv_color_hex(UI_COLOR_WARN_HIGH), 0);
    }else if(soil_humi<lim_value.shumi_lower){
        /* 土壤湿度低于下限，自动模式下开启水泵 */
        if(!mode&&!water_status){
            BUMP_ON;
            water_status=1;
        }
        lv_obj_set_style_text_color(label_soil_humi, lv_color_hex(UI_COLOR_WARN_LOW), 0);
    }else{
				if(!mode&&water_status){
            BUMP_OFF;
            water_status=0;
        }
        lv_obj_set_style_text_color(label_soil_humi, lv_color_white(), 0);
    }

    /* 光照强度告警处理及自动补光 */
    if(light_intensity>lim_value.light_upper){
        /* 光照超上限，自动模式下关闭补光灯 */
        lv_obj_set_style_text_color(label_light, lv_color_hex(UI_COLOR_WARN_HIGH), 0);
        if(!mode&&light_status){
            LED1=1;
            light_status=0;
        }
    }else if(light_intensity<lim_value.light_lower){
        /* 光照低于下限，自动模式下开启补光灯 */
        lv_obj_set_style_text_color(label_light, lv_color_hex(UI_COLOR_WARN_LOW), 0);
        if(!mode&&!light_status){
            LED1=0;
            light_status=1;
        }
    }else{
				if(!mode&&light_status){
            LED1=1;
            light_status=0;
        }
        lv_obj_set_style_text_color(label_light, lv_color_white(), 0);
    }
}

/**
 * @brief  获取当前屏幕类型
 * @retval screen_t: 当前屏幕枚举值
 */
screen_t get_current_screen(void) {
    return current_screen;
}

/**
 * @brief  设置当前屏幕类型
 * @param  screen: 目标屏幕枚举值
 * @retval 无
 */
void set_current_screen(screen_t screen) {
    current_screen = screen;
}
