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

#define XSD 50
#define XBT 127
#define XSP 205


uint8_t bt_status = UNLINK;
uint8_t sd_status = UNLINK;

extern uint32_t Time_BLight;
uint8_t Disloop=0;
uint8_t Disonce=0;
uint8_t Distmp=0;

uint8_t WFlag=0;
uint8_t DiisWFlag=0;
uint8_t DisWF=0;
uint8_t DisB=0;
char *pMp3=NULL;

void key_status(void)
{
    if((KeyMode==1||KeyMode==2)){
            while(!gpio_get_level(BTN_A)){
            TFT_jpg_image(55, 101, 0, NULL,  prev1_jpg_start,prev1_jpg_end - prev1_jpg_start);
            vTaskDelay(1/portTICK_RATE_MS);
            Time_BLight = 0;
        }
        
        while(!gpio_get_level(BTN_B)){
            vTaskDelay(1/portTICK_RATE_MS);
            Time_BLight = 0;
        }

        while(!gpio_get_level(BTN_C)){
            TFT_jpg_image(219, 101, 0, NULL, next1_jpg_start,next1_jpg_end - next1_jpg_start);
            vTaskDelay(1/portTICK_RATE_MS);
            Time_BLight = 0;
        }
    
        TFT_jpg_image(55, 101, 0, NULL,  prev_jpg_start,prev_jpg_end - prev_jpg_start);
        TFT_jpg_image(219, 101, 0, NULL, next_jpg_start,next_jpg_end - next_jpg_start);

        if(play_pause == PLAY){
                TFT_jpg_image(120, 85, 0, NULL, play_jpg_start,play_jpg_end - play_jpg_start);
        }else{
                TFT_jpg_image(120, 85, 0, NULL, pause_jpg_start,pause_jpg_end - pause_jpg_start);
        }
    }

    if(KeyMode==3){

    }

}

void Display_hum_tmp(void)
{
    if((KeyMode==1||KeyMode==2||KeyMode==3)){
        if(DisHumTum == DISPLAY_T_H){
            DisHumTum = UNDISPLAY_T_H;
            TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
            TFT_print(dataTmp, 40,10);
            TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
            TFT_print(dataHum, 129,10);
        }
    }
}

