/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_ldo_regulator.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_lcd_co5300.h"
#include "app_lcd.h"

static const char *TAG = "app_lcd";

#define EXAMPLE_LDO_MIPI_CHAN               (3)
#define EXAMPLE_LDO_MIPI_VOLTAGE_MV         (2500)

#define EXAMPLE_PIN_NUM_LCD_RST             (-1)

static esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
static esp_lcd_panel_io_handle_t mipi_dbi_io;
static esp_lcd_panel_handle_t display_handle;

static void lcd_ldo_power_on(void)
{
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = EXAMPLE_LDO_MIPI_CHAN,
        .voltage_mv = EXAMPLE_LDO_MIPI_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
}

static const co5300_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    // {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0x53, (uint8_t[]){0x20}, 1, 0},
    {0x51, (uint8_t[]){0xFF}, 1, 0},
    {0x63, (uint8_t[]){0xFF}, 1, 0},
    {0x2A, (uint8_t[]){0x00, 0x0A, 0x01, 0xD5}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xCB}, 4, 0},
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0x29, (uint8_t[]){0x00}, 0, 0},
};

esp_err_t app_lcd_init(esp_lcd_panel_handle_t *panel_handle)
{
    lcd_ldo_power_on();

    ESP_LOGD(TAG, "Install LCD driver");
    esp_lcd_dsi_bus_config_t bus_config = {                    \
        .bus_id = 0,                                           \
        .num_data_lanes = 1,                                   \
        .phy_clk_src = 0,                                      \
        .lane_bit_rate_mbps = 330,                             \
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_dbi_io_config_t dbi_config = CO5300_PANEL_IO_DBI_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    ESP_LOGI(TAG, "Install LCD driver of co5300");
    esp_lcd_dpi_panel_config_t dpi_config = {                 \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,          \
        .dpi_clock_freq_mhz = 16,                             \
        .virtual_channel = 0,                                 \
        .in_color_format = EXAMPLE_MIPI_DPI_PX_FORMAT,        \
        .num_fbs = EXAMPLE_LCD_BUF_NUM,                       \
        .video_timing = {                                     \
            .h_size = EXAMPLE_LCD_H_RES,                      \
            .v_size = EXAMPLE_LCD_V_RES,                      \
            .hsync_back_porch = 72,                           \
            .hsync_pulse_width = 4,                           \
            .hsync_front_porch = 32,                          \
            .vsync_back_porch = 4,                            \
            .vsync_pulse_width = 2,                           \
            .vsync_front_porch = 6,                           \
        },                                                    \
    };

    co5300_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,      // Uncomment these line if use custom initialization commands
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(co5300_lcd_init_cmd_t),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = EXAMPLE_LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_co5300(mipi_dbi_io, &panel_config, &display_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(display_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(display_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_co5300_set_brightness(display_handle, 80));

    *panel_handle = display_handle;

    return ESP_OK;
}
