# SmartFlowerPot

基于正点原子 STM32F103ZET6 开发板的智能花盆系统。带 2.8 寸触摸屏和 WiFi 远程控制。

![Platform](https://img.shields.io/badge/Platform-STM32F103ZET6-blue)
![IDE](https://img.shields.io/badge/IDE-Keil%20MDK5-green)
![GUI](https://img.shields.io/badge/GUI-LVGL%208.x-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)

## 简介

这是一个智能花盆项目，系统可以实时监测温湿度、土壤湿度、光照强度，并根据设定阈值自动控制浇水、补光和通风。支持手动/自动两种模式切换，可通过 WiFi 连接自定义 TCP 服务器实现手机远程监控。

## 功能特性

### 传感器采集
| 传感器 | 型号 | 引脚 | 说明 |
|--------|------|------|------|
| 温湿度 | DHT11 | PG11 | 单总线协议 |
| 土壤湿度 | 电容式 | PA5 (ADC1_CH5) | ADC 采集，10次取平均 |
| 光照强度 | 光敏电阻 | PF8 (ADC3_CH6) | ADC 采集，10次取平均 |

### 执行器控制
| 执行器 | 控制方式 | 引脚 | 说明 |
|--------|----------|------|------|
| 水泵 | 继电器 | PA7 | 高电平开启 |
| 风扇 | 继电器 | PA6 | 高电平开启 |
| 补光灯 | GPIO 直驱 | LED1 | 低电平点亮 |

### 人机交互
- **显示屏**: 2.8 寸 TFT LCD (ILI9341, 320x240)
- **GUI 框架**: LVGL 8.x
- **输入设备**:
  - 3 个物理按键 (KEY0, KEY1, KEY_UP)
  - 电容触摸按键 (TPAD)
  - 电阻触摸屏

### 界面说明

系统包含 4 个主要界面：

| 界面 | 功能 | 操作方式 |
|------|------|----------|
| **主界面** | 显示温度、湿度、土壤湿度、光照强度实时数据 | KEY_UP 进入菜单 |
| **菜单界面** | 选择功能：阈值设置、模式切换、手动控制 | KEY0/KEY1 上下移动，KEY_UP 确认，TPAD 返回 |
| **阈值设置** | 调整温度/土壤湿度/光照的上下限值 | KEY0 调下限，KEY1 调上限，TPAD 返回 |
| **手动控制** | 手动开关水泵、补光灯、风扇 | KEY0/KEY1 选择项目，KEY_UP 切换开关，TPAD 返回 |

### 工作模式

- **自动模式** (默认)
  - 温度超上限 → 开启风扇降温
  - 土壤湿度低于下限 → 开启水泵浇水
  - 光照低于下限 → 开启补光灯
  - 数据恢复正常范围后自动关闭对应设备

- **手动模式**
  - 禁用自动控制逻辑
  - 用户通过触摸屏或远程命令手动控制各执行器

### 远程通信

- **WiFi 模块**: ATK-MW8266D (ESP8266)
- **通信方式**: TCP 长连接
- **数据格式**: JSON
- **协议版本**: V2.0

#### 上行消息 (设备 → 服务器)
| 类型 | 字段 | 说明 |
|------|------|------|
| `reg` | 设备注册 | 包含 device_id, user_id |
| `hb` | 心跳 | 保持连接 |
| `dat` | 传感器数据 | temp, humi, soil, light |
| `sta` | 设备状态 | mode, light, water, fan |
| `ack` | 命令确认 | cmd_id, success |

#### 下行消息 (服务器 → 设备)
| 类型 | 说明 |
|------|------|
| `ctl` | 开关控制 (light_on/off, water_on/off, fan_on/off) |
| `act` | 功能操作 (mode_auto, mode_manual) |
| `cfg` | 配置同步 (阈值设置) |

## 硬件清单

| 模块 | 型号 | 接口 | 备注 |
|------|------|------|------|
| 主控 | STM32F103ZET6 | - | 正点原子战舰/精英板 |
| 显示屏 | 2.8寸 TFT LCD | FSMC | ILI9341 + 电阻触摸 |
| WiFi | ATK-MW8266D | USART3 | ESP8266 模块 |
| 温湿度 | DHT11 | PG11 | 单总线 |
| 土壤湿度 | 电容式传感器 | PA5 | ADC1_CH5 |
| 光照 | 光敏电阻 | PF8 | ADC3_CH6 |
| 水泵 | 5V 微型水泵 | PA7 | 继电器控制 |
| 风扇 | 5V 小风扇 | PA6 | 继电器控制 |

## 工程结构

```
SmartFlowerPot/
├── USER/                   # 主程序入口、Keil 工程文件
│   └── main.c              # 主函数，系统初始化和主循环
├── HARDWARE/               # 硬件驱动
│   ├── DHT11/              # 温湿度传感器驱动
│   ├── TS/                 # 土壤湿度传感器驱动
│   ├── LSENS/              # 光敏电阻驱动
│   ├── BUMP/               # 水泵和风扇继电器驱动
│   ├── LCD/                # LCD 显示驱动
│   ├── TOUCH/              # 触摸屏驱动
│   ├── ATK_MW8266D/        # WiFi 模块驱动
│   └── ...                 # 其他外设驱动
├── Functions/
│   ├── MyServer/           # 服务器通信模块
│   │   ├── myserver.c/h    # TCP 连接、数据收发
│   │   └── app_main.c/h    # 应用层封装
│   ├── Protocol/           # 通信协议
│   │   ├── json_builder.c/h    # JSON 构建
│   │   ├── json_parser.c/h     # JSON 解析
│   │   ├── protocol.c/h        # 协议处理
│   │   └── msg_types.h         # 消息类型定义
│   ├── UI/                 # 用户界面
│   │   └── ui.c/h          # LVGL 界面实现
│   ├── Config/             # 配置管理
│   │   ├── device_config.c/h   # 设备配置
│   │   ├── sensor_manager.c/h  # 传感器管理
│   │   ├── control_manager.c/h # 控制器管理
│   │   └── threshold_engine.c/h# 阈值引擎
│   └── WiFi/               # WiFi 连接管理
├── Middlewares/LVGL/       # LVGL 图形库
├── SYSTEM/                 # 系统级代码 (delay, usart, sys)
├── CORE/                   # Cortex-M3 内核文件
└── STM32F10x_FWLib/        # ST 标准外设库
```

## 快速开始

### 编译

1. 安装 Keil MDK5
2. 打开 `USER/TOUCH.uvprojx`
3. 编译 (F7) 并下载到开发板

### 配置 WiFi

修改 `Functions/MyServer/myserver.h`:

```c
#define MY_WIFI_SSID           "你的WiFi名"
#define MY_WIFI_PWD            "WiFi密码"
#define MY_SERVER_IP           "服务器IP"
#define MY_SERVER_PORT         "8003"
#define MY_DEVICE_ID           "SFP_001"
#define MY_USER_ID             "你的用户名"
```

### 引脚接线

| 功能 | 引脚 | 说明 |
|------|------|------|
| DHT11 数据 | PG11 | 需要 4.7K 上拉电阻 |
| 土壤湿度 | PA5 | ADC 输入，0-3.3V |
| 光敏电阻 | PF8 | ADC 输入，0-3.3V |
| 水泵继电器 | PA7 | 推挽输出 |
| 风扇继电器 | PA6 | 推挽输出 |
| WiFi TX | USART3_RX | 波特率 115200 |
| WiFi RX | USART3_TX | 波特率 115200 |

## 通信协议示例

```json
// 设备注册
{"t":"reg","did":"SFP_001","uid":"lockhart"}

// 上报传感器数据
{"t":"dat","did":"SFP_001","d":{"temp":25,"humi":60,"soil":45,"light":80}}

// 上报设备状态
{"t":"sta","did":"SFP_001","d":{"mode":0,"light":0,"water":0,"fan":0}}

// 服务器下发控制命令
{"t":"ctl","cmd":"water_on","cid":12345}

// 设备确认
{"t":"ack","cid":12345,"ok":1}
```

## 默认阈值

| 参数 | 下限 | 上限 | 单位 |
|------|------|------|------|
| 温度 | 10 | 30 | °C |
| 空气湿度 | 40 | 70 | % |
| 土壤湿度 | 40 | 65 | % |
| 光照强度 | 50 | 90 | % |

## 版本历史

- **V1.3** (2025-12-13)
  - 重构服务器通信模块，支持动态配置
  - 优化 JSON 协议，精简字段名
  - 新增手动控制界面
  - 完善自动控制逻辑

- **V1.2** (2025-12)
  - 替换机智云为自定义 TCP 服务器
  - 添加 Protocol 协议层

- **V1.0** (2025-06)
  - 初始版本
  - 基于机智云平台

## 相关链接

- [演示视频](https://b23.tv/nAN3Vkl)
- [正点原子论坛](http://www.openedv.com/)
- [LVGL 官方文档](https://docs.lvgl.io/)
- [STM32F103 参考手册](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

## 开源协议

MIT License

## 作者

NixStudio (Nix Lockhart)
