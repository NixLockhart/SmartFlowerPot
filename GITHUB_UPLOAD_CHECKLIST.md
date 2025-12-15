# SmartFlowerPot GitHub 上传文件清单

## 项目结构总览

```
SFP_v1.2/
├── 📄 README.md                    ✅ 需要更新
├── 📄 DEVELOPER_GUIDE.md           ✅ 上传 (开发者指南)
├── 📄 LICENSE                      ✅ 上传
├── 📄 .gitignore                   ✅ 上传 (已更新)
│
├── 📁 Server/                      ✅ Python服务器 (全部上传)
├── 📁 app/                         ✅ Flutter APP (部分上传)
├── 📁 Functions/                   ✅ STM32功能模块 (全部上传)
├── 📁 HARDWARE/                    ✅ 硬件驱动 (全部上传)
├── 📁 SYSTEM/                      ✅ 系统文件 (全部上传)
├── 📁 CORE/                        ✅ 核心文件 (全部上传)
├── 📁 USER/                        ⚠️ 用户代码 (部分上传)
├── 📁 Middlewares/                 ✅ 中间件LVGL (全部上传)
├── 📁 STM32F10x_FWLib/             ✅ ST标准库 (全部上传)
├── 📁 Utils/                       ✅ 工具 (全部上传)
│
├── 📁 OBJ/                         ❌ 不上传 (构建产物)
└── 📁 .claude/                     ❌ 不上传 (AI助手配置)
```

---

## 详细文件清单

### 1. 根目录文件 ✅

| 文件 | 状态 | 说明 |
|------|------|------|
| `README.md` | 需更新 | 项目说明文档 |
| `DEVELOPER_GUIDE.md` | ✅ 上传 | 开发者接入指南 |
| `LICENSE` | ✅ 上传 | 开源许可证 |
| `.gitignore` | ✅ 上传 | Git忽略配置 |
| `keilkilll.bat` | 可选 | Keil清理脚本 |

---

### 2. Server/ - Python服务器 ✅ 全部上传

```
Server/
├── server.py              ✅ 主服务器程序
├── config_manager.py      ✅ 配置管理模块
├── requirements.txt       ✅ Python依赖
├── API_DOC.md             ✅ API文档
└── config/
    ├── devices/
    │   └── .gitkeep       ✅ 占位文件 (设备配置运行时生成)
    └── templates/
        └── smart_flowerpot.json  ✅ 默认配置模板
```

---

### 3. app/ - Flutter APP ⚠️ 部分上传

**需要上传的文件:**

```
app/
├── lib/                           ✅ 核心代码
│   ├── main.dart
│   ├── models/
│   │   ├── config_model.dart
│   │   └── device_data.dart
│   ├── screens/
│   │   ├── config_editor_screen.dart
│   │   ├── dynamic_home_screen.dart
│   │   └── home_screen.dart
│   ├── services/
│   │   ├── api_service.dart
│   │   └── config_service.dart
│   ├── utils/
│   │   ├── color_utils.dart
│   │   └── icon_utils.dart
│   └── widgets/
│       ├── dynamic_widgets.dart
│       └── sensor_card.dart
├── pubspec.yaml                   ✅ 依赖配置
├── pubspec.lock                   ✅ 依赖锁定
├── analysis_options.yaml          ✅ 分析配置
├── README.md                      ✅ APP说明
├── .gitignore                     ✅ Flutter gitignore
├── android/                       ✅ Android配置
├── ios/                           ✅ iOS配置
├── linux/                         ✅ Linux配置
├── macos/                         ✅ macOS配置
├── windows/                       ✅ Windows配置
├── web/                           ✅ Web配置
└── test/                          ✅ 测试文件
```

**不上传的目录 (Flutter .gitignore 已配置):**
- `build/` - 构建输出
- `.dart_tool/` - Dart工具缓存
- `.idea/` - IDE配置

---

### 4. Functions/ - STM32功能模块 ✅ 全部上传

```
Functions/
├── MyServer/                      ✅ 服务器通信模块
│   ├── myserver.c
│   └── myserver.h
├── UI/                            ✅ 用户界面
│   ├── ui.c
│   └── ui.h
└── WiFi/                          ✅ WiFi功能
    ├── wifi.c
    └── wifi.h
```

---

### 5. HARDWARE/ - 硬件驱动 ✅ 全部上传

