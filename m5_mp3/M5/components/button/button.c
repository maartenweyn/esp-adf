#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "wm8978.h"
#include "btn.h"
#include "ui.h"

#include "BT_play.h"
#include "audio_pipeline.h"
#include "esp_peripherals.h"
#include "bluetooth_service.h"
#include "button.h"
#include "tftspi.h"
#include "tft.h"
#include "TIMER.h"

#define KEY_THRES_HOLD  500
#define KEY_THRES_DOUBLE  250

#define BL_ON 0
#define BL_OFF 40
extern uint32_t Time_BLight;
extern uint8_t NextTmp;

uint16_t KeyRead(void)
{
    uint16_t K = KEY_EVENT_NULL;
    if(!gpio_get_level(BTN_A)){
        vTaskDelay(10 / portTICK_RATE_MS);
        if(!gpio_get_level(BTN_A)){
            K = KEY_A_EVENT_SHORT;
        }

        uint32_t count = xTaskGetTickCount();
        while(!gpio_get_level(BTN_A)){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(xTaskGetTickCount() - count >= KEY_THRES_HOLD){
                K = KEY_A_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d) < KEY_THRES_DOUBLE){
            vTaskDelay(1/portTICK_RATE_MS);
            if(!gpio_get_level(BTN_A)){
            vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_A)){
                    K = KEY_A_EVENT_DOUBLE;  
                }
            }
        }

        if(OFF_Flag == 1){
            OFF_Flag = 0;
            Time_BLight = BL_ON;
            K = KEY_EVENT_NULL; 
        } 
        if(Stmp==4){ Stmp=3;}

         if(WFlag==1){
            K = KEY_B_EVENT_DOUBLE;
         }
         
    }


    if(!gpio_get_level(BTN_B)){
        vTaskDelay(10 / portTICK_RATE_MS);
        if(!gpio_get_level(BTN_B)){
            K = KEY_B_EVENT_SHORT;
        }

        uint32_t count = xTaskGetTickCount();
        while(!gpio_get_level(BTN_B)){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(xTaskGetTickCount() - count >= KEY_THRES_HOLD){
                K = KEY_B_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d) < KEY_THRES_DOUBLE){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(!gpio_get_level(BTN_B)){
            vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_B)){
                    K = KEY_B_EVENT_DOUBLE;  
                }
            }
        }

        uint32_t count_dd = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_dd) < KEY_THRES_DOUBLE){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(!gpio_get_level(BTN_B)){
            vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_B)){
                    K = KEY_B_EVENT_THREE;  
                }
            }
        }

        if(OFF_Flag == 1){
            OFF_Flag = 0;
            Time_BLight = BL_ON;
             K = KEY_EVENT_NULL; 
        }
        if(Stmp==4){ Stmp=3;}

        if(WFlag==1){
            K = KEY_B_EVENT_DOUBLE;
         }
    }



    if(!gpio_get_level(BTN_C)){
        vTaskDelay(10 / portTICK_RATE_MS);
        if(!gpio_get_level(BTN_C)){
            K = KEY_C_EVENT_SHORT;
        }

        uint32_t count = xTaskGetTickCount();
        while(!gpio_get_level(BTN_C)){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(xTaskGetTickCount() - count >= KEY_THRES_HOLD){
                K = KEY_C_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d) < KEY_THRES_DOUBLE){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(!gpio_get_level(BTN_C)){
                vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_C)){
                    K = KEY_C_EVENT_DOUBLE; 
                    if(NextTmp==1){
                        K = KEY_EVENT_NULL;
                    } 
                }
            }
        }

       if(OFF_Flag == 1){
            OFF_Flag = 0;
            Time_BLight = BL_ON;
             K = KEY_EVENT_NULL; 
        }
        
        if(Stmp==4){ Stmp=3;}

        if(WFlag==1){
            K = KEY_B_EVENT_DOUBLE;
         }

         
    }


    return K;
}

 
