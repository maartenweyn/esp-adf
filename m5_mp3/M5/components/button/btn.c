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
#include "SDMP3.h"
#include "tftspi.h"
#include "tft.h"
#include "dht12.h"
#include "Speaker.h"
#include "DHT12.h"

#define KEY_THRESHOLD  500
#define CLEAR 0X00
#define PREV 0X01
#define NEXT 0X02
#define PLAY_STATUS 0X04

#define PLAY 0X01
#define PAUSE 0X02 
#define RUN   0X03 

#define SD_NEXT 0X01

uint8_t  play_pause = PAUSE;
audio_pipeline_handle_t SD_pipeline;
FILE *get_file(int next_file);

int8_t  sel_mode=0;
uint8_t mode=0;


void BtnIOInit(void){

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = BTN;     // Pin37 Pin38 Pin39 initialization
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);        
}


/*buttons deal with
* buttons types:         A                B               C
*
*  Short press :    volume down       play/pause       volume up
*               
* double press :    prev                               next
*
*  Long  press :    rewind                             forward
*
*/

void BtKeyScan(void){
    if(mode==2){
         uxBits = xEventGroupWaitBits(xEventGroup,BIT_0,pdFALSE,pdTRUE,NULL );          //Bluetooth connection wait bit

        if( (uxBits & BIT_0)  ==  BIT_0){
            bt_status = LINK_BT;

            switch (KeyRead())              //key scan
            {
                case KEY_A_EVENT_SHORT:
                        volume_down(&volume);
                    break;

                case KEY_A_EVENT_DOUBLE:
                        periph_bluetooth_prev(bt_periph);
                        play_pause = PLAY;
                    break;

                case KEY_A_EVENT_LONG:
                        periph_bluetooth_rewind(bt_periph);
                    break;
              
                case KEY_B_EVENT_SHORT:
                        if(play_pause ==PAUSE){
                            periph_bluetooth_play(bt_periph);
                            play_pause = PLAY;
                        }else{
                            play_pause = PAUSE;
                            periph_bluetooth_pause(bt_periph);
                        }
                    break;

                case KEY_B_EVENT_DOUBLE:
                
                    break;

                case KEY_B_EVENT_LONG:
                    //    printf("long B\n");
                    //    periph_bluetooth_pause(bt_periph);
                    //    audio_pipeline_pause(BT_pipeline);
                    //    vTaskDelay(500);
                    //     mode=0;
                    break;

                case KEY_C_EVENT_SHORT:
                        volume_increase(&volume);
                    break;

               case KEY_C_EVENT_DOUBLE:
                        periph_bluetooth_next(bt_periph);
                        play_pause = PLAY;
                    break;

                case KEY_C_EVENT_LONG:
                        periph_bluetooth_fast_forward(bt_periph);
                    break;

                default:
                    break;
            }
        }else{

            switch (KeyRead())              //key scan
            {
                case KEY_A_EVENT_SHORT:
                        volume_down(&volume);
                    break;

                case KEY_B_EVENT_DOUBLE:
                
                    break;

                case KEY_B_EVENT_LONG:
                       
                    //    printf("long B\n");
                    //    mode=0;
                    break;


                case KEY_C_EVENT_SHORT:
                        volume_increase(&volume);
                    break;

                default:
                    break;
            }
                vTaskDelay(1/ portTICK_RATE_MS);
                bt_status = UNLINK_BT;
        }
    }
        vTaskDelay(1/ portTICK_RATE_MS);
}

