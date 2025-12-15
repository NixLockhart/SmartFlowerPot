/**
 * @file    json_builder.c
 * @brief   JSON 构建器实现
 * @version 2.0
 */

#include "json_builder.h"
#include <string.h>
#include <stdio.h>

/* 向缓冲区追加字符串 */
static void builder_append(json_builder_t *builder, const char *str)
{
    if (builder->error) return;

    uint16_t len = strlen(str);
    if (builder->pos + len >= builder->buf_size) {
        builder->error = true;
        return;
    }

    memcpy(builder->buffer + builder->pos, str, len);
    builder->pos += len;
    builder->buffer[builder->pos] = '\0';
}

/* 添加逗号 (如果需要) */
static void builder_add_comma(json_builder_t *builder)
{
    if (builder->need_comma) {
        builder_append(builder, ",");
    }
    builder->need_comma = false;
}

/* 添加带引号的键名 */
static void builder_add_key(json_builder_t *builder, const char *key)
{
    builder_add_comma(builder);
    builder_append(builder, "\"");
    builder_append(builder, key);
    builder_append(builder, "\":");
}

void json_builder_init(json_builder_t *builder, char *buffer, uint16_t buf_size)
{
    builder->buffer = buffer;
    builder->buf_size = buf_size;
    builder->pos = 0;
    builder->depth = 0;
    builder->need_comma = false;
    builder->error = false;
    builder->buffer[0] = '\0';
}

void json_begin_object(json_builder_t *builder)
{
    builder_add_comma(builder);
    builder_append(builder, "{");
    builder->depth++;
    builder->need_comma = false;
}

void json_end_object(json_builder_t *builder)
{
    builder_append(builder, "}");
    if (builder->depth > 0) builder->depth--;
    builder->need_comma = true;
}

void json_begin_array(json_builder_t *builder, const char *key)
{
    if (key) {
        builder_add_key(builder, key);
    } else {
        builder_add_comma(builder);
    }
    builder_append(builder, "[");
    builder->depth++;
    builder->need_comma = false;
}

void json_end_array(json_builder_t *builder)
{
    builder_append(builder, "]");
    if (builder->depth > 0) builder->depth--;
    builder->need_comma = true;
}

void json_add_string(json_builder_t *builder, const char *key, const char *value)
{
    builder_add_key(builder, key);
    builder_append(builder, "\"");
    builder_append(builder, value ? value : "");
    builder_append(builder, "\"");
    builder->need_comma = true;
}

void json_add_int(json_builder_t *builder, const char *key, int32_t value)
{
    char num_str[16];
    sprintf(num_str, "%ld", (long)value);

    builder_add_key(builder, key);
    builder_append(builder, num_str);
    builder->need_comma = true;
}

void json_add_float(json_builder_t *builder, const char *key, float value, uint8_t decimals)
{
    char num_str[24];
    char fmt[8];

    sprintf(fmt, "%%.%df", decimals);
    sprintf(num_str, fmt, value);

    builder_add_key(builder, key);
    builder_append(builder, num_str);
    builder->need_comma = true;
}

void json_add_bool(json_builder_t *builder, const char *key, bool value)
{
    builder_add_key(builder, key);
    builder_append(builder, value ? "true" : "false");
    builder->need_comma = true;
}

void json_add_object(json_builder_t *builder, const char *key)
{
    builder_add_key(builder, key);
    builder_append(builder, "{");
    builder->depth++;
    builder->need_comma = false;
}

void json_add_raw_int(json_builder_t *builder, int32_t value)
{
    char num_str[16];
    sprintf(num_str, "%ld", (long)value);

    builder_add_comma(builder);
    builder_append(builder, num_str);
    builder->need_comma = true;
}

void json_add_raw_string(json_builder_t *builder, const char *value)
{
    builder_add_comma(builder);
    builder_append(builder, "\"");
    builder_append(builder, value ? value : "");
    builder_append(builder, "\"");
    builder->need_comma = true;
}

const char* json_builder_finish(json_builder_t *builder)
{
    if (builder->error || builder->depth != 0) {
        return NULL;
    }
    return builder->buffer;
}

bool json_builder_has_error(json_builder_t *builder)
{
    return builder->error;
}

uint16_t json_builder_length(json_builder_t *builder)
{
    return builder->pos;
}
