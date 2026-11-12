# ESPHome: 2″ AMOLED on the ESP32-P4 + ESP32-C5 core board

English | [简体中文](./README.md)

An [ESPHome](https://esphome.io/) configuration for the osptek 2″ 460×460 AMOLED (CO5300 driver
over 1-lane MIPI DSI, CST820 touch) on the
[osptek ESP32-P4 + ESP32-C5 core board](https://github.com/osptek/esp32-p4c5-core-board).
The P4 drives the display and runs ESPHome; the C5 provides Wi-Fi through ESP-Hosted over SDIO.

It shows:

- the display brought up with ESPHome's `mipi_dsi` platform and a custom CO5300 init sequence
- touch through the `cst816` platform (the CST820 is compatible)
- an LVGL page with a big round button that toggles on/off and is exposed to Home Assistant as a
  switch ("Screen Button"), so you can toggle it from either side
- a dithered background image, tiled to avoid the banding of LVGL gradients at 16-bit colour
- AMOLED burn-in care: 10 s of random pixels after 5 minutes without a touch
- Wi-Fi via the C5, with the settings that make ESP-Hosted reliable on this board, and an update
  entity for the C5's own firmware

## ⚠️ Check the FPC cable orientation before powering up

A reversed display FPC cable has damaged a panel. Check the cable's contacts face the right way
at **both** ends before you apply power for the first time.

## Requirements

- ESPHome **2026.10** or later (tested with 2026.10.2)
- The core board, the display and its FPC cable
- Optional: Home Assistant

## Files

| File | |
|---|---|
| `osptek-esp32-p4-amoled.yaml` | the ESPHome configuration |
| `images/osptek_bg_gradient.ppm` | the background strip (8×460) |
| `tools/dithered_gradient.awk` | regenerates the strip, for other colours or sizes |

## Setup

1. Copy this folder to your ESPHome config directory (or open it there).
2. Create `secrets.yaml` next to the YAML with:

   ```yaml
   wifi_ssid: "your network"
   wifi_password: "your password"
   # 32 random bytes, base64: e.g. `openssl rand -base64 32`
   osptek_esp32_p4_amoled__encryption_key: "..."
   # fallback hotspot password, 8+ characters
   osptek_esp32_p4_amoled__ap_password: "..."
   ```

3. Connect the board over USB and flash it the first time:

   ```sh
   esphome run osptek-esp32-p4-amoled.yaml
   ```

   After that, updates can go over the air.

4. In Home Assistant, the device is discovered through the ESPHome integration; use the
   encryption key above when adding it.

The page should show "osptek", the round button and "Off - tap to turn on". Tapping the button
turns it orange and logs the touch position.

## Things you may want to change

- **Logger port.** Logs go to `UART0`, the core board's USB-UART bridge port. If you connect to
  the P4's own USB port instead, set `logger: hardware_uart: USB_SERIAL_JTAG`.
- **Log level.** `log_level: INFO` under `esp32: framework:` shows ESP-Hosted / Wi-Fi events from
  the C5, which help while bringing the board up. Set it back to `ERROR` once Wi-Fi works.
- **Fixed IP.** DHCP is used by default; a commented `manual_ip:` block shows a fixed address.
- **Brightness.** Set by the `0x51` command in `init_sequence` (0x00–0xFF; `0x99` is about 60%).
  It is only set at boot.
- **Home Assistant switch.** Commented blocks under `text_sensor:` and `switch:` make the button
  mirror and control a Home Assistant switch. Uncomment them, change the `entity_id`, and turn on
  "Allow the device to perform Home Assistant actions" for this device in Home Assistant.
- **Background colours.** Regenerate the strip with your own colours (hex, no `#`):

  ```sh
  awk -v W=8 -v H=460 -v top=1B0A3C -v bot=003B5C -f tools/dithered_gradient.awk > images/osptek_bg_gradient.ppm
  ```

## Notes on the hardware settings

- **SDIO at 20 MHz.** At ESP-Hosted's 40 MHz default, about half of boots associated with the
  access point and got an address but then passed no traffic.
- **`power_save_mode: none`.** With the default light power save, ping times were 150–1150 ms;
  without it, 4–13 ms.
- **`fast_connect: false`.** Reconnecting to the saved access point always failed with
  "Authentication Failed" through ESP-Hosted.
- **ESP-Hosted 2.12.13.** ESPHome 2026.10's default host library. The C5 should run the same
  version; the "ESP32-C5 Wi-Fi Firmware" update entity offers it from ESPHome's manifest, and
  the "C5 Firmware Installed" / "C5 Firmware Available" sensors show both versions. ESP-Hosted
  3.1.0 on both sides did not work: the C5 never sent its INIT event.
- **Software reset in the init sequence.** The panel's reset line isn't wired, so without the
  `0x01` command a soft reset of the P4 leaves the panel dark until a power cycle.
- **Display timings.** The porches come from osptek's ESP-IDF example
  ([`esp32p4-idf6-co5300-mipi-lvgl-port-lvgl9`](../esp32p4-idf6-co5300-mipi-lvgl-port-lvgl9/)),
  which uses a 16 MHz pixel clock and 330 Mbps lane rate. This config uses 8 MHz and 320 Mbps,
  which also work.
- **Column offset.** `0x2A` sets columns 10–469: the panel's visible area starts at column 10.
- **CPU clock warning.** ESPHome warns that its default 400 MHz on the ESP32-P4 is experimental.
  The board has booted reliably at it; set `esp32: cpu_frequency: 360MHZ` if yours doesn't.
