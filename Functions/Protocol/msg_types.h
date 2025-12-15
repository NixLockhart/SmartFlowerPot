/**
 * @file    msg_types.h
 * @brief   Server-Device 消息类型定义
 * @version 2.0
 */

#ifndef __MSG_TYPES_H
#define __MSG_TYPES_H

/* ===== 协议版本 ===== */
#define PROTOCOL_VERSION    "1.0"

/* ===== 上行消息类型 (Device -> Server) ===== */
#define MSG_TYPE_REG        "reg"       /* 设备注册 */
#define MSG_TYPE_DAT        "dat"       /* 传感器数据 */
#define MSG_TYPE_STA        "sta"       /* 设备状态 */
#define MSG_TYPE_ACK        "ack"       /* 命令确认 */
#define MSG_TYPE_HB         "hb"        /* 心跳 */

/* ===== 下行消息类型 (Server -> Device) ===== */
#define MSG_TYPE_REG_OK     "reg_ok"    /* 注册成功 */
#define MSG_TYPE_REG_ERR    "reg_err"   /* 注册失败 */
#define MSG_TYPE_CTL        "ctl"       /* 开关控制 */
#define MSG_TYPE_ACT        "act"       /* 功能操作 */
#define MSG_TYPE_CFG        "cfg"       /* 配置同步 */
#define MSG_TYPE_HB_OK      "hb_ok"     /* 心跳响应 */
#define MSG_TYPE_ERR        "err"       /* 错误 */

/* ===== 条件类型 ===== */
#define COND_GT             "gt"        /* 大于 */
#define COND_LT             "lt"        /* 小于 */
#define COND_GTE            "gte"       /* 大于等于 */
#define COND_LTE            "lte"       /* 小于等于 */
#define COND_EQ             "eq"        /* 等于 */

/* ===== 动作类型 ===== */
#define ACTION_ON           1           /* 开启 */
#define ACTION_OFF          0           /* 关闭 */

/* ===== 错误码 ===== */
#define ERR_NONE            0
#define ERR_PARSE_FAILED    1
#define ERR_UNKNOWN_TYPE    2
#define ERR_BUFFER_OVERFLOW 3
#define ERR_INVALID_PARAM   4

#endif /* __MSG_TYPES_H */
