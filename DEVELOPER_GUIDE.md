# SmartFlowerPot IoT 开发平台 - 开发者指南

## 公共云服务器接入文档

**版本**: V2.0
**更新日期**: 2025-12-14
**作者**: NixStudio (NixLockhart)

---

## 目录

1. [平台简介](#1-平台简介)
2. [服务器信息](#2-服务器信息)
3. [通信协议规范](#3-通信协议规范)
4. [开发板接入指南](#4-开发板接入指南)
5. [移动APP开发指南](#5-移动app开发指南)
6. [配置系统说明](#6-配置系统说明)
7. [完整示例代码](#7-完整示例代码)
8. [常见问题](#8-常见问题)

---

## 1. 平台简介

SmartFlowerPot IoT 平台是一个开源的物联网开发平台，支持：

- **多设备接入**: 任何支持TCP通信的嵌入式设备
- **动态配置**: 服务器端配置，无需重新烧录固件
- **跨平台APP**: Flutter开发，支持 Android/iOS
- **RESTful API**: 标准HTTP接口，方便二次开发

### 适用场景

- 智能花盆 / 智能鱼缸
- 环境监测站
- 智能家居设备
- 工业传感器采集
- 任何需要远程监控的IoT项目

---

## 2. 服务器信息

### 2.1 服务器地址

| 服务 | 协议 | 地址 | 端口 | 用途 |
|------|------|------|------|------|
| 设备接入 | TCP | 111.228.6.160 | 8002 | 嵌入式设备通信 |
| HTTP API | HTTP | 111.228.6.160 | 8003 | APP/Web 调用 |

### 2.2 设备ID命名规范

设备ID是设备的唯一标识，**请使用以下格式**以避免冲突：

```
格式: {项目缩写}_{你的标识}_{序号}
示例: SFP_ZHANGSAN_001
      TANK_LISI_002
      SENSOR_WANGWU_003
```

**注意**:
- 仅使用大写字母、数字和下划线
- 长度建议不超过20个字符
- 首次连接后设备ID不可更改

---

## 3. 通信协议规范

### 3.1 TCP通信协议 (设备端)

设备与服务器之间使用 **JSON over TCP** 协议通信，每条消息以换行符 `\n` 结尾。

#### 3.1.1 设备注册 (必需)

设备连接后必须首先发送注册消息：

```json
{
  "type": "register",
  "device_id": "SFP_001",
  "seq": 1,
  "version": "1.0"
}
```

服务器响应：
```json
{
  "type": "register_ack",
  "success": true
}
```

#### 3.1.2 心跳包 (建议每30秒)

```json
{
  "type": "heartbeat",
  "device_id": "SFP_001",
  "seq": 2
}
```

服务器响应：
```json
{
  "type": "heartbeat_ack",
  "timestamp": 1702540800
}
```

#### 3.1.3 上报传感器数据

```json
{
  "type": "sensor_data",
  "device_id": "SFP_001",
  "seq": 3,
  "data": {
    "temp": 25,
    "humi": 60,
    "soil": 45,
    "light": 80
  }
}
```

**字段名可自定义**，服务器会原样存储和转发。

#### 3.1.4 上报设备状态

```json
{
  "type": "status",
  "device_id": "SFP_001",
  "seq": 4,
  "status": {
    "mode": 0,
    "light": 1,
    "water": 0,
    "fan": 0
  }
}
```

#### 3.1.5 接收控制命令

服务器下发的命令格式：
```json
{
  "type": "command",
  "cmd": "light_on",
  "cmd_id": 1702540800123
}
```

设备应答：
```json
{
  "type": "ack",
  "device_id": "SFP_001",
  "cmd_id": 1702540800123,
  "success": 1
}
```

#### 3.1.6 接收阈值设置

```json
{
  "type": "threshold",
  "temp_upper": 35,
  "temp_lower": 10,
  "humi_upper": 80,
  "humi_lower": 30,
  "soil_upper": 70,
  "soil_lower": 30,
  "light_upper": 90,
  "light_lower": 20
}
```

### 3.2 HTTP API (APP端)

#### 3.2.1 基础信息

- **Base URL**: `http://111.228.6.160:8003`
- **Content-Type**: `application/json`
- **返回格式**:
```json
{
  "code": 0,
  "message": "success",
  "data": { ... }
}
```

#### 3.2.2 API端点列表

| 方法 | 端点 | 说明 |
|------|------|------|
| GET | `/api/health` | 服务器健康检查 |
| GET | `/api/devices` | 获取所有设备列表 |
| GET | `/api/device/{id}` | 获取单个设备信息 |
| GET | `/api/device/{id}/sensor` | 获取传感器数据 |
| GET | `/api/device/{id}/status` | 获取设备状态 |
| POST | `/api/device/{id}/command` | 发送控制命令 |
| GET | `/api/device/{id}/threshold` | 获取阈值设置 |
| POST | `/api/device/{id}/threshold` | 设置阈值 |
| GET | `/api/device/{id}/config` | 获取设备配置 |
| PUT | `/api/device/{id}/config` | 更新设备配置 |

#### 3.2.3 API详细说明

**健康检查**
```bash
curl http://111.228.6.160:8003/api/health
```
响应：
```json
{
  "code": 0,
  "message": "Server is running",
  "data": {
    "tcp_port": 8002,
    "http_port": 8003,
    "device_count": 5
  }
}
```

**获取设备信息**
```bash
curl http://111.228.6.160:8003/api/device/SFP_001
```
响应：
```json
{
  "code": 0,
  "message": "success",
  "data": {
    "device_id": "SFP_001",
    "online": true,
    "last_heartbeat": "2025-12-14T10:30:00",
    "sensor_data": {
      "temp": 25,
      "humi": 60,
      "soil": 45,
      "light": 80,
      "updated_at": "2025-12-14T10:29:55"
    },
    "status": {
      "mode": 0,
      "light": 1,
      "water": 0,
      "fan": 0
    }
  }
}
```

**发送控制命令**
```bash
curl -X POST http://111.228.6.160:8003/api/device/SFP_001/command \
  -H "Content-Type: application/json" \
  -d '{"cmd": "light_on"}'
```

**默认支持的命令**:
- `light_on` / `light_off` - 控制灯
- `water_on` / `water_off` - 控制水泵
- `fan_on` / `fan_off` - 控制风扇
- `mode_auto` / `mode_manual` - 切换模式
- `get_status` - 请求状态
- `reboot` - 重启设备

---

## 4. 开发板接入指南

### 4.1 硬件要求

- MCU: 支持 TCP/IP 通信的任何单片机
- 推荐: STM32F103 + ESP8266/ESP32
- 其他: ESP32单芯片方案、Arduino + W5500等

### 4.2 STM32 + ESP8266 开发规范

#### 4.2.1 必需修改的配置 (myserver.h)

```c
/* WiFi配置 - 修改为你的WiFi */
#define MY_WIFI_SSID           "你的WiFi名称"
#define MY_WIFI_PWD            "你的WiFi密码"

/* 服务器配置 - 不要修改 */
#define MY_SERVER_IP           "111.228.6.160"
#define MY_SERVER_PORT         "8002"

/* 设备ID - 必须修改为唯一ID */
#define MY_DEVICE_ID           "SFP_你的名字_001"
```

#### 4.2.2 初始化流程

```c
#include "myserver.h"

int main(void)
{
    // 1. 系统初始化
    HAL_Init();
    SystemClock_Config();

    // 2. 外设初始化
    sensor_init();  // 你的传感器初始化

    // 3. WiFi模块初始化
    if (myserver_wifi_init() != 0) {
        // 处理错误
    }

    // 4. 连接WiFi
    if (myserver_wifi_connect() != 0) {
        // 处理错误
    }

    // 5. 连接服务器
    if (myserver_connect() != 0) {
        // 处理错误
    }

    // 6. 主循环
    while (1) {
        myserver_process();           // 心跳处理
        myserver_wireless_control();  // 命令处理

        // 定时上报数据
        if (should_report_data()) {
            my_sensor_data_t data;
            data.temperature = read_temperature();
            data.humidity = read_humidity();
            data.soil_humidity = read_soil();
            data.light_intensity = read_light();
            myserver_send_sensor_data(&data);
        }

        HAL_Delay(50);
    }
}
```

#### 4.2.3 添加自定义传感器

使用动态配置API注册传感器：

```c
// 你的传感器变量
uint8_t water_level = 0;    // 水位传感器
float ph_value = 7.0;       // pH值传感器

void setup_sensors(void)
{
    // 初始化默认配置
    myserver_init_default_config();

    // 注册自定义传感器
    myserver_register_sensor("water_level", MY_DATA_TYPE_INT, &water_level);
    myserver_register_sensor("ph", MY_DATA_TYPE_FLOAT, &ph_value);
}
```

#### 4.2.4 添加自定义控制项

```c
uint8_t heater_status = 0;  // 加热器状态

void heater_on(void) {
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    heater_status = 1;
}

void heater_off(void) {
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    heater_status = 0;
}

void setup_controls(void)
{
    // 注册加热器控制
    myserver_register_control(
        "heater",           // 控制项标识
        "heater_on",        // 开启命令
        "heater_off",       // 关闭命令
        &heater_status,     // 状态变量
        heater_on,          // 开启回调
        heater_off          // 关闭回调
    );
}
```

### 4.3 ESP32 独立开发示例

```c
#include <WiFi.h>

const char* ssid = "你的WiFi";
const char* password = "你的密码";
const char* server_ip = "111.228.6.160";
const int server_port = 8002;
const char* device_id = "ESP32_你的名字_001";

WiFiClient client;

void setup() {
    Serial.begin(115200);

    // 连接WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // 连接服务器
    if (client.connect(server_ip, server_port)) {
        Serial.println("Server connected");

        // 发送注册消息
        String reg = "{\"type\":\"register\",\"device_id\":\"" +
                     String(device_id) + "\",\"seq\":1,\"version\":\"1.0\"}\n";
        client.print(reg);
    }
}

void loop() {
    static unsigned long lastHeartbeat = 0;
    static unsigned long lastData = 0;
    static int seq = 2;

    // 心跳 (每30秒)
    if (millis() - lastHeartbeat > 30000) {
        String hb = "{\"type\":\"heartbeat\",\"device_id\":\"" +
                    String(device_id) + "\",\"seq\":" + String(seq++) + "}\n";
        client.print(hb);
        lastHeartbeat = millis();
    }

    // 数据上报 (每5秒)
    if (millis() - lastData > 5000) {
        int temp = random(20, 30);
        int humi = random(50, 70);

        String data = "{\"type\":\"sensor_data\",\"device_id\":\"" +
                      String(device_id) + "\",\"seq\":" + String(seq++) +
                      ",\"data\":{\"temp\":" + String(temp) +
                      ",\"humi\":" + String(humi) + "}}\n";
        client.print(data);
        lastData = millis();
    }

    // 接收命令
    while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println("Received: " + line);
        // 解析JSON并处理命令...
    }

    // 检查连接
    if (!client.connected()) {
        Serial.println("Reconnecting...");
        client.connect(server_ip, server_port);
    }

    delay(100);
}
```

### 4.4 Arduino + W5500 示例

```c
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server(111, 228, 6, 160);
int port = 8002;
const char* device_id = "ARDUINO_你的名字_001";

EthernetClient client;

void setup() {
    Serial.begin(9600);

    // 初始化以太网
    if (Ethernet.begin(mac) == 0) {
        Serial.println("DHCP failed");
        while(1);
    }

    delay(1000);

    // 连接服务器
    if (client.connect(server, port)) {
        Serial.println("Connected");

        // 注册
        client.print("{\"type\":\"register\",\"device_id\":\"");
        client.print(device_id);
        client.println("\",\"seq\":1,\"version\":\"1.0\"}");
    }
}

void loop() {
    // 类似ESP32的处理逻辑
    // ...
}
```

---

## 5. 移动APP开发指南

### 5.1 Flutter 开发规范

#### 5.1.1 API服务封装

```dart
// lib/services/api_service.dart
import 'dart:convert';
import 'package:http/http.dart' as http;

class ApiService {
  static const String baseUrl = 'http://111.228.6.160:8003';

  /// 获取设备信息
  Future<Map<String, dynamic>?> getDevice(String deviceId) async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/device/$deviceId'),
      );

      if (response.statusCode == 200) {
        final json = jsonDecode(response.body);
        if (json['code'] == 0) {
          return json['data'];
        }
      }
      return null;
    } catch (e) {
      print('API Error: $e');
      return null;
    }
  }

  /// 发送控制命令
  Future<bool> sendCommand(String deviceId, String cmd) async {
    try {
      final response = await http.post(
        Uri.parse('$baseUrl/api/device/$deviceId/command'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'cmd': cmd}),
      );

      if (response.statusCode == 200) {
        final json = jsonDecode(response.body);
        return json['code'] == 0;
      }
      return false;
    } catch (e) {
      print('API Error: $e');
      return false;
    }
  }

  /// 服务器健康检查
  Future<Map<String, dynamic>?> healthCheck() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/api/health'),
      ).timeout(const Duration(seconds: 5));

      if (response.statusCode == 200) {
        return jsonDecode(response.body)['data'];
      }
      return null;
    } catch (e) {
      return null;
    }
  }
}
```

#### 5.1.2 设备数据模型

```dart
// lib/models/device_data.dart

class DeviceData {
  final String deviceId;
  final bool online;
  final SensorData sensorData;
  final DeviceStatus status;

  DeviceData({
    required this.deviceId,
    required this.online,
    required this.sensorData,
    required this.status,
  });

  factory DeviceData.fromJson(Map<String, dynamic> json) {
    return DeviceData(
      deviceId: json['device_id'] ?? '',
      online: json['online'] ?? false,
      sensorData: SensorData.fromJson(json['sensor_data'] ?? {}),
      status: DeviceStatus.fromJson(json['status'] ?? {}),
    );
  }
}

class SensorData {
  final int temp;
  final int humi;
  final int soil;
  final int light;

  SensorData({
    required this.temp,
    required this.humi,
    required this.soil,
    required this.light,
  });

  factory SensorData.fromJson(Map<String, dynamic> json) {
    return SensorData(
      temp: json['temp'] ?? 0,
      humi: json['humi'] ?? 0,
      soil: json['soil'] ?? 0,
      light: json['light'] ?? 0,
    );
  }
}

class DeviceStatus {
  final int mode;
  final int light;
  final int water;
  final int fan;

  DeviceStatus({
    required this.mode,
    required this.light,
    required this.water,
    required this.fan,
  });

  factory DeviceStatus.fromJson(Map<String, dynamic> json) {
    return DeviceStatus(
      mode: json['mode'] ?? 0,
      light: json['light'] ?? 0,
      water: json['water'] ?? 0,
      fan: json['fan'] ?? 0,
    );
  }
}
```

#### 5.1.3 动态UI渲染

```dart
// 根据配置动态创建传感器卡片
Widget buildSensorCard(SensorConfig config, dynamic value) {
  return Container(
    padding: const EdgeInsets.all(12),
    decoration: BoxDecoration(
      color: parseColor(config.color).withOpacity(0.1),
      borderRadius: BorderRadius.circular(12),
    ),
    child: Row(
      children: [
        Icon(getIcon(config.icon), color: parseColor(config.color)),
        const SizedBox(width: 12),
        Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(config.name, style: TextStyle(fontSize: 12, color: Colors.grey)),
            Text(
              '$value${config.unit}',
              style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
            ),
          ],
        ),
      ],
    ),
  );
}
```

### 5.2 Web 开发 (JavaScript)

```javascript
// api.js
const API_BASE = 'http://111.228.6.160:8003';

async function getDevice(deviceId) {
    const response = await fetch(`${API_BASE}/api/device/${deviceId}`);
    const data = await response.json();
    return data.code === 0 ? data.data : null;
}

async function sendCommand(deviceId, cmd) {
    const response = await fetch(`${API_BASE}/api/device/${deviceId}/command`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ cmd })
    });
    const data = await response.json();
    return data.code === 0;
}

// 使用示例
async function controlLight(deviceId, on) {
    const cmd = on ? 'light_on' : 'light_off';
    const success = await sendCommand(deviceId, cmd);
    console.log(success ? '命令发送成功' : '命令发送失败');
}
```

### 5.3 Python 开发

```python
import requests
import json

API_BASE = 'http://111.228.6.160:8003'

def get_device(device_id):
    """获取设备信息"""
    response = requests.get(f'{API_BASE}/api/device/{device_id}')
    data = response.json()
    return data['data'] if data['code'] == 0 else None

def send_command(device_id, cmd):
    """发送控制命令"""
    response = requests.post(
        f'{API_BASE}/api/device/{device_id}/command',
        json={'cmd': cmd}
    )
    data = response.json()
    return data['code'] == 0

# 使用示例
if __name__ == '__main__':
    device = get_device('SFP_001')
    if device:
        print(f"设备状态: {'在线' if device['online'] else '离线'}")
        print(f"温度: {device['sensor_data']['temp']}°C")

        # 控制灯光
        if send_command('SFP_001', 'light_on'):
            print("灯已打开")
```

---

## 6. 配置系统说明

### 6.1 获取设备配置

```bash
curl http://111.228.6.160:8003/api/device/SFP_001/config
```

### 6.2 配置格式

```json
{
  "device_id": "SFP_001",
  "device_name": "我的智能花盆",
  "device_type": "smart_flowerpot",
  "version": "1.0",

  "sensors": [
    {
      "key": "temp",
      "name": "温度",
      "unit": "°C",
      "icon": "thermostat",
      "color": "#FF9800",
      "data_type": "int",
      "enabled": true
    }
  ],

  "controls": [
    {
      "key": "light",
      "name": "补光灯",
      "type": "switch",
      "icon": "lightbulb",
      "color": "#FFC107",
      "cmd_on": "light_on",
      "cmd_off": "light_off",
      "enabled": true
    }
  ],

  "modes": [
    {"value": 0, "name": "自动", "cmd": "mode_auto"},
    {"value": 1, "name": "手动", "cmd": "mode_manual"}
  ]
}
```

### 6.3 更新配置

```bash
curl -X PUT http://111.228.6.160:8003/api/device/SFP_001/config \
  -H "Content-Type: application/json" \
  -d '{
    "device_name": "我的新花盆",
    "sensors": [...],
    "controls": [...]
  }'
```

---

## 7. 完整示例代码

### 7.1 最小可运行示例 (ESP32)

完整代码见本仓库 `examples/esp32_minimal/` 目录。

### 7.2 最小可运行APP (Flutter)

完整代码见本仓库 `app/` 目录。

---

## 8. 常见问题

### Q1: 设备无法连接服务器？

检查以下几点：
1. WiFi是否正确连接
2. 服务器地址是否正确 (111.228.6.160:8002)
3. 防火墙是否放行
4. 使用telnet测试: `telnet 111.228.6.160 8002`

### Q2: APP显示设备离线？

1. 检查设备是否正常发送心跳
2. 心跳超时时间为60秒
3. 检查设备ID是否匹配

### Q3: 命令发送失败？

1. 检查设备是否在线
2. 检查命令是否在有效命令列表中
3. 查看服务器日志确认命令是否送达

### Q4: 如何添加自定义传感器？

1. 在设备端注册传感器 (见4.2.3节)
2. 在配置中添加传感器定义 (见6.2节)
3. APP会自动渲染新传感器

### Q5: 服务器是否收费？

本服务器为公益开放，免费使用。请遵守以下规则：
- 合理使用，不要恶意攻击
- 不要上传敏感数据
- 心跳间隔不低于10秒
- 数据上报间隔不低于1秒

---

## 联系方式

- **GitHub**: [NixLockhart/SmartFlowerPort](https://github.com/NixLockhart/SmartFlowerPort)
- **Issues**: 如有问题请提交Issue

---

**感谢使用 SmartFlowerPot IoT 平台！**
