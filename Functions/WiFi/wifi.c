/**
 ****************************************************************************************************
 * @file        wifi.c
 * @author      NixStudio(NixLockhart)
 * @version     V1.1
 * @date        2025-06-08
 * @brief       ATK-MW8266D模块与原子云通信
 ****************************************************************************************************
 * @attention
 *
 * 平台:正点原子 STM32F103开发板
 *
 ****************************************************************************************************
 */

#include <string.h>
#include "wifi.h"
#include "atk_mw8266d.h"
#include "atk_mw8266d_uart.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"

#define DEMO_WIFI_SSID          "Nix_Wifi"	//自己的WiFi名
#define DEMO_WIFI_PWD           "12345678"	//WiFi密码
#define DEMO_ATKCLD_DEV_ID      "31366612859960558514"	//原子云设备编号 网址：https://cloud.alientek.com/
#define DEMO_ATKCLD_DEV_PWD     "12345678"							//原子云设备密码


// JSON格式发送数据
void send_data_to_cloud(uint8_t atkcld,uint8_t temp,uint8_t humi,uint8_t soil_humi,uint8_t light_ind){
    if (atkcld == 1){ 					// 确保已连接到原子云
        char json_buffer[128]; 	// JSON字符串缓冲区
        // 构造JSON格式的数据
        snprintf(json_buffer, sizeof(json_buffer), "{\"temperature\": %d, \"humidity\": %d, \"Soil Humi\": %d, \"Light\": %d}", temp, humi, soil_humi, light_ind);
        // 发送数据到原子云
        atk_mw8266d_uart_printf("%s\r\n", json_buffer);
    }
    else{
        printf("Not connected to ALIENTEK cloud!\r\n");
    }
}

// 连接原子云
void atkcld_con(uint8_t *is_atkcld)
{
    uint8_t ret;
    
    if (*is_atkcld == 0)
    {
        /* 连接原子云 */
        ret = atk_mw8266d_connect_atkcld(DEMO_ATKCLD_DEV_ID, DEMO_ATKCLD_DEV_PWD);
        if (ret == 0)
        {
            *is_atkcld = 1;
            printf("Connect to ALIENTEK cloud!\r\n");
        }
        else
        {
            printf("Error to connect ALIENTEK cloud!\r\n");
        }
    }
    else
    {
        /* 断开原子云连接 */
        atk_mw8266d_disconnect_atkcld();
        *is_atkcld = 0;
        printf("Disconnect to ALIENTEK cloud!\r\n");
    }
}

//接收信息并返回对应操作码
uint8_t execute()
{
    uint8_t *buf;
    /* 接收来自ATK-MW8266D UART的一帧数据 */
		buf = atk_mw8266d_uart_rx_get_frame();
		if (buf != NULL){
			printf("%s", buf);
			
			// 检查字符串内容
			if (strstr((char *)buf, "light on") != NULL){				//开灯1
					atk_mw8266d_uart_rx_restart();
					return 1;
			}
			else if (strstr((char *)buf, "light off") != NULL){	//关灯2
					atk_mw8266d_uart_rx_restart();
					return 2;
			}
			else if (strstr((char *)buf, "water on") != NULL){	//开水3
					atk_mw8266d_uart_rx_restart();
					return 3;
			}
			else if (strstr((char *)buf, "water off") != NULL){	//关水4
					atk_mw8266d_uart_rx_restart();
					return 4;
			}
			else if (strstr((char *)buf, "fun on") != NULL){		//自动
					atk_mw8266d_uart_rx_restart();
					return 5;
			}
			else if (strstr((char *)buf, "fun off") != NULL){		//自动
					atk_mw8266d_uart_rx_restart();
					return 6;
			}
			else if (strstr((char *)buf, "auto") != NULL){			//手动
					atk_mw8266d_uart_rx_restart();
					return 7;
			}
			else if (strstr((char *)buf, "manual") != NULL){		//自动
					atk_mw8266d_uart_rx_restart();
					return 8;
			}
			
			// 重新开始接收下一帧
			atk_mw8266d_uart_rx_restart();
    }
    return 0; // 如果没有匹配的字符串，返回0表示无操作
}

//连接Wifi
uint8_t wifi_connect(){
	uint8_t ret=0;
	char ip_buf[16];
	printf("WiFi Connecting\r\n");
	ret  = atk_mw8266d_restore();                               /* 恢复出厂设置 */
	ret += atk_mw8266d_at_test();                               /* AT测试 */
	ret += atk_mw8266d_set_mode(1);                             /* Station模式 */
	ret += atk_mw8266d_sw_reset();                              /* 软件复位 */
	ret += atk_mw8266d_ate_config(0);                           /* 关闭回显功能 */
	ret += atk_mw8266d_join_ap(DEMO_WIFI_SSID, DEMO_WIFI_PWD);  /* 连接WIFI */
	ret += atk_mw8266d_get_ip(ip_buf);                          /* 获取IP地址 */
	if (ret != 0)
	{
			printf("Error to connect WiFi!\r\n");
			return ret;
	}
	printf("Connected!\r\n");
	atk_mw8266d_uart_rx_restart();
	return ret;
}