void SDKeyScan(void){
  if(mode==1){
    switch (KeyRead())
    {
        case KEY_A_EVENT_SHORT:
                volume_down(&volume);
                printf("volume:%d\n",volume);
            break;

        case KEY_A_EVENT_LONG:
                
            break;

        case KEY_A_EVENT_DOUBLE:     
            
            break;

        case KEY_B_EVENT_SHORT:

               if(play_pause==RUN) {
                volume=40;
                volume_down(&volume);
                get_file(NEXT);
                audio_pipeline_run(SD_pipeline);
                play_pause = PLAY;
               }
               else if(play_pause ==PLAY){  
                    audio_pipeline_pause(SD_pipeline);
                    play_pause = PAUSE;
                    printf("pause\n");
                }else{
                    audio_pipeline_resume(SD_pipeline);
                    play_pause = PLAY;
                     printf("play\n");
                }
            break;  

        case KEY_B_EVENT_LONG:
                // audio_pipeline_resume(SD_pipeline);
                // audio_pipeline_terminate(SD_pipeline);
                // mode=0;
            break;

        case KEY_B_EVENT_DOUBLE:
                
            break;

        case KEY_C_EVENT_SHORT:
                volume_increase(&volume);
                printf("volume:%d\n",volume);
            break;

        case KEY_C_EVENT_LONG:
                
            break;

        case KEY_C_EVENT_DOUBLE:
                audio_pipeline_terminate(SD_pipeline);
                get_file(NEXT);
                printf( "next song\n");
                audio_pipeline_run(SD_pipeline);
                play_pause = PLAY;
            break;

        default:

            break;
    }
  }
    vTaskDelay(1/ portTICK_RATE_MS);
}



void SelectMode(void){

    if(mode==0){
        switch (KeyRead())
        {
            case KEY_A_EVENT_SHORT:
                    sel_mode--;
                    if(sel_mode<=0){
                        sel_mode=1;
                    }
                    printf("sel_mode:%d\n",sel_mode);
                break;

            case KEY_B_EVENT_LONG:
        
                if(sel_mode==1){
                    xEventGroupSetBits(xEventGroup,BIT_1_SD);
                    mode=1;
                    play_pause = RUN;
                    printf("BIT_1_SD\n");
                    
                }
                else if (sel_mode==2)
                {
                    xEventGroupSetBits(xEventGroup,BIT_2_BT);
                    mode=2;
                    printf("BIT_2_BT\n");
                    
                }else if(sel_mode==3)
                {
                    xEventGroupSetBits(xEventGroup,BIT_3_SP);
                    mode=3;
                }
                break;

            case KEY_C_EVENT_SHORT:
                sel_mode++;
                if(sel_mode>3){
                        sel_mode=3;
                    }
                    printf("sel_mode:%d\n",sel_mode);
                break;

            default:

                break;
        }
    }
    vTaskDelay(1/ portTICK_RATE_MS);
}






void TaskSelect(void){

 static uint8_t BTTaskSupend=0;
 static uint8_t SDTaskSupend=0;
 static uint8_t SPTaskSupend=0;

  uxBits = xEventGroupWaitBits(xEventGroup,BIT_1_SD|BIT_2_BT|BIT_3_SP,pdFALSE,pdTRUE,NULL );          
        if( (uxBits & BIT_1_SD)  ==  BIT_1_SD){
            if(SDTaskSupend==0){
                SDTaskSupend=1;
                SD_task_create();
                dht12_task_create();
            }else{
                SD_task_create();
                // vTaskResume(xSD_TaskHandle);
                // printf("resume SD\n");
            }
            xEventGroupClearBits(xEventGroup,BIT_1_SD);
        }else if((uxBits & BIT_2_BT)  ==  BIT_2_BT){
            if(BTTaskSupend==0){
                BTTaskSupend=1;
            BT_player_task_create();
            dht12_task_create();
            }else
            {
                BT_player_task_create();
                // vTaskResume(xBT_TaskHandle);
                // audio_pipeline_resume(BT_pipeline);
                printf("resume BT\n");
            }
            
            xEventGroupClearBits(xEventGroup,BIT_2_BT);
        }else if((uxBits & BIT_3_SP)  ==  BIT_3_SP)
        {
            speaker_tast_create();
            // dht12_init();
            dht12_task_create();
            xEventGroupClearBits(xEventGroup,BIT_3_SP);
        }
 
}
