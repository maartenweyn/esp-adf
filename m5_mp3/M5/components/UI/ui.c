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



void UIInit(void *agr){
    esp_err_t ret;
    TFT_PinsInit();     //Initialize TFT_LCD pins
    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,				// set SPI MISO pin
        .mosi_io_num=PIN_NUM_MOSI,				// set SPI MOSI pin
        .sclk_io_num=PIN_NUM_CLK,				// set SPI CLK pin
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
		.max_transfer_sz = 6*1024,
    };
    spi_lobo_device_interface_config_t devcfg={
        .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=-1,                       // we will use external CS pin
	    .spics_ext_io_num=PIN_NUM_CS,           // external CS pin
		.flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

	ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret==ESP_OK);
	disp_spi = spi;

    ret = spi_lobo_device_select(spi, 1);       //Test the SPI CS
    assert(ret==ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret==ESP_OK);

    TFT_display_init();     //LCD Register initialization
    TFT_setRotation(1);
    TFT_setFont(DEJAVU24_FONT, NULL);

    TFT_jpg_image(0, 0, 0, NULL,  master_jpg_start,master_jpg_end - master_jpg_start);      //Display main frame
    // TFT_bmp_image(0, 0, 0, NULL,  master_bmp_start,master_bmp_end - master_bmp_start);       //Display main frame
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
    while(1){
        Display_hum_tmp();                  //Display temperature and humidity
        Display_bt_status();                //Displays bluetooth connection status
        key_status();                       //Display button status
        DisplayVolume(volume);              //Display volume
        vTaskDelay(1/portTICK_RATE_MS);
    }
}

void UI_Task_Create(void) {

    xTaskCreatePinnedToCore(UIInit,  "UITask", 3 * 1024, NULL,4, NULL, tskNO_AFFINITY);   
}




