#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "portmacro.h"

#define OLED_CS     9
#define OLED_RST    10
#define OLED_SA0    11
#define OLED_CLK    12
#define OLED_SDA    13


void send_command(i2c_master_dev_handle_t dev_handle, uint8_t data_wr)
{
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, &data_wr, 20, -1));
    vTaskDelay(10 / portTICK_PERIOD_MS);
}


void app_main(void)
{
    // OLED Power on sequence :
    // Set RES# pin to low for at least and then to high
    // TODO: Still need to confirm if this is needed
    gpio_set_direction(OLED_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(OLED_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(OLED_RST, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    // Set SA0 / DC pin for command output
    gpio_set_direction(OLED_SA0, GPIO_MODE_OUTPUT);
    gpio_set_level(OLED_SA0, 0);

    // I2C configuration
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_1,
        .scl_io_num = OLED_CLK,
        .sda_io_num = OLED_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    // Check if the device is there
    ESP_ERROR_CHECK(i2c_master_probe(bus_handle, 0x3c, -1));

    // Add device to bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x3c,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // turn display off to configure
    send_command(dev_handle, 0xae);

    // set display clock divide ration
    send_command(dev_handle, 0xd5);
    send_command(dev_handle, 0x80);

    // set the COM multiplex ration
    send_command(dev_handle, 0xa8);
    send_command(dev_handle, 0x3f);

    // set display offset
    send_command(dev_handle, 0xd3);
    send_command(dev_handle, 0x00);

    // set start line
    send_command(dev_handle, 0x40);

    // set charge pump
    send_command(dev_handle, 0x8d);
    send_command(dev_handle, 0x14);

    // set memory mode
    send_command(dev_handle, 0x20);
    send_command(dev_handle, 0x00);

    // Command for setting contrast to 100%
    send_command(dev_handle, 0x81);
    send_command(dev_handle, 0xff);

    // Commands for continuous horizontal scroll setup
    // Set orientation to right
    send_command(dev_handle, 0x26);

    // Dummy byte
    send_command(dev_handle, 0x00);
}
