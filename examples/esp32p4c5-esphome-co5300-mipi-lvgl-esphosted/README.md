# ESPHome：ESP32-P4 + ESP32-C5 核心板驱动 2″ AMOLED

[English](./README_EN.md) | 简体中文

本工程是 osptek 2″ 460×460 AMOLED（CO5300 驱动，1 通道 MIPI DSI，CST820 触摸）在
[osptek ESP32-P4 + ESP32-C5 核心板](https://github.com/osptek/esp32-p4c5-core-board)上的
[ESPHome](https://esphome.io/) 配置。P4 驱动屏幕并运行 ESPHome；C5 通过 SDIO 上的 ESP-Hosted 提供 Wi-Fi。

演示内容：

- 使用 ESPHome 的 `mipi_dsi` 平台和自定义 CO5300 初始化序列点亮屏幕
- 通过 `cst816` 平台读取触摸（CST820 与之兼容）
- 一个 LVGL 页面，中间是一个大圆形开关按钮，并作为开关（"Screen Button"）暴露给 Home Assistant，
  两边都可以切换
- 抖动处理的背景图，平铺显示，避免 16 位色下 LVGL 渐变的色带
- AMOLED 防烧屏：5 分钟无触摸后显示 10 秒随机像素
- 通过 C5 连接 Wi-Fi，包含让 ESP-Hosted 在本板上稳定工作的设置，以及 C5 自身固件的更新实体

## ⚠️ 上电前请确认 FPC 排线方向

排线插反曾导致屏幕损坏。首次上电前，请确认排线**两端**的触点朝向正确。

## 环境要求

- ESPHome **2026.10** 或更新版本（已在 2026.10.2 上测试）
- 核心板、屏幕及其 FPC 排线
- 可选：Home Assistant

## 文件

| 文件 | |
|---|---|
| `osptek-esp32-p4-amoled.yaml` | ESPHome 配置 |
| `images/osptek_bg_gradient.ppm` | 背景条带图（8×460） |
| `tools/dithered_gradient.awk` | 重新生成背景条带，可换颜色或尺寸 |

## 使用步骤

1. 将本文件夹复制到 ESPHome 配置目录（或直接在此处打开）。
2. 在 YAML 同目录下创建 `secrets.yaml`：

   ```yaml
   wifi_ssid: "你的网络"
   wifi_password: "你的密码"
   # 32 字节随机数的 base64，例如：`openssl rand -base64 32`
   osptek_esp32_p4_amoled__encryption_key: "..."
   # 备用热点密码，至少 8 个字符
   osptek_esp32_p4_amoled__ap_password: "..."
   ```

3. 通过 USB 连接开发板，首次烧录：

   ```sh
   esphome run osptek-esp32-p4-amoled.yaml
   ```

   之后可以通过 OTA 无线更新。

4. Home Assistant 会通过 ESPHome 集成发现该设备；添加时使用上面的加密密钥。

页面应显示 "osptek"、圆形按钮和 "Off - tap to turn on"。点击按钮会变为橙色，并在日志中输出触摸坐标。

## 可按需修改的地方

- **日志端口。** 日志输出到 `UART0`，即核心板的 USB 转串口。如果改接 P4 自带的 USB 口，
  请设置 `logger: hardware_uart: USB_SERIAL_JTAG`。
- **日志级别。** `esp32: framework:` 下的 `log_level: INFO` 会显示 C5 的 ESP-Hosted / Wi-Fi 事件，
  便于调试。Wi-Fi 正常后可改回 `ERROR`。
- **固定 IP。** 默认使用 DHCP；注释掉的 `manual_ip:` 块演示了固定地址的写法。
- **亮度。** 由 `init_sequence` 中的 `0x51` 命令设置（0x00–0xFF；`0x99` 约为 60%），仅在启动时设置。
- **Home Assistant 开关。** `text_sensor:` 和 `switch:` 下注释掉的代码块可以让按钮同步并控制
  Home Assistant 中的某个开关。取消注释、修改 `entity_id`，并在 Home Assistant 中为本设备开启
  "Allow the device to perform Home Assistant actions"。
- **背景颜色。** 用你自己的颜色重新生成条带图（十六进制，不带 `#`）：

  ```sh
  awk -v W=8 -v H=460 -v top=1B0A3C -v bot=003B5C -f tools/dithered_gradient.awk > images/osptek_bg_gradient.ppm
  ```

## 硬件相关设置说明

- **SDIO 20 MHz。** 使用 ESP-Hosted 默认的 40 MHz 时，约一半的启动能连上路由器并获得地址，
  但之后无法收发数据。
- **`power_save_mode: none`。** 默认的 light 省电模式下 ping 延迟为 150–1150 ms；关闭后为 4–13 ms。
- **`fast_connect: false`。** 通过 ESP-Hosted 重连已保存的接入点时总是报 "Authentication Failed"。
- **ESP-Hosted 2.12.13。** 这是 ESPHome 2026.10 默认的主机端库版本，C5 应运行相同版本。
  "ESP32-C5 Wi-Fi Firmware" 更新实体会从 ESPHome 的清单中提供该版本，
  "C5 Firmware Installed" / "C5 Firmware Available" 传感器显示两端的版本。
  两端都使用 ESP-Hosted 3.1.0 时无法工作：C5 始终不发送 INIT 事件。
- **初始化序列中的软件复位。** 屏幕的复位线没有连接，如果没有 `0x01` 命令，
  P4 软复位后屏幕会保持黑屏，直到断电重启。
- **显示时序。** 消隐参数来自 osptek 的 ESP-IDF 例程
  （[`esp32p4-idf6-co5300-mipi-lvgl-port-lvgl9`](../esp32p4-idf6-co5300-mipi-lvgl-port-lvgl9/)），
  该例程使用 16 MHz 像素时钟和 330 Mbps 通道速率。本配置使用 8 MHz 和 320 Mbps，同样可用。
- **列偏移。** `0x2A` 设置列 10–469：屏幕可视区域从第 10 列开始。
- **CPU 频率警告。** ESPHome 会提示 ESP32-P4 默认的 400 MHz 属于实验性设置。
  本板在该频率下启动稳定；如果你的板子不稳定，可设置 `esp32: cpu_frequency: 360MHZ`。
