/**
 * @file    app_main.h
 * @brief   应用主模块 - 整合所有服务器通信和自动控制功能
 * @version 2.0
 */

#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  初始化应用 (在main函数开始时调用)
 * @note   初始化所有模块: WiFi、服务器客户端、配置、传感器、控制
 */
void app_init(void);

/**
 * @brief  应用主循环处理 (在main循环中调用)
 * @note   处理服务器通信、数据上报、阈值检查
 */
void app_process(void);

/**
 * @brief  注册传感器读取函数
 * @note   调用后传感器数据会自动读取和上报
 */
void app_register_sensors(void);

/**
 * @brief  连接到服务器
 * @retval 0: 成功发起连接, 其他: 失败
 */
int app_connect_server(void);

/**
 * @brief  断开服务器连接
 */
void app_disconnect_server(void);

/**
 * @brief  检查是否已连接并注册到服务器
 * @retval true: 已就绪, false: 未就绪
 */
bool app_is_server_ready(void);

/**
 * @brief  手动触发数据上报
 */
void app_report_data(void);

/**
 * @brief  设置工作模式
 * @param  mode: 0=自动, 1=手动
 */
void app_set_mode(uint8_t mode);

/**
 * @brief  获取当前工作模式
 * @retval 0=自动, 1=手动
 */
uint8_t app_get_mode(void);

/**
 * @brief  手动控制设备
 * @param  key: 控制项键名 (pump/fan/light/heater)
 * @param  state: 0=关, 1=开
 * @retval 0: 成功, 其他: 失败
 */
int app_manual_control(const char *key, uint8_t state);

#endif /* __APP_MAIN_H */
