#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"


#include "tftspi.h"
#include "tft.h"

#define KEY_THRESHOLD  500

uint32_t Time_BLight=0;
static void vBacklightTimerCallback(void)
{
    static uint32_t BL,tmp=0;
    if(Time_BLight==40)
    {
        led_pwm_init(); 
        for(BL=700;BL>20;){
            BL-=10;
            led_setBrightness(BL);
            vTaskDelay(10/ portTICK_PERIOD_MS);
            tmp = 1;
        }
    }

    if(Time_BLight==0&&tmp == 1){
        tmp =0;
        led_pwm_init(); 
        for(BL=0;BL<700;){
            BL+=10;
            led_setBrightness(BL);
            vTaskDelay(10/ portTICK_PERIOD_MS);
        }
    }
    Time_BLight++;
}

static TimerHandle_t xTimers = NULL;
void CreactTime(void){
    xTimers = xTimerCreate( "BacklightTimer", ( 500/ portTICK_PERIOD_MS), pdTRUE,0,vBacklightTimerCallback);

        if( xTimers == NULL )
          {
               printf("timer failed\n");
          }
          else
          {
              if( xTimerStart( xTimers, 0 ) != pdPASS )
              {
                 
              }
          }
}

 