```
HARDWARE/
├── 24CXX/          ✅ EEPROM驱动
├── ADC/            ✅ ADC驱动
├── ATK_MW8266D/    ✅ ESP8266驱动
├── BEEP/           ✅ 蜂鸣器驱动
├── BUMP/           ✅ 水泵驱动
├── DHT11/          ✅ 温湿度传感器
├── IIC/            ✅ I2C驱动
├── KEY/            ✅ 按键驱动
├── LCD/            ✅ LCD驱动
├── LED/            ✅ LED驱动
├── LSENS/          ✅ 光敏传感器
├── SPI/            ✅ SPI驱动
├── TIMER/          ✅ 定时器驱动
├── TOUCH/          ✅ 触摸驱动
├── TPAD/           ✅ 触摸按键
├── TS/             ✅ 触摸屏
├── USART3/         ✅ 串口3驱动
└── W25QXX/         ✅ Flash驱动
```

---

### 6. SYSTEM/ - 系统文件 ✅ 全部上传

```
SYSTEM/
├── adcx/           ✅ ADC扩展
├── delay/          ✅ 延时函数
├── sys/            ✅ 系统函数
└── usart/          ✅ 串口函数
```

---

### 7. CORE/ - 核心文件 ✅ 全部上传

```
CORE/
├── core_cm3.c              ✅
├── core_cm3.h              ✅
└── startup_stm32f10x_hd.s  ✅ 启动文件
```

---

### 8. USER/ - 用户代码 ⚠️ 部分上传

**上传:**
```
USER/
├── main.c                  ✅ 主程序
├── stm32f10x.h             ✅ 芯片头文件
├── stm32f10x_conf.h        ✅ 配置文件
├── stm32f10x_it.c          ✅ 中断处理
├── stm32f10x_it.h          ✅
├── system_stm32f10x.c      ✅ 系统初始化
├── system_stm32f10x.h      ✅
├── TOUCH.uvprojx           ✅ Keil工程文件
└── DebugConfig/            ✅ 调试配置
```

**不上传 (.gitignore 已配置):**
- `TOUCH.uvoptx` - 用户选项
- `TOUCH.uvguix.*` - GUI配置
- `TOUCH.map` - 链接映射
- `startup_stm32f10x_hd.lst` - 列表文件
- `JLinkSettings.ini` - 调试器配置

---

### 9. Middlewares/ - 中间件 ✅ 全部上传

```
Middlewares/
└── LVGL/           ✅ LVGL图形库
```

---

### 10. STM32F10x_FWLib/ - ST标准库 ✅ 全部上传

包含所有ST官方外设库文件。

---

### 11. Utils/ - 工具 ✅ 全部上传

包含项目工具文件。

---

## 不上传的目录/文件 ❌

| 路径 | 原因 |
|------|------|
| `OBJ/` | Keil编译输出 |
| `.claude/` | AI助手配置 |
| `app/build/` | Flutter构建输出 |
| `app/.dart_tool/` | Dart缓存 |
| `*.o, *.d, *.crf` | 编译中间文件 |
| `*.hex, *.bin, *.axf` | 固件文件 |
| `*.map, *.lst` | 链接/列表文件 |
| `__pycache__/` | Python缓存 |

---

## 上传前检查清单

### 必须完成:

- [ ] 更新 `README.md` 为完整的项目说明
- [ ] 确认 `Server/config/templates/smart_flowerpot.json` 存在
- [ ] 确认 `.gitignore` 已正确配置
- [ ] 移除任何敏感信息 (WiFi密码等)

### 敏感信息检查:

**需要修改的文件:**

1. **`Functions/MyServer/myserver.h`** - WiFi配置
```c
// 用户需要自行修改:
#define MY_WIFI_SSID           "你的WiFi名称"
#define MY_WIFI_PWD            "你的WiFi密码"
```

2. **服务器IP** - 可以保留公开服务器地址

---

## Git 命令参考

```bash
# 1. 查看将要提交的文件
git status

# 2. 添加所有文件 (遵循.gitignore)
git add .

# 3. 检查暂存的文件
git diff --staged --name-only

# 4. 提交
git commit -m "v2.0: 添加动态配置支持和Flutter APP"

# 5. 推送
git push origin main
```

---

## 统计信息

| 类别 | 文件数(约) | 说明 |
|------|-----------|------|
| 服务器代码 | 5 | Python + JSON |
| Flutter APP | 15+ | Dart源码 |
| STM32固件 | 100+ | C源码 + 驱动 |
| 文档 | 4 | MD文档 |
| 配置文件 | 10+ | 各类配置 |

---

**最后更新**: 2025-12-14
