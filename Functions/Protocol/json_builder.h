/**
 * @file    json_builder.h
 * @brief   JSON 构建器
 * @note    链式调用，固定缓冲区
 * @version 2.0
 */

#ifndef __JSON_BUILDER_H
#define __JSON_BUILDER_H

#include <stdint.h>
#include <stdbool.h>

/* 构建器结构 */
typedef struct {
    char    *buffer;        /* 输出缓冲区 */
    uint16_t buf_size;      /* 缓冲区大小 */
    uint16_t pos;           /* 当前位置 */
    uint8_t  depth;         /* 嵌套深度 */
    bool     need_comma;    /* 是否需要逗号 */
    bool     error;         /* 错误标志 */
} json_builder_t;

/**
 * @brief  初始化 JSON 构建器
 * @param  builder: 构建器指针
 * @param  buffer: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 */
void json_builder_init(json_builder_t *builder, char *buffer, uint16_t buf_size);

/**
 * @brief  开始对象 {
 */
void json_begin_object(json_builder_t *builder);

/**
 * @brief  结束对象 }
 */
void json_end_object(json_builder_t *builder);

/**
 * @brief  开始数组 [
 * @param  key: 数组键名 (NULL 表示匿名数组)
 */
void json_begin_array(json_builder_t *builder, const char *key);

/**
 * @brief  结束数组 ]
 */
void json_end_array(json_builder_t *builder);

/**
 * @brief  添加字符串字段 "key":"value"
 */
void json_add_string(json_builder_t *builder, const char *key, const char *value);

/**
 * @brief  添加整数字段 "key":123
 */
void json_add_int(json_builder_t *builder, const char *key, int32_t value);

/**
 * @brief  添加浮点字段 "key":1.23
 */
void json_add_float(json_builder_t *builder, const char *key, float value, uint8_t decimals);

/**
 * @brief  添加布尔字段 "key":true/false
 */
void json_add_bool(json_builder_t *builder, const char *key, bool value);

/**
 * @brief  添加对象字段 (开始) "key":{
 */
void json_add_object(json_builder_t *builder, const char *key);

/**
 * @brief  添加裸整数 (用于数组内)
 */
void json_add_raw_int(json_builder_t *builder, int32_t value);

/**
 * @brief  添加裸字符串 (用于数组内)
 */
void json_add_raw_string(json_builder_t *builder, const char *value);

/**
 * @brief  获取构建结果
 * @retval 成功返回 JSON 字符串指针，失败返回 NULL
 */
const char* json_builder_finish(json_builder_t *builder);

/**
 * @brief  检查是否有错误
 */
bool json_builder_has_error(json_builder_t *builder);

/**
 * @brief  获取当前长度
 */
uint16_t json_builder_length(json_builder_t *builder);

#endif /* __JSON_BUILDER_H */
