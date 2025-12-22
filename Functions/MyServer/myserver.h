/**
 ****************************************************************************************************
 * @file        myserver.h
 * @author      NixStudio(NixLockhart)
 * @version     V2.0
 * @date        2025-12-14
 * @brief       自定义服务器通信模块 - 支持动态配置
 ****************************************************************************************************
 * @attention
 *
 * 平台: 正点原子 STM32F103开发板
 * 服务器: 自定义TCP服务器 111.228.6.160:8003
 *
 * V2.0 更新:
 * - 添加动态传感器配置支持
 * - 添加动态控制项配置支持
 * - 支持从服务器获取配置
 * - 动态JSON数据构建
 *
 ****************************************************************************************************
 */

#ifndef __MYSERVER_H
#define __MYSERVER_H

#include "sys.h"
#include <stdint.h>

/******************************************************************************************/
/* 配置参数 - 根据实际情况修改 */

#define MY_WIFI_SSID           "Nix_Wifi"          /* WiFi名称 */
#define MY_WIFI_PWD            "12345678"          /* WiFi密码 */
#define MY_SERVER_IP           "111.228.6.160"     /* 服务器IP */
#define MY_SERVER_PORT         "8003"              /* 服务器TCP端口 (设备连接) */
#define MY_DEVICE_ID           "SFP"           /* 设备唯一标识 */
#define MY_USER_ID             "nix"                  /* 绑定的用户ID (在服务器注册的用户名) */

/******************************************************************************************/
/* 连接状态定义 */

#define MY_WIFI_DISCONNECTED    0   /* WiFi未连接 */
#define MY_WIFI_CONNECTED       1   /* WiFi已连接 */
#define MY_WIFI_ERROR           2   /* WiFi连接错误 */

#define MY_SERVER_DISCONNECTED  0   /* 服务器未连接 */
#define MY_SERVER_CONNECTED     1   /* 服务器已连接 */

/******************************************************************************************/
/* V2.0 消息类型定义 (精简字段名) */

/* 上行消息类型 (设备->服务器) */
#define MSG_TYPE_REG            "reg"           /* 设备注册 */
#define MSG_TYPE_HB             "hb"            /* 心跳 */
#define MSG_TYPE_DAT            "dat"           /* 传感器数据 */
#define MSG_TYPE_STA            "sta"           /* 设备状态 */
#define MSG_TYPE_ACK            "ack"           /* 命令确认 */

/* 下行消息类型 (服务器->设备) */
#define MSG_TYPE_CTL            "ctl"           /* 开关控制 */
#define MSG_TYPE_ACT            "act"           /* 功能操作 */
#define MSG_TYPE_CFG            "cfg"           /* 配置同步 */

/******************************************************************************************/
/* 命令类型定义 */

typedef enum {
    CMD_NONE = 0,           /* 无命令 */
    CMD_LIGHT_ON,           /* 开灯 */
    CMD_LIGHT_OFF,          /* 关灯 */
    CMD_WATER_ON,           /* 开水泵 */
    CMD_WATER_OFF,          /* 关水泵 */
    CMD_FAN_ON,             /* 开风扇 */
    CMD_FAN_OFF,            /* 关风扇 */
    CMD_MODE_AUTO,          /* 自动模式 */
    CMD_MODE_MANUAL,        /* 手动模式 */
    CMD_SET_THRESHOLD,      /* 设置阈值 */
    CMD_GET_STATUS,         /* 获取状态 */
    CMD_REBOOT,             /* 重启设备 */
} my_cmd_type_t;

/******************************************************************************************/
/* 数据结构定义 */

/* 传感器数据结构 */
typedef struct {
    uint8_t temperature;        /* 温度 (°C) */
    uint8_t humidity;           /* 空气湿度 (%) */
    uint8_t soil_humidity;      /* 土壤湿度 (%) */
    uint8_t light_intensity;    /* 光照强度 (%) */
} my_sensor_data_t;

