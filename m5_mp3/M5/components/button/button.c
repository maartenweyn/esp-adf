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

#define KEY_THRESHOLD  500

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
            if(xTaskGetTickCount() - count >= KEY_THRESHOLD){
                K = KEY_A_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d)<200){
            vTaskDelay(1/portTICK_RATE_MS);
            if(!gpio_get_level(BTN_A)){
            vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_A)){
                    K = KEY_A_EVENT_DOUBLE;  
                }
            }
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
            if(xTaskGetTickCount() - count >= KEY_THRESHOLD){
                K = KEY_B_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d)<200){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(!gpio_get_level(BTN_B)){
            vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_B)){
                    K = KEY_B_EVENT_DOUBLE;  
                }
            }
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
            if(xTaskGetTickCount() - count >= KEY_THRESHOLD){
                K = KEY_C_EVENT_LONG;
            }
        }

        uint32_t count_d = xTaskGetTickCount();
        while((xTaskGetTickCount() - count_d)<200){
            vTaskDelay(1 / portTICK_RATE_MS);
            if(!gpio_get_level(BTN_C)){
                vTaskDelay(10 / portTICK_RATE_MS);
                if(!gpio_get_level(BTN_C)){
                    K = KEY_C_EVENT_DOUBLE;  
                }
            }
        }
    }

    return K;
}

 
