/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "driver/gpio.h"

#include "sdcard.h"
#include "board.h"

static const char *TAG = "SDCARD";
int g_gpio = -1;

static void sdmmc_card_print_info(const sdmmc_card_t *card)
{
    ESP_LOGD(TAG, "Name: %s\n", card->cid.name);
    ESP_LOGD(TAG, "Type: %s\n", (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC");
    ESP_LOGD(TAG, "Speed: %s\n", (card->csd.tr_speed > 25000000) ? "high speed" : "default speed");
    ESP_LOGD(TAG, "Size: %lluMB\n", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    ESP_LOGD(TAG, "CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
             card->csd.csd_ver,
             card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
    ESP_LOGD(TAG, "SCR: sd_spec=%d, bus_width=%d\n", card->scr.sd_spec, card->scr.bus_width);
}

static void _setPins(int8_t miso, int8_t mosi, int8_t clk, int8_t cs, int8_t dat1, int8_t dat2)
{
    if (miso >= 0) { // miso/dat0/dO
		gpio_pad_select_gpio(miso);
		gpio_set_direction(miso, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_pull_mode(miso, GPIO_PULLUP_ONLY);
        gpio_set_level(miso, 1);
    }
    if (mosi >= 0) { // mosi/cmd/dI
		gpio_pad_select_gpio(mosi);
		gpio_set_direction(mosi, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_pull_mode(mosi, GPIO_PULLUP_ONLY);
        gpio_set_level(mosi, 1);
    }
    if (clk >= 0) { // clk/sck
		gpio_pad_select_gpio(clk);
		gpio_set_direction(clk, GPIO_MODE_INPUT_OUTPUT_OD);
        gpio_set_pull_mode(clk, GPIO_PULLUP_ONLY);
		gpio_set_level(clk, 1);
    }
    if (cs >= 0) { // cs/dat3
        gpio_pad_select_gpio(cs);
        gpio_set_direction(cs, GPIO_MODE_INPUT_OUTPUT);
        gpio_set_pull_mode(cs, GPIO_PULLUP_ONLY);
        gpio_set_level(cs, 1);
    }
    if (dat1 >= 0) { // dat1
        gpio_pad_select_gpio(dat1);
        gpio_set_direction(dat1, GPIO_MODE_INPUT_OUTPUT_OD);
        gpio_set_pull_mode(dat1, GPIO_PULLUP_ONLY);
        gpio_set_level(dat1, 1);
    }
    if (dat2 >= 0) { // dat2
        gpio_pad_select_gpio(dat2);
        gpio_set_direction(dat2, GPIO_MODE_INPUT_OUTPUT_OD);
        gpio_set_pull_mode(dat2, GPIO_PULLUP_ONLY);
        gpio_set_level(dat2, 1);
    }
}

esp_err_t sdcard_mount(const char *base_path)
{
    ESP_LOGI(TAG, "Using SPI peripheral");
    esp_err_t ret;
    sdmmc_card_t *card;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();

    slot_config.gpio_miso = 19;
    slot_config.gpio_mosi = 23;
    slot_config.gpio_sck  = 18;
    slot_config.gpio_cs   = 4;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0
    };

    host.slot = VSPI_HOST;
    host.max_freq_khz = 40000;
    slot_config.dma_channel = 2;
    _setPins(slot_config.gpio_miso, slot_config.gpio_mosi, slot_config.gpio_sck, slot_config.gpio_cs, -1, -1);
    ret = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot_config, &mount_config, &card);


    switch (ret) {
        case ESP_OK:
            // Card has been initialized, print its properties
            sdmmc_card_print_info(card);
            ESP_LOGI(TAG, "CID name %s!\n", card->cid.name);
            break;

        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(TAG, "File system already mounted");
            break;

        case ESP_FAIL:
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
            break;

        default:
            ESP_LOGE(TAG, "Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
            break;
    }

    return ret;

}

esp_err_t sdcard_unmount(void)
{
    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();

    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "File system not mounted");
    }
    return ret;
}

bool sdcard_is_exist()
{
    // if (g_gpio >= 0) {
    //     return (gpio_get_level(g_gpio) == 0x00);
    // }
    return true;
}

int IRAM_ATTR sdcard_read_detect_pin(void)
{
    if (g_gpio >= 0) {
        return gpio_get_level(g_gpio);
    }
    return 0;
}

esp_err_t sdcard_destroy()
{
    if (g_gpio >= 0) {
        return gpio_isr_handler_remove(g_gpio);
    }
    return ESP_OK;
}

esp_err_t sdcard_init(int card_detect_pin, void (*detect_intr_handler)(void *), void *isr_context)
{
    esp_err_t ret = ESP_OK;
    if (card_detect_pin >= 0) {
        gpio_set_direction(card_detect_pin, GPIO_MODE_INPUT);
        if (detect_intr_handler) {
            gpio_set_intr_type(card_detect_pin, GPIO_INTR_ANYEDGE);
            gpio_isr_handler_add(card_detect_pin, detect_intr_handler, isr_context);
            gpio_intr_enable(card_detect_pin);
        }
        gpio_pullup_en(card_detect_pin);
    }
    g_gpio = card_detect_pin;
    return ret;
}