void DisplayVolume(uint8_t vol)
{
    if(KeyMode==1||KeyMode==2||KeyMode==3){
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
}

void LinkStatus(void){

        if(KeyMode == M_SD){

            if(sd_status == UNLINK)
            {
                TFT_jpg_image(287, 10, 0, NULL,SDU_jpg_start,SDU_jpg_end - SDU_jpg_start);
            }else
            {
                TFT_jpg_image(287, 10, 0, NULL, SDL_jpg_start,SDL_jpg_end - SDL_jpg_start);
            }
        }else if(KeyMode == M_BT){

            if(bt_status == UNLINK)
            {
                TFT_jpg_image(287, 10, 0, NULL,bt_unlink_jpg_start,bt_unlink_jpg_end - bt_unlink_jpg_start);
            }else
            {
                TFT_jpg_image(287, 10, 0, NULL, bt_linked_jpg_start,bt_linked_jpg_end - bt_linked_jpg_start); 
            }

        }else if (KeyMode == M_SPEAK){
            TFT_jpg_image(287, 10, 0, NULL, SPL_jpg_start,SPL_jpg_end - SPL_jpg_start); 
        }
}



void DisplaySelMode(void){

    
   if(KeyMode == M_SEL&&Stmp==0){

        if(sel_mode==1){
            TFT_jpg_image(XSD, 52, 0, NULL,  SD1_jpg_start,SD1_jpg_end - SD1_jpg_start);
        }else 
        {
            TFT_jpg_image(XSD, 52, 0, NULL,  SD2_jpg_start,SD2_jpg_end -  SD2_jpg_start);
        }
        
        if(sel_mode==2)
        {
            TFT_jpg_image(XBT, 52, 0, NULL,  BT1_jpg_start,BT1_jpg_end - BT1_jpg_start);
        }else {

            TFT_jpg_image(XBT, 52, 0, NULL,  BT2_jpg_start,BT2_jpg_end - BT2_jpg_start);
        }

        if(sel_mode==3)
        {     
            TFT_jpg_image(XSP, 52, 0, NULL,  SP1_jpg_start,SP1_jpg_end - SP1_jpg_start);
        }else
        {      
            TFT_jpg_image(XSP, 52, 0, NULL,  SP2_jpg_start,SP2_jpg_end - SP2_jpg_start); 
        }

   }else if(KeyMode == M_SEL && Stmp==1){

       if(sel_mode==1){
            TFT_jpg_image(0, 0, 0, NULL,  SDW_jpg_start,SDW_jpg_end -  SDW_jpg_start);
        }
        
        if(sel_mode==2)
        {
            TFT_jpg_image(0, 0, 0, NULL,  BTW_jpg_start,BTW_jpg_end - BTW_jpg_start);
        }

        if(sel_mode==3)
        {     
            TFT_jpg_image(0, 0, 0, NULL,  SPW_jpg_start,SPW_jpg_end - SPW_jpg_start);
        }

   }else if(Stmp==3){
 
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


void gobackW(void)
{
        if(WFlag==1){

            if(KeyMode==1)
             TFT_jpg_image(0, 0, 0, NULL,  SDW_jpg_start,SDW_jpg_end -  SDW_jpg_start);

             if(KeyMode==2)
             TFT_jpg_image(0, 0, 0, NULL,  BTW_jpg_start,BTW_jpg_end -  BTW_jpg_start);

             if(KeyMode==3)
             TFT_jpg_image(0, 0, 0, NULL,  SPW_jpg_start,SPW_jpg_end -  SPW_jpg_start);
        }
}


void Disback(void)
{
    if(KeyMode==1||KeyMode==2||KeyMode==3)
    {
        if(DisB==0){
            DisB=1;
            TFT_jpg_image(0, 0, 0, NULL,    master1_jpg_start,master1_jpg_end - master1_jpg_start);

            TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
            TFT_print(dataTmp, 40,10);
            TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
            TFT_print(dataHum, 129,10);
        }
    }
}

void UIInit(void *agr){
    // TFT_bmp_image(0, 0, 0, NULL,  master1_bmp_start,master1_bmp_end - master1_bmp_start);       //Display main frame
    TFT_jpg_image(0, 0, 0, NULL,    master1_jpg_start,master1_jpg_end - master1_jpg_start);

    TFT_jpg_image(XSD, 52, 0, NULL,  SD1_jpg_start,SD1_jpg_end - SD1_jpg_start);
    TFT_jpg_image(XBT, 52, 0, NULL,  BT1_jpg_start,BT1_jpg_end - BT1_jpg_start);
    TFT_jpg_image(XSP, 52, 0, NULL,  SP1_jpg_start,SP1_jpg_end - SP1_jpg_start);

    // TFT_jpg_image(37, 214, 0, NULL,  A2_jpg_start,A2_jpg_end - A2_jpg_start);
    // TFT_jpg_image(129, 214, 0, NULL, B2_jpg_start,B2_jpg_end - B2_jpg_start);
    // TFT_jpg_image(220, 214, 0, NULL, C2_jpg_start,C2_jpg_end - C2_jpg_start);
    // TFT_jpg_image(55, 101, 0, NULL,  prev_jpg_start,prev_jpg_end - prev_jpg_start);
    // TFT_jpg_image(219, 101, 0, NULL, next_jpg_start,next_jpg_end - next_jpg_start);
    // TFT_jpg_image(287, 10, 0, NULL, bt_unlink_jpg_start,bt_unlink_jpg_end - bt_unlink_jpg_start);
    // TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
    // TFT_print(dataTmp, 40,10);
    // TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
    // TFT_print(dataHum, 129,10);
    // TFT_jpg_image(196, 10, 0, NULL, vol7_jpg_start,vol7_jpg_end - vol7_jpg_start);
    // TFT_jpg_image(120, 85, 0, NULL, pause_jpg_start,pause_jpg_end - pause_jpg_start);

    // TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
    // TFT_print(dataTmp, 40,10);
    // TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
    // TFT_print(dataHum, 129,10);

    while(1){

        

        gobackW();

        // if(WFlag==1){
        //     DiisWFlag = 0;
        //     if(DisWF==0){
        //         DisWF =1;
        //         TFT_jpg_image(0, 0, 0, NULL,    master1_jpg_start,master1_jpg_end - master1_jpg_start);
        //         TFT_jpg_image(7, 7, 0, NULL,  tmp_jpg_start,tmp_jpg_end - tmp_jpg_start);
        //         TFT_print("35", 40,10);
        //         TFT_jpg_image(93, 7, 0, NULL,  hum_jpg_start,hum_jpg_end - hum_jpg_start);
        //         TFT_print("35", 129,10);
        //     }
        // }

            if(WFlag==0){
                Disback();
                DisplaySelMode();
                Display_hum_tmp();                  //Display temperature and humidity
                key_status();                       //Display button status
                DisplayVolume(volume);              //Display volume
                LinkStatus();
            }

        printf("sel_mode:%d--Stmp:%d--KeyMode:%d--DisB:%d--WFlag:%d-- Disonce:%d---DiisWFlag:%d ---DisWF:%d\n"
        ,sel_mode,Stmp,KeyMode,DisB,WFlag,Disonce,DiisWFlag,DisWF);

        vTaskDelay(200/portTICK_RATE_MS);
    }
}


void UI_Task_Create(void) {

    xTaskCreatePinnedToCore(UIInit,  "UITask", 5 * 512, NULL,1, NULL, 0);   
}



