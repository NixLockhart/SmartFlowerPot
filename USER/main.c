/**
  ******************************************************************************
  * @file    main.c
  * @author  NixStudio(Nix Lockhart)
  * @version V1.3
  * @date    2025-12-13
  * @brief
  ******************************************************************************
  * @attention
  *
  * 平台:正点原子 STM32F103开发板
  *
  ******************************************************************************
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
#include "ui.h"
#include "dht11.h"
#include "tpad.h"
#include "ts.h"
#include "myserver.h"
#include "atk_mw8266d.h"
#include "atk_mw8266d_uart.h"
#include "bump.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"



//系统初始化
void System_Init() {
	uint8_t ret;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);
	LED_Init();	 						//初始化LED
	Adc_Init();							//初始化Adc
	KEY_Init();							//初始化按键
	tpad_init(6);						//初始化触摸按键
	Lsens_Init();						//初始化光敏传感器
	TS_Init();							//初始化土壤湿度传感器(PA5)
	tp_dev.init();					//初始化触摸屏
	TIM3_Int_Init(71, 999);
	lv_init();							//初始化lvgl
	lv_port_disp_init();
	lv_port_indev_init();
	BUMP_Init();						//初始化水泵继电器(PA7)
	FUN_Init();							//初始化风扇继电器(PA6)
	while (DHT11_Init())		//初始化温湿度传感器(PG11)
	{
		delay_us(500);
	}

	/* 初始化WiFi模块 */
	ret = myserver_wifi_init();
	if (ret != 0)
	{
		wifi_sta = 2;
		printf("WiFi module init failed!\r\n");
	}

	// 初始化UI模块
	UI_Init();
}

//更新传感器数值
void Get_Monitor_Value(void) {
	DHT11_Read_Data(&temp, &humi);   		//温湿度
	TS_GetData(&soil_humi);         		//土壤湿度
	Lsens_Get_Val(&light_intensity);		//光照强度
}

int main(void)
{
	uint8_t t = 0;
	my_sensor_data_t sensor_data;
	my_device_status_t device_status;

	System_Init();
	create_main_screen();

	/* 连接WiFi */
	if (wifi_sta == 2 || myserver_wifi_connect()) {
		create_popup();
		show_popup("WiFi Connect Failed!", 3000);
		wifi_sta = 0;
	}
	else {
		wifi_sta = 1;
		create_popup();
		show_popup("WiFi Connected!", 3000);
	}

	/* 连接服务器 */
	if (wifi_sta) {
		if (myserver_connect() == 0) {
			atkcld_sta = 1;
			create_popup();
			show_popup("Server Connected!", 3000);
		} else {
			atkcld_sta = 0;
			create_popup();
			show_popup("Server Connect Failed!", 3000);
		}
	}

	while (1)
	{
		uint8_t key = KEY_Scan(0);
		UI_Switch(key);

		// 触摸按键检测
		if (tpad_scan(0)) {
			UI_Switch(10);
		}

		Get_Monitor_Value();

		/* 每200ms(t%4==0)执行数据上报 */
		if (t % 4 == 0) {
			Warn_function();

			/* 发送传感器数据到服务器 */
			if (atkcld_sta) {
				sensor_data.temperature = temp;
				sensor_data.humidity = humi;
				sensor_data.soil_humidity = soil_humi;
				sensor_data.light_intensity = light_intensity;
				myserver_send_sensor_data(&sensor_data);

				/* 发送设备状态 */
				device_status.mode = mode;
				device_status.light_status = light_status;
				device_status.water_status = water_status;
				device_status.fan_status = fun_status;
				myserver_send_device_status(&device_status);
			}
		}

		/* 每5秒(t>=99)检测WiFi和服务器连接状态 */
		if (t >= 99) {
			t = 0;
			/* WiFi已连接但服务器未连接，尝试重连服务器 */
			if (wifi_sta == 1 && atkcld_sta == 0) {
				printf("[Reconnect] Trying to reconnect server...\r\n");
				if (myserver_connect() == 0) {
					atkcld_sta = 1;
					create_popup();
					show_popup("Server Reconnected!", 2000);
					printf("[Reconnect] Server reconnected!\r\n");
				}
			}
		}

		/* 处理服务器命令 */
		myserver_wireless_control();

		/* 心跳处理 */
		myserver_process();

		if (get_current_screen() == SCREEN_MAIN) {
			update_main_screen();
		}

		lv_timer_handler();
		delay_ms(50);
		t++;
	}
}
