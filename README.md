# SmartFlowerPot

基于正点原子 STM32F103ZET6 开发板的智能花盆系统。带 2.8 寸触摸屏和 WiFi 远程控制。

![Platform](https://img.shields.io/badge/Platform-STM32F103-blue)
![IDE](https://img.shields.io/badge/IDE-Keil%20MDK5-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## 简介

这是一个智能花盆项目。系统可以实时监测温湿度、土壤湿度、光照强度，并根据设定阈值自动控制浇水、补光和通风。支持手动/自动两种模式切换，可通过 WiFi 连接服务器实现手机远程监控。

## 功能特性

**传感器采集**
- DHT11 温湿度传感器
- 土壤湿度传感器 (ADC)
- 光敏电阻 (ADC)

**执行器控制**
- 继电器控制水泵
- LED 补光灯
- 小风扇

**人机交互**
- 2.8 寸 TFT 触摸屏 (ILI9341)
- LVGL 图形界面
- 实时数据显示和参数设置

**远程功能**
- ESP8266 WiFi 模块
- TCP 长连接通信
- JSON 数据格式
- 支持远程查看和控制

## 硬件清单

| 模块 | 型号 | 备注 |
|-----|-----|-----|
| 主控 | STM32F103ZET6 | 正点原子战舰/精英板 |
| 显示屏 | 2.8寸 TFT LCD | ILI9341 + 电阻触摸 |
| WiFi | ATK-MW8266D | 正点原子 ESP8266 模块 |
| 温湿度 | DHT11 | - |
| 土壤湿度 | 电容式/电阻式 | ADC 采集 |
| 光照 | 光敏电阻 | ADC 采集 |
| 水泵 | 5V 微型水泵 | 继电器控制 |

## 工程结构

```
├── USER/               # 主程序、Keil 工程文件
├── HARDWARE/           # 外设驱动 (LCD、DHT11、按键等)
├── SYSTEM/             # 系统级代码 (delay、usart、sys)
├── CORE/               # Cortex-M3 内核文件
├── Functions/
│   ├── MyServer/       # 服务器通信模块
│   ├── UI/             # LVGL 界面实现
│   └── WiFi/           # WiFi 连接管理
├── Middlewares/LVGL/   # LVGL 图形库
└── STM32F10x_FWLib/    # ST 标准外设库
```

## 快速开始

### 编译

1. 安装 Keil MDK5
2. 打开 `USER/TOUCH.uvprojx`
3. 编译 (F7) 并下载

### 配置 WiFi

修改 `Functions/MyServer/myserver.h`:

```c
#define MY_WIFI_SSID      "你的WiFi名"
#define MY_WIFI_PWD       "WiFi密码"
#define MY_SERVER_IP      "服务器IP"
#define MY_SERVER_PORT    "8002"
#define MY_DEVICE_ID      "SFP_001"
```

### 接线说明

DHT11、土壤传感器、光敏电阻的接线请参考 `HARDWARE/` 目录下各模块的头文件注释。LCD 和 WiFi 模块直接插在开发板对应接口即可。

## 通信协议

设备通过 TCP 连接服务器，使用 JSON 格式通信：

```json
// 上报传感器数据
{"type":"sensor_data","device_id":"SFP_001","data":{"temp":25,"humi":60,"soil":45,"light":80}}

// 接收控制命令
{"type":"command","cmd":"water_on","cmd_id":12345}
```

支持的命令: `light_on/off`, `water_on/off`, `fan_on/off`, `mode_auto`, `mode_manual`

## 界面预览

系统包含三个主要界面:
- **主界面**: 显示实时传感器数据和设备状态
- **菜单界面**: 手动控制各执行器
- **设置界面**: 调整自动模式阈值参数

## 相关链接

- [正点原子论坛](http://www.openedv.com/)
- [LVGL 官方文档](https://docs.lvgl.io/)

## 开源协议

MIT License
