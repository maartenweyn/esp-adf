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
#include "tftspi.h"
#include "tft.h"
#include "dht12.h"
#include "DHT12.h"
#include<math.h>

#define TAG  "DHT12"
float tmp=0.0, hum=0.0,Ttmp=0.0, Thum=0.0;
char dataHum[20] = "35";
char dataTmp[20] = "35 ";

uint8_t DisHumTum = UNDISPLAY_T_H;
void dht12_task(void *arg){
    // dht12_init();
    while(1) {
            //Read the temperature and humidity value
            dataRead(&tmp, &hum);      
      
            if(fabs(Ttmp-tmp)||fabs(Thum-hum))
            {
                sprintf(dataHum, "%0.0f", hum);
                sprintf(dataTmp, "%0.0f", tmp);
                DisHumTum = DISPLAY_T_H;
            }
            vTaskDelay(1000 / portTICK_RATE_MS);
            Ttmp = tmp; 
            Thum = hum;
           
        }
}

void dht12_task_create(void){
    xTaskCreatePinnedToCore(dht12_task,  "dht12_task",   2 * 1024, NULL, 5, NULL,tskNO_AFFINITY);      
}

