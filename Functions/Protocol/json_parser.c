/**
 * @file    json_parser.c
 * @brief   轻量级 JSON 解析器实现
 * @version 2.0
 */

#include "json_parser.h"
#include <string.h>
#include <stdlib.h>

/* 跳过空白字符 */
static const char* skip_whitespace(const char *str)
{
    while (*str && (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')) {
        str++;
    }
    return str;
}

/* 查找键名位置 */
static const char* find_key(const char *json, const char *key)
{
    char search[JSON_MAX_STRING + 4];
    int key_len = strlen(key);

    if (key_len > JSON_MAX_STRING) {
        return NULL;
    }

    /* 构造搜索模式 "key" */
    search[0] = '"';
    memcpy(search + 1, key, key_len);
    search[key_len + 1] = '"';
    search[key_len + 2] = '\0';

    const char *pos = strstr(json, search);
    if (!pos) {
        return NULL;
    }

    /* 跳过键名和冒号 */
    pos += key_len + 2;
    pos = skip_whitespace(pos);

    if (*pos != ':') {
        return NULL;
    }
    pos++;
    pos = skip_whitespace(pos);

    return pos;
}

/* 提取字符串值 */
static char* extract_string(const char *pos, char *out, uint8_t max_len)
{
    if (*pos != '"') {
        return NULL;
    }
    pos++;  /* 跳过开始引号 */

    uint8_t i = 0;
    while (*pos && *pos != '"' && i < max_len - 1) {
        if (*pos == '\\' && *(pos + 1)) {
            pos++;  /* 跳过转义字符 */
            switch (*pos) {
                case 'n': out[i++] = '\n'; break;
                case 'r': out[i++] = '\r'; break;
                case 't': out[i++] = '\t'; break;
                case '"': out[i++] = '"'; break;
                case '\\': out[i++] = '\\'; break;
                default: out[i++] = *pos; break;
            }
        } else {
            out[i++] = *pos;
        }
        pos++;
    }
    out[i] = '\0';

    return out;
}

/* 提取数值 */
static int32_t extract_int(const char *pos)
{
    return (int32_t)atoi(pos);
}

/* 提取浮点数 */
static float extract_float(const char *pos)
{
    return (float)atof(pos);
}

/* 提取对象或数组 */
static char* extract_block(const char *pos, char open, char close, char *out, uint16_t max_len)
{
    if (*pos != open) {
        return NULL;
    }

    int depth = 1;
    uint16_t i = 0;
    out[i++] = *pos++;

    while (*pos && depth > 0 && i < max_len - 1) {
        if (*pos == '"') {
            /* 跳过字符串 */
            out[i++] = *pos++;
            while (*pos && i < max_len - 1) {
                out[i++] = *pos;
                if (*pos == '"' && *(pos - 1) != '\\') {
                    pos++;
                    break;
                }
                pos++;
            }
        } else {
            if (*pos == open) depth++;
            else if (*pos == close) depth--;
            out[i++] = *pos++;
        }
    }
    out[i] = '\0';

    return (depth == 0) ? out : NULL;
}

/* ===== 公共接口实现 ===== */

char* json_get_string(const char *json, const char *key, char *out, uint8_t max_len)
{
    const char *pos = find_key(json, key);
    if (!pos) {
        return NULL;
    }
    return extract_string(pos, out, max_len);
}

int32_t json_get_int(const char *json, const char *key, int32_t default_val)
{
    const char *pos = find_key(json, key);
    if (!pos) {
        return default_val;
    }

    /* 检查是否是数字 */
    if ((*pos >= '0' && *pos <= '9') || *pos == '-') {
        return extract_int(pos);
    }
    return default_val;
}

float json_get_float(const char *json, const char *key, float default_val)
{
    const char *pos = find_key(json, key);
    if (!pos) {
        return default_val;
    }

    if ((*pos >= '0' && *pos <= '9') || *pos == '-' || *pos == '.') {
        return extract_float(pos);
    }
    return default_val;
}

char* json_get_object(const char *json, const char *key, char *out, uint16_t max_len)
{
    const char *pos = find_key(json, key);
    if (!pos) {
        return NULL;
    }
    return extract_block(pos, '{', '}', out, max_len);
}

int json_get_array_length(const char *json, const char *key)
{
    const char *pos = find_key(json, key);
    if (!pos || *pos != '[') {
        return -1;
    }

    pos++;  /* 跳过 [ */
    pos = skip_whitespace(pos);

    if (*pos == ']') {
        return 0;  /* 空数组 */
    }

    int count = 1;
    int depth = 0;
    bool in_string = false;

    while (*pos && !(*pos == ']' && depth == 0 && !in_string)) {
        if (*pos == '"' && *(pos - 1) != '\\') {
            in_string = !in_string;
        } else if (!in_string) {
            if (*pos == '[' || *pos == '{') depth++;
            else if (*pos == ']' || *pos == '}') depth--;
            else if (*pos == ',' && depth == 0) count++;
        }
        pos++;
    }

    return count;
}

char* json_get_array_string(const char *json, const char *key, int index, char *out, uint8_t max_len)
{
    const char *pos = find_key(json, key);
    if (!pos || *pos != '[') {
        return NULL;
    }

    pos++;  /* 跳过 [ */
    pos = skip_whitespace(pos);

    int current = 0;
    while (*pos && current < index) {
        /* 跳过当前元素 */
        if (*pos == '"') {
            pos++;
            while (*pos && !(*pos == '"' && *(pos - 1) != '\\')) pos++;
            if (*pos) pos++;
        } else if (*pos == '{') {
            int depth = 1;
            pos++;
            while (*pos && depth > 0) {
                if (*pos == '{') depth++;
                else if (*pos == '}') depth--;
                pos++;
            }
        }

        pos = skip_whitespace(pos);
        if (*pos == ',') {
            pos++;
            pos = skip_whitespace(pos);
            current++;
        } else {
            break;
        }
    }

    if (current != index) {
        return NULL;
    }

    return extract_string(pos, out, max_len);
}

char* json_get_array_object(const char *json, const char *key, int index, char *out, uint16_t max_len)
{
    const char *pos = find_key(json, key);
    if (!pos || *pos != '[') {
        return NULL;
    }

    pos++;  /* 跳过 [ */
    pos = skip_whitespace(pos);

    int current = 0;
    while (*pos && current < index) {
        /* 跳过当前元素 */
        if (*pos == '{') {
            int depth = 1;
            pos++;
            while (*pos && depth > 0) {
                if (*pos == '{') depth++;
                else if (*pos == '}') depth--;
                pos++;
            }
        } else if (*pos == '"') {
            pos++;
            while (*pos && !(*pos == '"' && *(pos - 1) != '\\')) pos++;
            if (*pos) pos++;
        }

        pos = skip_whitespace(pos);
        if (*pos == ',') {
            pos++;
            pos = skip_whitespace(pos);
            current++;
        } else {
            break;
        }
    }

    if (current != index) {
        return NULL;
    }

    return extract_block(pos, '{', '}', out, max_len);
}