/* 设备状态结构 */
typedef struct {
    uint8_t mode;               /* 模式: 0-自动, 1-手动 */
    uint8_t light_status;       /* 灯状态: 0-关, 1-开 */
    uint8_t water_status;       /* 水泵状态: 0-关, 1-开 */
    uint8_t fan_status;         /* 风扇状态: 0-关, 1-开 */
} my_device_status_t;

/* 阈值设置结构 */
typedef struct {
    uint8_t temp_upper;         /* 温度上限 */
    uint8_t temp_lower;         /* 温度下限 */
    uint8_t humi_upper;         /* 空气湿度上限 */
    uint8_t humi_lower;         /* 空气湿度下限 */
    uint8_t soil_upper;         /* 土壤湿度上限 */
    uint8_t soil_lower;         /* 土壤湿度下限 */
    uint8_t light_upper;        /* 光照上限 */
    uint8_t light_lower;        /* 光照下限 */
} my_threshold_t;

/* 接收到的命令结构 */
typedef struct {
    my_cmd_type_t type;         /* 命令类型 */
    uint32_t cmd_id;            /* 命令ID (用于ACK) */
    my_threshold_t threshold;   /* 阈值数据 (当type为CMD_SET_THRESHOLD时有效) */
} my_received_cmd_t;

/******************************************************************************************/
/* 动态配置相关定义 (V2.0新增) */

#define MY_MAX_SENSORS      8       /* 最大传感器数量 */
#define MY_MAX_CONTROLS     8       /* 最大控制项数量 */
#define MY_MAX_KEY_LEN      16      /* 字段名最大长度 */
#define MY_MAX_CMD_LEN      20      /* 命令名最大长度 */

/* 数据类型定义 */
#define MY_DATA_TYPE_INT    0       /* 整数类型 */
#define MY_DATA_TYPE_FLOAT  1       /* 浮点类型 */
#define MY_DATA_TYPE_BOOL   2       /* 布尔类型 */

/* 传感器配置结构 */
typedef struct {
    char key[MY_MAX_KEY_LEN];       /* 字段名 (如 "temp", "humi") */
    uint8_t enabled;                /* 是否启用 */
    uint8_t data_type;              /* 数据类型 */
    void *value_ptr;                /* 数据指针 (指向实际数据变量) */
} my_sensor_config_t;

/* 控制项配置结构 */
typedef struct {
    char key[MY_MAX_KEY_LEN];       /* 控制项标识 (如 "light", "water") */
    char cmd_on[MY_MAX_CMD_LEN];    /* 开启命令 (如 "light_on") */
    char cmd_off[MY_MAX_CMD_LEN];   /* 关闭命令 (如 "light_off") */
    uint8_t *status_ptr;            /* 状态指针 */
    void (*action_on)(void);        /* 开启回调函数 */
    void (*action_off)(void);       /* 关闭回调函数 */
} my_control_config_t;

/* 设备动态配置结构 */
typedef struct {
    uint8_t sensor_count;                           /* 传感器数量 */
    uint8_t control_count;                          /* 控制项数量 */
    my_sensor_config_t sensors[MY_MAX_SENSORS];     /* 传感器配置数组 */
    my_control_config_t controls[MY_MAX_CONTROLS];  /* 控制项配置数组 */
    uint8_t config_loaded;                          /* 配置是否已加载 */
} my_device_config_t;

/******************************************************************************************/
/* 全局变量声明 */

extern uint8_t g_my_wifi_status;        /* WiFi连接状态 */
extern uint8_t g_my_server_status;      /* 服务器连接状态 */
extern my_received_cmd_t g_received_cmd;/* 接收到的命令 */
extern my_device_config_t g_device_config;  /* 设备动态配置 (V2.0新增) */

/******************************************************************************************/
/* 函数声明 */

/* ========== 初始化与连接 ========== */

