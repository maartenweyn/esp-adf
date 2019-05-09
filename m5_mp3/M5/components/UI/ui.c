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
#include "nvs_flash.h"
#include "esp_peripherals.h"
#include "bluetooth_service.h"
#include "wm8978.h"
#include "ui.h"
#include "btn.h"
#include "DHT12.h"
#include "BT_play.h"
#include <time.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "neopixelApp.h"

#define KEY_THRESHOLD  500
#define PREV 0X01
#define NEXT 0X02
#define PLAY_STATUS 0X04

#define PLAY 0X01
#define PAUSE 0X02

#define TAG "UI DISPLAY"

uint8_t bt_status = UNLINK_BT;
void Display_bt_status(void)
{
    if(bt_status == UNLINK_BT)
    {
         TFT_jpg_image(287, 10, 0, NULL,bt_unlink_jpg_start,bt_unlink_jpg_end - bt_unlink_jpg_start);
    }else
    {
        TFT_jpg_image(287, 10, 0, NULL, bt_linked_jpg_start,bt_linked_jpg_end - bt_linked_jpg_start); 
    }
    
}

void key_status(void)
{
    while(!gpio_get_level(BTN_A)){
        TFT_jpg_image(55, 101, 0, NULL,  prev1_jpg_start,prev1_jpg_end - prev1_jpg_start);
        TFT_jpg_image(37, 214, 0, NULL,  A1_jpg_start,A1_jpg_end - A1_jpg_start);
        vTaskDelay(1/portTICK_RATE_MS);
    }
    
    while(!gpio_get_level(BTN_B)){
        TFT_jpg_image(129, 214, 0, NULL, B1_jpg_start,B1_jpg_end - B1_jpg_start);
        vTaskDelay(1/portTICK_RATE_MS);
    }

    while(!gpio_get_level(BTN_C)){
        TFT_jpg_image(219, 101, 0, NULL, next1_jpg_start,next1_jpg_end - next1_jpg_start);
        // TFT_bmp_image(219, 101, 0, NULL, next1_bmp_start,next1_bmp_end - next1_bmp_start);
        TFT_jpg_image(220, 214, 0, NULL, C1_jpg_start,C1_jpg_end - C1_jpg_start);
        vTaskDelay(1/portTICK_RATE_MS);
    }
 
    TFT_jpg_image(37, 214,  0, NULL, A2_jpg_start,A2_jpg_end - A2_jpg_start);
    TFT_jpg_image(129, 214, 0, NULL, B2_jpg_start,B2_jpg_end - B2_jpg_start);
    TFT_jpg_image(220, 214, 0, NULL, C2_jpg_start,C2_jpg_end - C2_jpg_start);  
    TFT_jpg_image(55, 101, 0, NULL,  prev_jpg_start,prev_jpg_end - prev_jpg_start);
    TFT_jpg_image(219, 101, 0, NULL, next_jpg_start,next_jpg_end - next_jpg_start);

    if(play_pause == PLAY){
            TFT_jpg_image(120, 85, 0, NULL, play_jpg_start,play_jpg_end - play_jpg_start);
    }else{
            TFT_jpg_image(120, 85, 0, NULL, pause_jpg_start,pause_jpg_end - pause_jpg_start);
    }

}

void Display_hum_tmp(void)
{
    if(DisHumTum == DISPLAY_T_H){
        DisHumTum = UNDISPLAY_T_H;
        TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
        TFT_print(dataTmp, 40,10);
        TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
        TFT_print(dataHum, 129,10);
    }
}


void DisplayVolume(uint8_t vol)
{
   
    if(vol==1){
        TFT_jpg_image(219, 10, 0, NULL, vol6_jpg_start,vol6_jpg_end - vol6_jpg_start);
        TFT_jpg_image(196, 10, 0, NULL, vol8_jpg_start,vol8_jpg_end - vol8_jpg_start);
    }else {
        if((1<vol)&&(vol<=13)){
            TFT_jpg_image(219, 10, 0, NULL, vol1_jpg_start,vol1_jpg_end - vol1_jpg_start);
        }
        else if((13<vol)&&(vol<=26)){
            TFT_jpg_image(219, 10, 0, NULL, vol2_jpg_start,vol2_jpg_end - vol2_jpg_start);
        }
        else if((26<vol)&&(vol<=39)){
            TFT_jpg_image(219, 10, 0, NULL, vol3_jpg_start,vol3_jpg_end - vol3_jpg_start);

        }else if((39<vol)&&(vol<=52)) {
            TFT_jpg_image(219, 10, 0, NULL, vol4_jpg_start,vol4_jpg_end - vol4_jpg_start);
        }
        else {
            TFT_jpg_image(219, 10, 0, NULL, vol5_jpg_start,vol5_jpg_end - vol5_jpg_start);
        }  

        TFT_jpg_image(196, 10, 0, NULL, vol7_jpg_start,vol7_jpg_end - vol7_jpg_start);
    }
}


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
	font_transparent = 1;
	font_forceFixed = 0;
	gray_scale = 0;
    TFT_setRotation(1);
    TFT_setGammaCurve(0);
	TFT_resetclipwin();
	TFT_setFont(DEJAVU24_FONT, NULL);
    ESP_LOGI(TAG, "tft init finish");
}



void UIInit(void *agr){
    TFT_bmp_image(0, 0, 0, NULL,  master_bmp_start,master_bmp_end - master_bmp_start);       //Display main frame
    TFT_jpg_image(37, 214, 0, NULL,  A2_jpg_start,A2_jpg_end - A2_jpg_start);
    TFT_jpg_image(129, 214, 0, NULL, B2_jpg_start,B2_jpg_end - B2_jpg_start);
    TFT_jpg_image(220, 214, 0, NULL, C2_jpg_start,C2_jpg_end - C2_jpg_start);
    TFT_jpg_image(55, 101, 0, NULL,  prev_jpg_start,prev_jpg_end - prev_jpg_start);
    TFT_jpg_image(219, 101, 0, NULL, next_jpg_start,next_jpg_end - next_jpg_start);
    TFT_jpg_image(287, 10, 0, NULL, bt_unlink_jpg_start,bt_unlink_jpg_end - bt_unlink_jpg_start);
    TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
    TFT_print(dataTmp, 40,10);
    TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
    TFT_print(dataHum, 129,10);
    TFT_jpg_image(196, 10, 0, NULL, vol7_jpg_start,vol7_jpg_end - vol7_jpg_start);
    TFT_jpg_image(120, 85, 0, NULL, pause_jpg_start,pause_jpg_end - pause_jpg_start);

    TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
    TFT_print(dataTmp, 40,10);
    TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
    TFT_print(dataHum, 129,10);

    while(1){
        Display_hum_tmp();                  //Display temperature and humidity
        Display_bt_status();                //Displays bluetooth connection status
        key_status();                       //Display button status
        DisplayVolume(volume);              //Display volume

        vTaskDelay(300/portTICK_RATE_MS);
    }
}

void UI_Task_Create(void) {

    xTaskCreatePinnedToCore(UIInit,  "UITask", 5 * 512, NULL,1, NULL, 0);   
}



