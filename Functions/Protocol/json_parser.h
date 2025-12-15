/**
 * @file    json_parser.h
 * @brief   轻量级 JSON 解析器
 * @note    针对嵌入式优化，不使用动态内存分配
 * @version 2.0
 */

#ifndef __JSON_PARSER_H
#define __JSON_PARSER_H

#include <stdint.h>
#include <stdbool.h>

/* 最大嵌套深度 */
#define JSON_MAX_DEPTH      4
/* 最大字符串长度 */
#define JSON_MAX_STRING     64
/* 最大数组元素 */
#define JSON_MAX_ARRAY      16

/**
 * @brief  从 JSON 字符串中获取字符串值
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  out: 输出缓冲区
 * @param  max_len: 缓冲区最大长度
 * @retval 成功返回 out 指针，失败返回 NULL
 */
char* json_get_string(const char *json, const char *key, char *out, uint8_t max_len);

/**
 * @brief  从 JSON 字符串中获取整数值
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  default_val: 默认值 (找不到时返回)
 * @retval 整数值
 */
int32_t json_get_int(const char *json, const char *key, int32_t default_val);

/**
 * @brief  从 JSON 字符串中获取浮点值
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  default_val: 默认值
 * @retval 浮点值
 */
float json_get_float(const char *json, const char *key, float default_val);

/**
 * @brief  获取嵌套对象
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  out: 输出缓冲区 (存储子对象 JSON)
 * @param  max_len: 缓冲区最大长度
 * @retval 成功返回 out 指针，失败返回 NULL
 */
char* json_get_object(const char *json, const char *key, char *out, uint16_t max_len);

/**
 * @brief  获取数组元素个数
 * @param  json: JSON 字符串
 * @param  key: 键名 (数组字段)
 * @retval 数组长度，失败返回 -1
 */
int json_get_array_length(const char *json, const char *key);

/**
 * @brief  获取字符串数组元素
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  index: 数组索引
 * @param  out: 输出缓冲区
 * @param  max_len: 缓冲区最大长度
 * @retval 成功返回 out 指针，失败返回 NULL
 */
char* json_get_array_string(const char *json, const char *key, int index, char *out, uint8_t max_len);

/**
 * @brief  获取对象数组元素
 * @param  json: JSON 字符串
 * @param  key: 键名
 * @param  index: 数组索引
 * @param  out: 输出缓冲区 (存储子对象 JSON)
 * @param  max_len: 缓冲区最大长度
 * @retval 成功返回 out 指针，失败返回 NULL
 */
char* json_get_array_object(const char *json, const char *key, int index, char *out, uint16_t max_len);

#endif /* __JSON_PARSER_H */