/**
 * @brief  初始化WiFi模块
 * @retval 0:成功 1:失败
 */
uint8_t myserver_wifi_init(void);

/**
 * @brief  连接WiFi路由器
 * @retval 0:成功 其他:失败
 */
uint8_t myserver_wifi_connect(void);

/**
 * @brief  连接到自定义服务器
 * @retval 0:成功 1:失败
 */
uint8_t myserver_connect(void);

/**
 * @brief  断开服务器连接
 * @retval 0:成功 1:失败
 */
uint8_t myserver_disconnect(void);

/**
 * @brief  重新连接服务器
 * @retval 0:成功 1:失败
 */
uint8_t myserver_reconnect(void);

/* ========== 数据发送 ========== */

/**
 * @brief  发送设备注册信息
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_register(void);

/**
 * @brief  发送心跳包
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_heartbeat(void);

/**
 * @brief  发送传感器数据
 * @param  data: 传感器数据指针
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_sensor_data(my_sensor_data_t *data);

/**
 * @brief  发送设备状态
 * @param  status: 设备状态指针
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_device_status(my_device_status_t *status);

/**
 * @brief  发送命令确认
 * @param  cmd_id: 命令ID
 * @param  success: 是否成功 0-失败 1-成功
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_ack(uint32_t cmd_id, uint8_t success);

/* ========== 数据接收 ========== */

/**
 * @brief  接收并解析服务器消息
 * @retval 命令类型
 */
my_cmd_type_t myserver_receive_command(void);

/**
 * @brief  获取接收到的阈值设置
 * @param  threshold: 阈值数据指针
 * @retval 0:成功 1:失败/无数据
 */
uint8_t myserver_get_threshold(my_threshold_t *threshold);

/* ========== 状态检查 ========== */

/**
 * @brief  检查WiFi连接状态
 * @retval 连接状态
 */
uint8_t myserver_check_wifi(void);

/**
 * @brief  检查服务器连接状态
 * @retval 连接状态
 */
uint8_t myserver_check_server(void);

/* ========== 主处理函数 ========== */

/**
 * @brief  服务器通信处理 (在主循环中周期调用)
 * @note   处理心跳、接收命令等
 */
void myserver_process(void);

/**
 * @brief  处理服务器下发的无线控制命令
 * @note   在主循环中周期调用，接收并执行控制命令
 */
void myserver_wireless_control(void);

/* ========== 动态配置相关函数 (V2.0新增) ========== */

/**
 * @brief  初始化默认配置
 * @note   设置默认的传感器和控制项配置
 */
void myserver_init_default_config(void);

/**
 * @brief  动态发送传感器数据 (根据配置)
 * @retval 0:成功 1:失败
 */
uint8_t myserver_send_sensor_data_dynamic(void);

/**
 * @brief  动态处理控制命令 (根据配置)
 * @param  cmd: 命令字符串
 * @retval 0:成功处理 1:未找到匹配命令
 */
uint8_t myserver_handle_command_dynamic(const char *cmd);

/**
 * @brief  注册传感器配置
 * @param  key: 字段名
 * @param  data_type: 数据类型
 * @param  value_ptr: 数据指针
 * @retval 0:成功 1:失败
 */
uint8_t myserver_register_sensor(const char *key, uint8_t data_type, void *value_ptr);

/**
 * @brief  注册控制项配置
 * @param  key: 控制项标识
 * @param  cmd_on: 开启命令
 * @param  cmd_off: 关闭命令
 * @param  status_ptr: 状态指针
 * @param  action_on: 开启回调
 * @param  action_off: 关闭回调
 * @retval 0:成功 1:失败
 */
uint8_t myserver_register_control(const char *key, const char *cmd_on, const char *cmd_off,
                                   uint8_t *status_ptr, void (*action_on)(void), void (*action_off)(void));

#endif /* __MYSERVER_H */
