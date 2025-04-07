/**
 ****************************************************************************************************
 * @file        main.c
 * @author      NixStudio(Nix Lockhart)
 * @version     V1.2
 * @date        2025-04-07
 * @brief       Smart Flower Pot 智能花盆v1.2
 ****************************************************************************************************
 * @attention
 *
 * 实现平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#include "adc.h"
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "24cxx.h"
#include "w25qxx.h"
#include "touch.h"
#include "timer.h"
#include "usart3.h"
#include "lsens.h"
#include "lcd.h"
#include "dht11.h"
#include "tpad.h"
#include "ts.h"
#include "wifi.h"
#include "atk_mw8266d.h"
#include "atk_mw8266d_uart.h"
#include "bump.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_MANUAL,
		SCREEN_LIMIT
} screen_t;

typedef struct{
	uint8_t temp_upper;
	uint8_t temp_lower;
	uint8_t humi_upper;
	uint8_t humi_lower;
	uint8_t shumi_upper;
	uint8_t shumi_lower;
	uint8_t light_upper;
	uint8_t light_lower;
}limits;

limits lim_value;

static screen_t current_screen = SCREEN_MAIN;

/****全局变量****/


//界面变量

static lv_obj_t *scr_main;  					//主界面
static lv_obj_t *scr_menu;  					//菜单界面
static lv_obj_t *scr_manual; 					//手动操作界面

static lv_obj_t *scr_limit; 					//温度上下限界面

static lv_obj_t *label_temp;  					//温度标签
static lv_obj_t *label_humi;  					//湿度标签
static lv_obj_t *label_soil_humi;  			//土壤湿度标签
static lv_obj_t *label_light;  					//光照强度标签
static lv_obj_t *label_mode;  					//模式标签
static lv_obj_t *label_menu;  					//菜单标签
static lv_obj_t *label_light_status; 		//浇水状态标签
static lv_obj_t *label_water_status; 		//补光状态标签

static lv_obj_t *label_lower_value; 		//上限标签
static lv_obj_t *label_upper_value; 		//下限标签

static uint8_t menu_index = 0; 					//菜单选项索引

// 定义弹窗对象
static lv_obj_t *popup_container;
static lv_obj_t *popup_label;

static uint8_t light_status=0;			//浇水状态
static uint8_t water_status=0;			//补光状态

static uint8_t menu_item=4;					//菜单选项数量

static uint8_t temp = 25;						//温度值
static uint8_t humi = 60;						//湿度值
static uint8_t soil_humi = 40;			//土壤湿度值
static uint8_t light_intensity = 80;//环境光强度

static uint8_t mode = 0;						//操作模式{自动、手动}
static uint8_t limit_page = 1;

static uint8_t temp_max = 50;				//环境温度阈值（最大）
static uint8_t temp_min = 0;				//环境温度阈值（最小）
static uint8_t humi_max = 70;				//环境湿度阈值（最大）
static uint8_t humi_min = 40;				//环境湿度阈值（最小）
static uint8_t shumi_max = 65;			//土壤湿度阈值（最大）
static uint8_t shumi_min = 40;			//土壤湿度阈值（最小）
static uint8_t light_max = 100;			//环境光强阈值（最大）
static uint8_t light_min = 30;			//环境光强阈值（最小）
static uint8_t wifi_sta=0; 					//Wifi连接状态
//wifi_sta 0: 断开
//         1: 已连接
//				 2: 模块初始化失败
static uint8_t atkcld_sta=0;				//原子云连接状态
//atkcld_sta	0:断开
//						1:连接

void System_Init(){
	lim_value.temp_upper=30;
	lim_value.temp_lower=10;
	lim_value.humi_upper=70;
	lim_value.humi_lower=40; 
	lim_value.shumi_upper=65;
	lim_value.shumi_lower=40;
	lim_value.light_upper=90;
	lim_value.light_lower=50;
	uint8_t ret;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);
 	LED_Init();	 							//初始化LED	
 	Adc_Init();								//初始化Adc
	KEY_Init();								//初始化按键
	tpad_init(6);							//初始化触控按键
	Lsens_Init();							//初始化光敏传感器
	TS_Init();								//初始化土壤湿度传感器(PA5)
	tp_dev.init();						//初始化触摸屏
	TIM3_Int_Init(71,999);
	lv_init();								//初始化lvgl
	lv_port_disp_init();
	lv_port_indev_init();
	BUMP_Init();							//初始化水泵继电器(PA7)
	FUN_Init();								//初始化风扇继电器(PA6)
	while(DHT11_Init())				 //初始化环境温湿度传感器(PG11)
	{
		delay_us(500);
	}
	ret = atk_mw8266d_init(115200);
	if (ret != 0)
	{
		wifi_sta=2;
		printf("ATK-MW8266D Init Failed!\r\n");
	}
}

