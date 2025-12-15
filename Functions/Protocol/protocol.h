/**
 * @file    protocol.h
 * @brief   协议主模块
 * @version 2.0
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>
#include "msg_types.h"
#include "json_builder.h"

/* 消息缓冲区大小 */
#define MSG_BUFFER_SIZE     512

/* 消息结构 */
typedef struct {
    char        version[8];         /* 协议版本 */
    char        msg_id[24];         /* 消息ID */
    uint32_t    timestamp;          /* 时间戳 */
    char        type[16];           /* 消息类型 */
    char        device_id[32];      /* 设备ID */
    char        payload[256];       /* payload JSON 子串 */
} protocol_msg_t;

/**
 * @brief  解析接收到的消息
 * @param  json: 原始 JSON 字符串
 * @param  msg: 输出消息结构
 * @retval 0: 成功, 其他: 失败
 */
uint8_t protocol_parse_message(const char *json, protocol_msg_t *msg);

/**
 * @brief  构建注册消息
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  device_id: 设备ID
 * @param  user_id: 用户ID
 * @param  version: 固件版本
 * @retval 消息长度，0表示失败
 */
uint16_t protocol_build_register(char *buffer, uint16_t buf_size,
                                  const char *device_id, const char *user_id, const char *version);

/**
 * @brief  构建传感器数据消息
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  device_id: 设备ID
 * @param  sensor_count: 传感器数量
 * @param  keys: 传感器键名数组
 * @param  values: 传感器值数组
 * @retval 消息长度
 */
uint16_t protocol_build_sensor_data(char *buffer, uint16_t buf_size,
                                     const char *device_id,
                                     uint8_t sensor_count,
                                     const char **keys, const int32_t *values);

/**
 * @brief  构建状态消息
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  device_id: 设备ID
 * @param  mode: 模式
 * @param  control_count: 控制项数量
 * @param  keys: 控制项键名数组
 * @param  states: 状态数组
 * @retval 消息长度
 */
uint16_t protocol_build_status(char *buffer, uint16_t buf_size,
                                const char *device_id, uint8_t mode,
                                uint8_t control_count,
                                const char **keys, const uint8_t *states);

/**
 * @brief  构建 ACK 消息
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  device_id: 设备ID
 * @param  ref_msg_id: 引用的消息ID
 * @param  success: 是否成功
 * @param  key: 控制项键名 (可选)
 * @param  state: 状态 (可选)
 * @retval 消息长度
 */
uint16_t protocol_build_ack(char *buffer, uint16_t buf_size,
                             const char *device_id, const char *ref_msg_id,
                             uint8_t success, const char *key, uint8_t state);

/**
 * @brief  构建心跳消息
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @param  device_id: 设备ID
 * @retval 消息长度
 */
uint16_t protocol_build_heartbeat(char *buffer, uint16_t buf_size, const char *device_id);

/**
 * @brief  生成简单的消息ID
 * @param  out: 输出缓冲区 (至少16字节)
 */
void protocol_generate_msg_id(char *out);

/**
 * @brief  获取当前时间戳 (秒)
 * @retval 时间戳
 */
uint32_t protocol_get_timestamp(void);

#endif /* __PROTOCOL_H */
