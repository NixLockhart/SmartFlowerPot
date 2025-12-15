/**
 * @file    protocol.c
 * @brief   协议主模块实现
 * @version 2.0
 */

#include "protocol.h"
#include "json_parser.h"
#include "json_builder.h"
#include <string.h>
#include <stdio.h>

/* 消息ID计数器 */
static uint32_t s_msg_id_counter = 0;

/* 时间戳 (需要外部更新) */
static uint32_t s_timestamp = 0;

/**
 * @brief  设置时间戳 (供外部RTC或NTP更新)
 */
void protocol_set_timestamp(uint32_t ts)
{
    s_timestamp = ts;
}

/**
 * @brief  获取当前时间戳
 */
uint32_t protocol_get_timestamp(void)
{
    return s_timestamp;
}

/**
 * @brief  生成简单的消息ID
 */
void protocol_generate_msg_id(char *out)
{
    s_msg_id_counter++;
    sprintf(out, "m%08lx", (unsigned long)s_msg_id_counter);
}

/**
 * @brief  解析接收到的消息
 */
uint8_t protocol_parse_message(const char *json, protocol_msg_t *msg)
{
    if (!json || !msg) {
        return ERR_INVALID_PARAM;
    }

    /* 清空结构 */
    memset(msg, 0, sizeof(protocol_msg_t));

    /* 解析版本 */
    if (!json_get_string(json, "v", msg->version, sizeof(msg->version))) {
        strcpy(msg->version, PROTOCOL_VERSION);
    }

    /* 解析消息ID */
    json_get_string(json, "id", msg->msg_id, sizeof(msg->msg_id));

    /* 解析时间戳 */
    msg->timestamp = (uint32_t)json_get_int(json, "ts", 0);

    /* 解析消息类型 */
    if (!json_get_string(json, "t", msg->type, sizeof(msg->type))) {
        return ERR_PARSE_FAILED;
    }

    /* 解析设备ID */
    json_get_string(json, "d", msg->device_id, sizeof(msg->device_id));

    /* 解析payload */
    json_get_object(json, "p", msg->payload, sizeof(msg->payload));

    return ERR_NONE;
}

/**
 * @brief  构建注册消息
 */
uint16_t protocol_build_register(char *buffer, uint16_t buf_size,
                                  const char *device_id, const char *user_id, const char *version)
{
    json_builder_t builder;
    char msg_id[24];

    json_builder_init(&builder, buffer, buf_size);
    protocol_generate_msg_id(msg_id);

    json_begin_object(&builder);

    json_add_string(&builder, "v", PROTOCOL_VERSION);
    json_add_string(&builder, "id", msg_id);
    json_add_int(&builder, "ts", protocol_get_timestamp());
    json_add_string(&builder, "t", MSG_TYPE_REG);

    /* payload */
    json_add_object(&builder, "p");
    json_add_string(&builder, "d", device_id);
    json_add_string(&builder, "u", user_id);
    json_add_string(&builder, "ver", version);
    json_end_object(&builder);

    json_end_object(&builder);

    if (json_builder_finish(&builder)) {
        return json_builder_length(&builder);
    }
    return 0;
}

/**
 * @brief  构建传感器数据消息
 */
uint16_t protocol_build_sensor_data(char *buffer, uint16_t buf_size,
                                     const char *device_id,
                                     uint8_t sensor_count,
                                     const char **keys, const int32_t *values)
{
    json_builder_t builder;
    char msg_id[24];

    json_builder_init(&builder, buffer, buf_size);
    protocol_generate_msg_id(msg_id);

    json_begin_object(&builder);

    json_add_string(&builder, "v", PROTOCOL_VERSION);
    json_add_string(&builder, "id", msg_id);
    json_add_int(&builder, "ts", protocol_get_timestamp());
    json_add_string(&builder, "t", MSG_TYPE_DAT);
    json_add_string(&builder, "d", device_id);

    /* payload - 传感器数据 */
    json_add_object(&builder, "p");
    for (uint8_t i = 0; i < sensor_count; i++) {
        json_add_int(&builder, keys[i], values[i]);
    }
    json_end_object(&builder);

    json_end_object(&builder);

    if (json_builder_finish(&builder)) {
        return json_builder_length(&builder);
    }
    return 0;
}

/**
 * @brief  构建状态消息
 */
uint16_t protocol_build_status(char *buffer, uint16_t buf_size,
                                const char *device_id, uint8_t mode,
                                uint8_t control_count,
                                const char **keys, const uint8_t *states)
{
    json_builder_t builder;
    char msg_id[24];

    json_builder_init(&builder, buffer, buf_size);
    protocol_generate_msg_id(msg_id);

    json_begin_object(&builder);

    json_add_string(&builder, "v", PROTOCOL_VERSION);
    json_add_string(&builder, "id", msg_id);
    json_add_int(&builder, "ts", protocol_get_timestamp());
    json_add_string(&builder, "t", MSG_TYPE_STA);
    json_add_string(&builder, "d", device_id);

    /* payload - 状态数据 */
    json_add_object(&builder, "p");
    json_add_int(&builder, "mode", mode);
    for (uint8_t i = 0; i < control_count; i++) {
        json_add_int(&builder, keys[i], states[i]);
    }
    json_end_object(&builder);

    json_end_object(&builder);

    if (json_builder_finish(&builder)) {
        return json_builder_length(&builder);
    }
    return 0;
}

/**
 * @brief  构建 ACK 消息
 */
uint16_t protocol_build_ack(char *buffer, uint16_t buf_size,
                             const char *device_id, const char *ref_msg_id,
                             uint8_t success, const char *key, uint8_t state)
{
    json_builder_t builder;

    json_builder_init(&builder, buffer, buf_size);

    json_begin_object(&builder);

    json_add_string(&builder, "v", PROTOCOL_VERSION);
    json_add_string(&builder, "id", ref_msg_id);  /* 使用原消息ID */
    json_add_int(&builder, "ts", protocol_get_timestamp());
    json_add_string(&builder, "t", MSG_TYPE_ACK);
    json_add_string(&builder, "d", device_id);

    /* payload */
    json_add_object(&builder, "p");
    json_add_int(&builder, "ok", success);
    if (key) {
        json_add_string(&builder, "k", key);
        json_add_int(&builder, "s", state);
    }
    json_end_object(&builder);

    json_end_object(&builder);

    if (json_builder_finish(&builder)) {
        return json_builder_length(&builder);
    }
    return 0;
}

/**
 * @brief  构建心跳消息
 */
uint16_t protocol_build_heartbeat(char *buffer, uint16_t buf_size, const char *device_id)
{
    json_builder_t builder;

    json_builder_init(&builder, buffer, buf_size);

    json_begin_object(&builder);

    json_add_string(&builder, "v", PROTOCOL_VERSION);
    json_add_string(&builder, "id", "hb");
    json_add_int(&builder, "ts", protocol_get_timestamp());
    json_add_string(&builder, "t", MSG_TYPE_HB);
    json_add_string(&builder, "d", device_id);

    /* 空 payload */
    json_add_object(&builder, "p");
    json_end_object(&builder);

    json_end_object(&builder);

    if (json_builder_finish(&builder)) {
        return json_builder_length(&builder);
    }
    return 0;
}