//更新传感器数值
void Get_Monitor_Value(void){
		DHT11_Read_Data(&temp,&humi);   		//环境温湿度
		TS_GetData(&soil_humi);         		//土壤湿度
		Lsens_Get_Val(&light_intensity);		//环境光照
}

//删除所有子界面
void destroy_all_children(lv_obj_t *parent) {
    lv_obj_t *child = lv_obj_get_child(parent, NULL);
    while (child) {
        lv_obj_del(child);
        child = lv_obj_get_child(parent, NULL);
    }
}

//删除活动页面
void destory_active_screen(){
	lv_obj_t *active_screen = lv_scr_act();
	destroy_all_children(active_screen);
}

//创建主界面
void create_main_screen(void) {
    scr_main = lv_scr_act();
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(0xADD8E6), 0);  //背景为浅蓝色
    lv_obj_set_style_bg_opa(scr_main, LV_OPA_COVER, 0);

    //顶部标题框
    lv_obj_t *title_container = lv_obj_create(scr_main);
    lv_obj_set_size(title_container, 320, 40);  // 设置标题框大小（屏幕宽度为320）
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(title_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Smart Flower Pot");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);  // 白色文字
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);  // 字体字号设置
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);  // 标题居中

    //数据区域：方块布局
    //使用格子布局，将数据分布为4个方块

    //温度
    lv_obj_t *temp_container = lv_obj_create(scr_main);
    lv_obj_set_size(temp_container, 140, 60);  // 方块大小
    lv_obj_align(temp_container, LV_ALIGN_TOP_LEFT, 10, 50);  // 左上方对齐
    lv_obj_set_style_bg_color(temp_container, lv_color_hex(0xFF7F7F), 0);  // 红色背景
    lv_obj_set_style_bg_opa(temp_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(temp_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *temp_label = lv_label_create(temp_container);
    lv_label_set_text(temp_label, "Temp");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_align(temp_label, LV_ALIGN_TOP_MID, 0, -5);  // 标签位置

    label_temp = lv_label_create(temp_container);
    lv_label_set_text(label_temp, "25 C");
    lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
    lv_obj_align(label_temp, LV_ALIGN_BOTTOM_MID, 0, 5);  // 数值位置

    //湿度
    lv_obj_t *humi_container = lv_obj_create(scr_main);
    lv_obj_set_size(humi_container, 140, 60);  // 方块大小
    lv_obj_align(humi_container, LV_ALIGN_TOP_RIGHT, -10, 50);  // 右上方对齐
    lv_obj_set_style_bg_color(humi_container, lv_color_hex(0x00AA00), 0);  // 绿色背景
    lv_obj_set_style_bg_opa(humi_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(humi_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *humi_label = lv_label_create(humi_container);
    lv_label_set_text(humi_label, "Humi");
    lv_obj_set_style_text_color(humi_label, lv_color_white(), 0);
    lv_obj_align(humi_label, LV_ALIGN_TOP_MID, 0, -5);  // 标签位置

    label_humi = lv_label_create(humi_container);
    lv_label_set_text(label_humi, "60 %");
    lv_obj_set_style_text_color(label_humi, lv_color_white(), 0);
    lv_obj_align(label_humi, LV_ALIGN_BOTTOM_MID, 0, 5);  // 数值位置

    //土壤湿度
    lv_obj_t *soil_humi_container = lv_obj_create(scr_main);
    lv_obj_set_size(soil_humi_container, 140, 60);  // 方块大小
    lv_obj_align(soil_humi_container, LV_ALIGN_BOTTOM_LEFT, 10, -50);  // 左下方对齐
    lv_obj_set_style_bg_color(soil_humi_container, lv_color_hex(0xFFD700), 0);  // 黄色背景
    lv_obj_set_style_bg_opa(soil_humi_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(soil_humi_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *soil_humi_label = lv_label_create(soil_humi_container);
    lv_label_set_text(soil_humi_label, "Soil Humi");
    lv_obj_set_style_text_color(soil_humi_label, lv_color_white(), 0);
    lv_obj_align(soil_humi_label, LV_ALIGN_TOP_MID, 0, -5);  // 标签位置

    label_soil_humi = lv_label_create(soil_humi_container);
    lv_label_set_text(label_soil_humi, "40 %");
    lv_obj_set_style_text_color(label_soil_humi, lv_color_white(), 0);
    lv_obj_align(label_soil_humi, LV_ALIGN_BOTTOM_MID, 0, 5);  // 数值位置

    //光照强度
    lv_obj_t *light_container = lv_obj_create(scr_main);
    lv_obj_set_size(light_container, 140, 60);  // 方块大小
    lv_obj_align(light_container, LV_ALIGN_BOTTOM_RIGHT, -10, -50);  // 右下方对齐
    lv_obj_set_style_bg_color(light_container, lv_color_hex(0x00BFFF), 0);  // 蓝色背景
    lv_obj_set_style_bg_opa(light_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(light_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *light_label = lv_label_create(light_container);
    lv_label_set_text(light_label, "Light");
    lv_obj_set_style_text_color(light_label, lv_color_white(), 0);
    lv_obj_align(light_label, LV_ALIGN_TOP_MID, 0, -5);  // 标签位置

    label_light = lv_label_create(light_container);
    lv_label_set_text(label_light, "80 %");
    lv_obj_set_style_text_color(label_light, lv_color_white(), 0);
    lv_obj_align(label_light, LV_ALIGN_BOTTOM_MID, 0, 5);  // 数值位置

    //底部模式显示
		lv_obj_t *mode_container = lv_obj_create(scr_main);
		lv_obj_set_size(mode_container, 320, 30);  // 方块大小
		lv_obj_align(mode_container, LV_ALIGN_BOTTOM_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(mode_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(mode_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(mode_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能
		
    lv_obj_t *mode_label = lv_label_create(mode_container);
    lv_label_set_text(mode_label, "Mode: ");
    lv_obj_set_style_text_color(mode_label, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(mode_label, LV_ALIGN_BOTTOM_MID, mode?-30:-20, 6);  // 底部居中对齐
		
		label_mode = lv_label_create(mode_container);
    lv_label_set_text(label_mode, "Auto");
    lv_obj_set_style_text_color(label_mode, lv_color_black(), 0);
    lv_obj_align(label_mode, LV_ALIGN_BOTTOM_MID, mode?23:18, 6);  // 数值位置
}

//更新主界面数据
void update_main_screen() {
    lv_label_set_text_fmt(label_temp, "%d C", temp);
    lv_label_set_text_fmt(label_humi, "%d %%", humi);
    lv_label_set_text_fmt(label_soil_humi,"%d %%", soil_humi);
    lv_label_set_text_fmt(label_light, "%d %%", light_intensity);
		lv_label_set_text_fmt(label_mode, mode==1 ?"Manual":"Auto");
}

//创建菜单界面
void create_menu_screen(void) {
    scr_menu = lv_obj_create(NULL);  // 创建新的屏幕
    lv_obj_set_style_bg_color(scr_menu, lv_color_hex(0xADD8E6), 0);  // 设置背景为浅蓝色
    lv_obj_set_style_bg_opa(scr_menu, LV_OPA_COVER, 0);

    //顶部标题框
    lv_obj_t *title_container = lv_obj_create(scr_menu);
    lv_obj_set_size(title_container, 320, 40);  // 设置标题框大小（屏幕宽度为320）
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(title_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Menu");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);  // 白色文字
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);  // 字号
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);  // 标题居中

    //菜单选项
    //使用多行标签显示菜单选项
    label_menu = lv_label_create(scr_menu);
	if(mode)
		lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode\n  5. Manual Control");
	else
		lv_label_set_text(label_menu, "> 1. Set Temp Limits\n  2. Set Humi Limits\n  3. Set Light Limits\n  4. Set Mode");
    lv_obj_set_style_text_color(label_menu, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_menu, &lv_font_montserrat_16, 0);  // 字号
    lv_obj_align(label_menu, LV_ALIGN_TOP_LEFT, 10, 60);  // 菜单放置在顶部下方
	
		//底部按键提示
		lv_obj_t *tip_container = lv_obj_create(scr_menu);
		lv_obj_set_size(tip_container, 320, 30);  // 方块大小
		lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能
		
    lv_obj_t *mode_label = lv_label_create(tip_container);
		lv_label_set_text(mode_label, "KEY_UP:Confrim\tKEY0: Down\tKEY1:Up");
    lv_obj_set_style_text_color(mode_label, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(mode_label, LV_ALIGN_BOTTOM_MID, 0, 6);  // 底部居中对齐
}

//更新菜单界面
void update_menu_screen() {
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

//创建上下限界面
void create_limit_screen() {
    // 创建页面，并设置背景为浅蓝色
    scr_limit = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_limit, lv_color_hex(0xADD8E6), 0);  // 浅蓝色背景
    lv_obj_set_style_bg_opa(scr_limit, LV_OPA_COVER, 0);

    // === 顶部标题框 ===
    lv_obj_t *title_container = lv_obj_create(scr_limit);
    lv_obj_set_size(title_container, 320, 40);  // 标题框大小（宽320，高40）
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(title_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, limit_page == 1 ? "Temperature Limits" :
                                     (limit_page == 2 ? "Soil Humidity Limit" : "Light Intensity Limits"));
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);  // 白色文字
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);  // 字体字号
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);  // 居中放置

    // === 上限板块 ("Upper") ===
    lv_obj_t *upper_container = lv_obj_create(scr_limit);
    lv_obj_set_size(upper_container, 200, 60);  // 板块大小（宽200，高60）
    lv_obj_align(upper_container, LV_ALIGN_TOP_MID, 0, 60);  // 在顶部下方居中对齐
    lv_obj_set_style_bg_color(upper_container, lv_color_hex(0x87CEEB), 0);  // 浅蓝色背景
    lv_obj_set_style_bg_opa(upper_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(upper_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动

    // 左侧显示 "Upper" 标签
    lv_obj_t *label_upper = lv_label_create(upper_container);
    lv_label_set_text(label_upper, "Upper");  // 上限标签
    lv_obj_set_style_text_color(label_upper, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_upper, &lv_font_montserrat_16, 0);  // 字号
    lv_obj_align(label_upper, LV_ALIGN_LEFT_MID, 10, 0);  // 左侧对齐，距左10px

    // 右侧显示当前上限值
    label_upper_value = lv_label_create(upper_container);
    lv_label_set_text_fmt(label_upper_value, limit_page == 1 ? "%d C" : "%d %%", 
                          limit_page == 1 ? lim_value.temp_upper :
                          (limit_page == 2 ? lim_value.humi_upper : lim_value.light_upper));  // 默认上限值
    lv_obj_set_style_text_color(label_upper_value, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_upper_value, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(label_upper_value, LV_ALIGN_RIGHT_MID, -10, 0);  // 右侧对齐，距右10px

    // === 下限板块 ("Lower") ===
    lv_obj_t *lower_container = lv_obj_create(scr_limit);
    lv_obj_set_size(lower_container, 200, 60);  // 板块大小（宽200，高60）
    lv_obj_align(lower_container, LV_ALIGN_TOP_MID, 0, 130);  // 在上限板块下方对齐
    lv_obj_set_style_bg_color(lower_container, lv_color_hex(0xFFEC8B), 0);  // 浅黄色背景
    lv_obj_set_style_bg_opa(lower_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(lower_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动

    // 左侧显示 "Lower" 标签
    lv_obj_t *label_lower = lv_label_create(lower_container);
    lv_label_set_text(label_lower, "Lower");  // 下限标签
    lv_obj_set_style_text_color(label_lower, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_lower, &lv_font_montserrat_16, 0);  // 字号
    lv_obj_align(label_lower, LV_ALIGN_LEFT_MID, 10, 0);  // 左侧对齐，距左10px

    // 右侧显示当前下限值
    label_lower_value = lv_label_create(lower_container);
    lv_label_set_text_fmt(label_lower_value, limit_page == 1 ? "%d C" : "%d %%", 
                          limit_page == 1 ? lim_value.temp_lower :
                          (limit_page == 2 ? lim_value.humi_lower : lim_value.light_lower));  // 默认下限值
    lv_obj_set_style_text_color(label_lower_value, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_lower_value, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(label_lower_value, LV_ALIGN_RIGHT_MID, -10, 0);  // 右侧对齐，距右10px
		
		//底部按键提示
		lv_obj_t *tip_container = lv_obj_create(scr_limit);
		lv_obj_set_size(tip_container, 320, 30);  // 方块大小
		lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能
		
    lv_obj_t *mode_label = lv_label_create(tip_container);
		lv_label_set_text(mode_label, "KEY0: Lower+\tKEY1:Upper+");
    lv_obj_set_style_text_color(mode_label, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(mode_label, LV_ALIGN_BOTTOM_MID, 0, 6);  // 底部居中对齐
}


//更新上下限界面数据
void update_limit_value() {	
		lv_label_set_text_fmt(label_upper_value,limit_page==1?"%d C":"%d %%",limit_page==1?lim_value.temp_upper:(limit_page==2?lim_value.shumi_upper:lim_value.light_upper));  // 更新上限值
		lv_label_set_text_fmt(label_lower_value,limit_page==1?"%d C":"%d %%",limit_page==1?lim_value.temp_lower:(limit_page==2?lim_value.shumi_lower:lim_value.light_lower));  // 更新下限值
}



//创建手动操作界面
void create_manual_screen(void) {
     // 创建手动操作屏幕，并设置背景底色为浅蓝色
    scr_manual = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_manual, lv_color_hex(0xADD8E6), 0);  // 浅蓝色背景
    lv_obj_set_style_bg_opa(scr_manual, LV_OPA_COVER, 0);

    // === 顶部标题框 ===
    lv_obj_t *title_container = lv_obj_create(scr_manual);
    lv_obj_set_size(title_container, 320, 40);  // 标题框大小（宽320，高40）
    lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(title_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(title_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(title_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动

    lv_obj_t *title_label = lv_label_create(title_container);
    lv_label_set_text(title_label, "Manual Operation");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);  // 白色文字
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);  // 字体字号
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);  // 居中放置

    // === 浇水板块 ("Water") ===
    lv_obj_t *water_container = lv_obj_create(scr_manual);
    lv_obj_set_size(water_container, 140, 100);  // 板块大小
    lv_obj_align(water_container, LV_ALIGN_LEFT_MID, 20, 0);  // 左侧居中对齐
    lv_obj_set_style_bg_color(water_container, lv_color_hex(0x87CEEB), 0);  // 蓝色背景
    lv_obj_set_style_bg_opa(water_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(water_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动

    // 显示 "Water" 标签
    lv_obj_t *label_water = lv_label_create(water_container);
    lv_label_set_text(label_water, "Water");
    lv_obj_set_style_text_color(label_water, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_water, &lv_font_montserrat_16, 0);  // 字号
    lv_obj_align(label_water, LV_ALIGN_TOP_MID, 0, 10);  // 放置在板块顶部

    // 显示状态 "Open" 或 "Close"
    label_water_status = lv_label_create(water_container);
    lv_label_set_text(label_water_status, water_status?"Open":"Close");  // 默认状态为 Close
    lv_obj_set_style_text_color(label_water_status, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_water_status, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(label_water_status, LV_ALIGN_BOTTOM_MID, 0, -10);  // 放置在板块底部

    // === 补光板块 ("Light") ===
    lv_obj_t *light_container = lv_obj_create(scr_manual);
    lv_obj_set_size(light_container, 140, 100);  // 板块大小
    lv_obj_align(light_container, LV_ALIGN_RIGHT_MID, -20, 0);  // 右侧居中对齐
    lv_obj_set_style_bg_color(light_container, lv_color_hex(0xFFEC8B), 0);  // 黄色背景
    lv_obj_set_style_bg_opa(light_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(light_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动

    // 显示 "Light" 标签
    lv_obj_t *label_light = lv_label_create(light_container);
    lv_label_set_text(label_light, "Light");
    lv_obj_set_style_text_color(label_light, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_light, &lv_font_montserrat_16, 0);  // 字号
    lv_obj_align(label_light, LV_ALIGN_TOP_MID, 0, 10);  // 放置在板块顶部

    // 显示状态 "Open" 或 "Close"
    label_light_status = lv_label_create(light_container);
    lv_label_set_text(label_light_status, light_status?"Open":"Close");  // 默认状态为 Close
    lv_obj_set_style_text_color(label_light_status, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(label_light_status, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(label_light_status, LV_ALIGN_BOTTOM_MID, 0, -10);  // 放置在板块底部
		
				//底部按键提示
		lv_obj_t *tip_container = lv_obj_create(scr_limit);
		lv_obj_set_size(tip_container, 320, 30);  // 方块大小
		lv_obj_align(tip_container, LV_ALIGN_BOTTOM_MID, 0, 0);  // 顶部居中对齐
    lv_obj_set_style_bg_color(tip_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(tip_container, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tip_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能
		
    lv_obj_t *mode_label = lv_label_create(tip_container);
		lv_label_set_text(mode_label, "KEY0: Water\tKEY1:Light");
    lv_obj_set_style_text_color(mode_label, lv_color_black(), 0);  // 黑色文字
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(mode_label, LV_ALIGN_BOTTOM_MID, 0, 6);  // 底部居中对齐
}

//处理手动操作
void update_manual_screen() {
		lv_label_set_text(label_light_status, light_status?"Open":"Close");
		lv_label_set_text(label_water_status, water_status?"Open":"Close");
}

//销毁弹窗
void hide_and_destroy_popup(lv_timer_t *timer) {
    if (popup_container) {
        lv_obj_del(popup_container);  // 销毁弹窗容器及其子对象
        popup_container = NULL;      // 设置为 NULL，防止重复销毁
        popup_label = NULL;          // 同样清空标签指针
    }
    lv_timer_del(timer);  // 销毁定时器
}

//创建弹窗
void create_popup(void) {
		if (popup_container) {
        return;
    }
    // 创建弹窗容器
    popup_container = lv_obj_create(current_screen==SCREEN_MAIN?scr_main:(current_screen==SCREEN_MENU)?scr_menu:(current_screen==SCREEN_MANUAL)?scr_manual:scr_limit);  // 添加弹窗
    lv_obj_set_size(popup_container, 200, 40);  // 设置弹窗大小 (宽200，高40)
    lv_obj_align(popup_container, LV_ALIGN_BOTTOM_RIGHT, -10, -10);  // 屏幕右下角，距离底部和右侧边缘10像素
    lv_obj_set_style_bg_color(popup_container, lv_color_hex(0x4682B4), 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(popup_container, LV_OPA_COVER, 0);  // 不透明背景
    lv_obj_set_style_radius(popup_container, 8, 0);  // 圆角样式
    lv_obj_clear_flag(popup_container, LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动功能
    lv_obj_add_flag(popup_container, LV_OBJ_FLAG_HIDDEN);  // 默认隐藏弹窗

    // 创建弹窗文本标签
    popup_label = lv_label_create(popup_container);
    lv_label_set_text(popup_label, "");  // 默认无文字
    lv_obj_set_style_text_color(popup_label, lv_color_white(), 0);  // 白色文字
    lv_obj_set_style_text_font(popup_label, &lv_font_montserrat_14, 0);  // 字号
    lv_obj_align(popup_label, LV_ALIGN_CENTER, 0, 0);  // 居中对齐
}

//销毁弹窗
void hide_popup(lv_timer_t *timer) {
    lv_obj_add_flag(popup_container, LV_OBJ_FLAG_HIDDEN);  // 隐藏弹窗
    lv_timer_del(timer);  // 删除定时器
}

//显示弹窗
void show_popup(const char *message, uint32_t duration_ms) {
		lv_label_set_text(popup_label, message);  // 设置弹窗内容
		lv_obj_clear_flag(popup_container, LV_OBJ_FLAG_HIDDEN);  // 显示弹窗
		// 设置定时器，自动隐藏弹窗
		lv_timer_t *timer = lv_timer_create(hide_and_destroy_popup, duration_ms, NULL);
		lv_timer_set_repeat_count(timer, 1);  // 只运行一次
}


//处理菜单选择
void handle_menu_selection(int index) {
    switch (index) {
			case 0:
					// 进入设置温度上下限界面
					limit_page=1;
					destory_active_screen();
					create_limit_screen();
					current_screen = SCREEN_LIMIT;
					lv_scr_load(scr_limit);
					break;
			case 1:
					// 进入设置湿度上下限界面
					limit_page=2;
					destory_active_screen();
					create_limit_screen();
					current_screen = SCREEN_LIMIT;
					lv_scr_load(scr_limit);
					break;
			case 2:
					// 进入设置光强上下限界面
					limit_page=3;
					destory_active_screen();
					create_limit_screen();
					current_screen = SCREEN_LIMIT;
					lv_scr_load(scr_limit);
					break;
			case 3:
					//切换操作模式
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
					//进入手动操作界面
					destory_active_screen();
					create_manual_screen();
					current_screen = SCREEN_MANUAL;
					lv_scr_load(scr_manual);
					break;
    }
}

//界面交互逻辑
void UI_Switch(uint8_t key){
	if(current_screen==SCREEN_MAIN){	//主界面下按钮操作
		switch(key){
			case WKUP_PRES:								//按下Key_up进入操作菜单
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
	}else if(current_screen==SCREEN_MENU){	//菜单界面下按钮操作
		switch(key){
			case WKUP_PRES:											//按下Key_up进入当前选中功能
				handle_menu_selection(menu_index);
				menu_index=0;
				break;
			case KEY0_PRES:											//按下Key0菜单下移
				menu_index = (menu_index + 1) % menu_item; 
				update_menu_screen();
				break;
			case KEY1_PRES:											//按下Key1菜单上移
				menu_index = (menu_index - 1 + menu_item) % menu_item;
				update_menu_screen();
				break;
			case 10:														//按下TPAD返回上一页面（主页面）
				menu_index=0;
				destory_active_screen();
				create_main_screen();
				lv_scr_load(scr_main);
				current_screen = SCREEN_MAIN;
		}
	}else if(current_screen==SCREEN_MANUAL){ //手动页面下按钮操作
		switch(key){
			case WKUP_PRES:
				break;
			case KEY0_PRES:											//按下Key1开关浇水
				if(water_status){
					water_status=0;
					BUMP_OFF;
				}
				else{
					water_status=1;
					BUMP_ON;
				}
				update_manual_screen();
				break;
			case KEY1_PRES:											//按下Key1开关补光
				if(light_status){
					light_status=0;
					LED1 = 1;
				}
				else{
					light_status=1;
					LED1 = 0;
				}
				update_manual_screen();
				break;
			case 10:														//按下TPAD返回上一页面（菜单）
				destory_active_screen();
				create_menu_screen();
				lv_scr_load(scr_menu);
				current_screen = SCREEN_MENU;
		}
	}else if(current_screen==SCREEN_LIMIT){	//上下限调节页面下按钮操作
		switch(key){
			case WKUP_PRES:
				break;
			case KEY1_PRES:	
				if(limit_page==1){				//按下Key1调节上限
					lim_value.temp_upper=(lim_value.temp_upper+10)<=temp_max?lim_value.temp_upper+10:temp_min;
				}else if(limit_page==2){
					lim_value.shumi_upper=(lim_value.shumi_upper+10)<=shumi_max?lim_value.shumi_upper+10:shumi_min;
				}else{
					lim_value.light_upper=(lim_value.light_upper+10)<=light_max?lim_value.light_upper+10:light_min;
				}
				update_limit_value();
				break;
			case KEY0_PRES:							//按下Key0调节下限
				if(limit_page==1){
					lim_value.temp_lower=(lim_value.temp_lower+10)<=temp_max?lim_value.temp_lower+10:temp_min;
				}else if(limit_page==2){
					lim_value.shumi_lower=(lim_value.shumi_lower+10)<=shumi_max?lim_value.shumi_lower+10:shumi_min;
				}else{
					lim_value.light_lower=(lim_value.light_lower+10)<=light_max?lim_value.light_lower+10:light_min;
				}
				update_limit_value();
				break;
			case 10:										//按下TPAD返回上一页面（菜单）
				destory_active_screen();
				create_menu_screen();
				lv_scr_load(scr_menu);
				current_screen = SCREEN_MENU;
				break;
		}
	}
}

//报警功能
void Warn_function(void){
	if(current_screen!=SCREEN_MAIN)
		return;
	if(temp>lim_value.temp_upper){
		//开启风扇
			FUN_ON;
			lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFF0000), 0);
	}else if(temp<lim_value.temp_lower){
			FUN_OFF;
			lv_obj_set_style_text_color(label_temp, lv_color_hex(0x0000FF), 0);
	}else{
			FUN_OFF;
			lv_obj_set_style_text_color(label_temp, lv_color_white(), 0);
	}
	if(humi>lim_value.humi_upper){
			lv_obj_set_style_text_color(label_humi, lv_color_hex(0xFF0000), 0);
	}else if(humi<lim_value.humi_lower){
			lv_obj_set_style_text_color(label_humi, lv_color_hex(0x0000FF), 0);
	}else{
			lv_obj_set_style_text_color(label_humi, lv_color_white(), 0);
	}
	if(soil_humi>lim_value.shumi_upper){
			if(!mode&&water_status){
				BUMP_OFF;
				water_status=0;
			}
			lv_obj_set_style_text_color(label_soil_humi, lv_color_hex(0xFF0000), 0);
	}else if(soil_humi<lim_value.shumi_lower){
			if(!mode&&!water_status){
				BUMP_ON;
				water_status=1;
			}
			lv_obj_set_style_text_color(label_soil_humi, lv_color_hex(0x0000FF), 0);
	}else{
			lv_obj_set_style_text_color(label_soil_humi, lv_color_white(), 0);
	}
	if(light_intensity>lim_value.light_upper){
			lv_obj_set_style_text_color(label_light, lv_color_hex(0xFF0000), 0);
			if(!mode&&light_status){
				LED1=1;
				light_status=0;
			}
	}else if(light_intensity<lim_value.light_lower){
			lv_obj_set_style_text_color(label_light, lv_color_hex(0x0000FF), 0);
			if(!mode&&!light_status){
				LED1=0;
				light_status=1;
			}
	}else{
			lv_obj_set_style_text_color(label_light, lv_color_white(), 0);
	}
}

//原子云无线控制
void wireless_control(){
	uint8_t exe=execute();
	if(!mode&&exe!=8){
		return;
	}
	if(exe==1){
		light_status=1;
		LED1=0;
		create_popup();
		show_popup("Light ON", 2000);
	}else if(exe==2){
		light_status=0;
		LED1=1;
		create_popup();
		show_popup("Light OFF", 2000);
	}else if(exe==3){
		water_status=1;
		BUMP_ON;
		create_popup();
		show_popup("Water ON", 2000);
	}else if(exe==4){
		water_status=0;
		BUMP_OFF;
		create_popup();
		show_popup("Water OFF", 2000);
	}else if(exe==5){
		FUN_ON;
		create_popup();
		show_popup("Fun ON", 2000);
	}else if(exe==6){
		FUN_OFF;
		create_popup();
		show_popup("Fun OFF", 2000);
	}else if(exe==7){
		mode=0;
		menu_item=mode?5:4;
		menu_index=0;
		if(current_screen==SCREEN_MAIN){
			update_main_screen();
		}else if(current_screen==SCREEN_MENU){
			update_menu_screen();
		}
		create_popup();
		show_popup(mode?"Manual Mode":"Auto Mode", 2000);
	}else if(exe==8){
		mode=1;
		menu_item=mode?5:4;
		menu_index=0;
		if(current_screen==SCREEN_MAIN){
			update_main_screen();
		}else if(current_screen==SCREEN_MENU){
			update_menu_screen();
		}
		create_popup();
		show_popup(mode?"Manual Mode":"Auto Mode", 2000);
	}else{
		return;
	}
	if(current_screen==SCREEN_MANUAL)
		update_manual_screen();
}


int main(void)
{
		uint8_t t=0;
		System_Init();
		create_main_screen();
		if(wifi_sta==2||wifi_connect()){
			create_popup();
			show_popup("WiFi Connect Failed!", 3000);
			wifi_sta=0;
		}else{
			wifi_sta=1;
			create_popup();
			show_popup("WiFi Connected!", 3000);
		}
		if(wifi_sta){
			atkcld_con(&atkcld_sta);
		}
		while(1)
		{
			uint8_t key = KEY_Scan(0);
			UI_Switch(key);
      // 触控按钮返回
			if(tpad_scan(0)){
				UI_Switch(10);
			}
			Get_Monitor_Value();
			if(t==4){
				Warn_function();
				if(atkcld_sta)
					send_data_to_cloud(atkcld_sta,temp,humi,soil_humi,light_intensity);
				t=0;
			}
			wireless_control();
			if(current_screen == SCREEN_MAIN) {
				update_main_screen();
			}
			lv_timer_handler();
			delay_ms(50);
			t++;
		}
}
