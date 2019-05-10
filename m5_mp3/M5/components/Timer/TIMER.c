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

#define BL_ON 0
#define BL_OFF 40
uint32_t Time_BLight=0;
uint8_t OFF_Flag = 0;
static void vBacklightTimerCallback(void *arg)
{
    static uint32_t BL,tmp=0;
    if(Time_BLight == BL_OFF)
    {
        OFF_Flag =1;
        led_pwm_init(); 
        for(BL=700;BL>20;){
            BL-=10;
            led_setBrightness(BL);
            vTaskDelay(10/ portTICK_PERIOD_MS);
            tmp = 1;
        }

    }

    if(Time_BLight == BL_ON && tmp == 1){
        tmp =0;
        led_pwm_init(); 
        for(BL=BL_ON;BL<700;){
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

 
