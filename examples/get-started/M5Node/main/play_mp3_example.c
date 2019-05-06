/* Play mp3 file by audio pipeline

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "audio_mem.h"
#include "mp3_decoder.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "filter_resample.h"
#include "esp_sr_iface.h"
#include "esp_sr_models.h"
#include "tftspi.h"
#include "tft.h"
#include "neopixel.h"
#include "dht12.h"
#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "ringbuf.h"
#include "audio_hal.h"

#define BTN_A   39
#define BTN_B   38
#define BTN_C   37
#define IR_IN   35
#define IR_OUT  12
#define BTN  ((1ULL<<BTN_A) | (1ULL<<BTN_B) | (1ULL<<BTN_C) | (1ULL<<IR_IN))

SemaphoreHandle_t spiMux = NULL;

static const char *TAG = "PLAY_MP3_FLASH";
/*
   To embed it in the app binary, the mp3 file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t adf_music_mp3_start[] asm("_binary_adf_music_mp3_start");
extern const uint8_t adf_music_mp3_end[]   asm("_binary_adf_music_mp3_end");
void ledSetAll(uint32_t color);

static const char *SPEAK_TAG = "example_asr_keywords";

static const char *EVENT_TAG = "asr_event";


display_config_t dconfig;
extern exspi_device_handle_t *disp_spi;
void lcdInit(void) {
    disp_spi = (exspi_device_handle_t *)malloc(sizeof(exspi_device_handle_t));
    dconfig.speed = 40000000;
    dconfig.rdspeed = 40000000;
    dconfig.type = DISP_TYPE_M5STACK;
    dconfig.host = HSPI_HOST;
    dconfig.mosi = 23;
    dconfig.miso = 19;
    dconfig.sck = 18;
    dconfig.cs = 14;
    dconfig.dc = 27;
    dconfig.tcs = -1;
    dconfig.rst = 33;
    dconfig.bckl = 32;
    dconfig.bckl_on = 1;
    dconfig.color_bits = 24;
    dconfig.gamma = 0;
    dconfig.width = 320;
    dconfig.height = 240;
    dconfig.invrot = 3;
    dconfig.bgr = 8;
    dconfig.touch = TOUCH_TYPE_NONE;
    esp_err_t ret = TFT_display_init(&dconfig);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "TFT init fail");
	}
    spi_set_speed(disp_spi, dconfig.speed);

	font_rotate = 0;
	text_wrap = 1;
	font_transparent = 0;
	font_forceFixed = 0;
	gray_scale = 0;
    TFT_setRotation(1);
    TFT_setGammaCurve(0);
	TFT_resetclipwin();
	TFT_setFont(DEFAULT_FONT, NULL);

    ESP_LOGI(TAG, "tft init finish");
}
// neopixel
typedef struct _machine_neopixel_obj_t {
    rmt_channel_t channel;
    int gpio_num;
    pixel_settings_t px;
} neopixel_obj_t;

neopixel_obj_t* nodeNeopixel;

static neopixel_obj_t* usr_neopixel_init(int gpio_num, uint16_t num, uint16_t rmtchan) {
    neopixel_obj_t* neopixel_n = (neopixel_obj_t *)malloc(sizeof(neopixel_obj_t));

    neopixel_n->px.timings.mark.level0 = 1;
	neopixel_n->px.timings.mark.level1 = 0;
	neopixel_n->px.timings.space.level0 = 1;
	neopixel_n->px.timings.space.level1 = 0;
	neopixel_n->px.timings.reset.level0 = 0;
	neopixel_n->px.timings.reset.level1 = 0;
	neopixel_n->px.timings.mark.duration0 = 12,

    neopixel_n->px.nbits = 24;
    neopixel_n->px.timings.mark.duration1 = 14;
    neopixel_n->px.timings.space.duration0 = 7;
    neopixel_n->px.timings.space.duration1 = 16;
    neopixel_n->px.timings.reset.duration0 = 600;
    neopixel_n->px.timings.reset.duration1 = 600;

    neopixel_n->channel = rmtchan;
    neopixel_n->gpio_num = gpio_num;
    neopixel_n->px.pixel_count = num;
    neopixel_n->px.pixels = (uint8_t *)malloc((neopixel_n->px.nbits/8) * neopixel_n->px.pixel_count);
    neopixel_n->px.brightness = 50;
    sprintf(neopixel_n->px.color_order, "GRBW");

    if (neopixel_init(neopixel_n->gpio_num, rmtchan) != ESP_OK) {
        printf("neopixel init error\r\n");
        return ;
    } 

    printf("neopixel init ok...\r\n");

    np_clear(&neopixel_n->px);
    np_show(&neopixel_n->px, neopixel_n->channel);
    return neopixel_n;
}

void ledSetAll(uint32_t color) {
    color = color << 8;
    for(uint8_t i = 0; i < 12; i++) {
        np_set_pixel_color(&nodeNeopixel->px, i, color);
    }
    np_show(&nodeNeopixel->px, nodeNeopixel->channel);
}

float tmp, hum;
uint8_t irData = 1;
uint8_t irDateLast = 1;

void sd_test(void) {
    static FILE *file;
    file = fopen("/sdcard/a.txt", "w+");
    if (!file) {
        printf("Error opening file\n");
        return -1;
    }
    fprintf(file, "Hello 123!\n");
    fclose(file);

    file = fopen("/sdcard/a.txt", "r");
    if (!file) {
        printf("Error opening file\n");
        return -1;
    }
    char line[64];
    fgets(line, sizeof(line), file);
    fclose(file);
    ESP_LOGI(TAG, "a.txt: %s", line);
}

void app_main(void) {
    esp_log_level_set("gpio", ESP_LOG_WARN);
    lcdInit();

    TFT_fillScreen(TFT_BLACK);
    vTaskDelay(1000);
    TFT_fillScreen(TFT_RED);


    esp_periph_config_t periph_cfg = DEFAULT_ESP_PHERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    // Initialize SD Card peripheral
    periph_sdcard_cfg_t sdcard_cfg = {
        .root = "/sdcard",
        .card_detect_pin = get_sdcard_intr_gpio(), //GPIO_NUM_34
    };
    esp_periph_handle_t sdcard_handle = periph_sdcard_init(&sdcard_cfg);
    // Start sdcard & button peripheral
    esp_periph_start(set, sdcard_handle);

    // Wait until sdcard is mounted
    while (!periph_sdcard_is_mounted(sdcard_handle)) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    sd_test();
    TFT_fillScreen(TFT_BLACK);
    vTaskDelay(1000);
    TFT_fillScreen(TFT_RED);
    sd_test();
    TFT_jpg_image(0, 0, 0, "/sdcard/1-1.jpg", NULL, 0);
}
