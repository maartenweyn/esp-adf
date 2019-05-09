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
#include "esp_sr_iface.h"
#include "esp_sr_models.h"
#include "neopixel.h"
#include "neopixelApp.h"


neopixel_obj_t* nodeNeopixel;
neopixel_obj_t* usr_neopixel_init(int gpio_num, uint16_t num, uint16_t rmtchan) {

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
        return 0;
    } 

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

void ledSetAllo(uint32_t color) {
    color = color << 8;
    for(uint8_t i = 0; i < 12; i++) {
        np_set_pixel_color(&nodeNeopixel->px, i, color+i*50);
        vTaskDelay(1/portTICK_RATE_MS);
    }
    np_show(&nodeNeopixel->px, nodeNeopixel->channel);
}


void neooixel(void){
    static uint8_t i=0,j=0;
    
    if(i%10==0) {
        uint32_t num = rand()%12345678;
        for(j=0;j<150;j++){
        vTaskDelay(4/portTICK_RATE_MS);
        nodeNeopixel->px.brightness=j;
        ledSetAllo(num);
        vTaskDelay(4/portTICK_RATE_MS);
        }
        vTaskDelay(500/portTICK_RATE_MS);
        for(j=150;j>1;j--){
            vTaskDelay(4/portTICK_RATE_MS);
            nodeNeopixel->px.brightness=j;
            ledSetAllo(num);
            vTaskDelay(4/portTICK_RATE_MS);
        }
    }
    i++;
}

uint8_t LightTurn = ON;
void NeooixelTask(void *arg)
{
    nodeNeopixel = usr_neopixel_init(GPIO_NUM_15, 12, 1);

	while(1){
        if(LightTurn == ON){
            neooixel();
        }else
        {
            nodeNeopixel->px.brightness=0;
            np_show(&nodeNeopixel->px, nodeNeopixel->channel);
        }
        vTaskDelay(1/portTICK_RATE_MS);  
    }
}

void Neopixel_Task_Create(void) {

    xTaskCreatePinnedToCore(NeooixelTask,  "NeooixelTask", 2 * 512, NULL,1, NULL, 0);   
}