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

int8_t  sel_mode=1;
uint8_t KeyMode = M_SEL;
uint8_t NextTmp=0;

extern uint8_t LightTurn;
#define ON 0
#define OFF 1

void BtnIOInit(void){

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = BTN;     // Pin37 Pin38 Pin39 initialization
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);        
}

#define BL_ON 0
#define BL_OFF 40
extern uint32_t Time_BLight;


void Light_Neop_On_Off(void){
    if(LightTurn == ON){
        LightTurn = OFF;
        Time_BLight=BL_OFF;
        printf("off\n");
    }else{
            LightTurn = ON;
            Time_BLight=BL_ON;
            printf("on\n");
    }
}

// void CPU_RESET(void)
// {
//     char *p = NULL;
//     *P = 1;    

// }


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
    if(KeyMode == M_BT){
         uxBits = xEventGroupWaitBits(xEventGroup,BIT_0,pdFALSE,pdTRUE,NULL );          //Bluetooth connection wait bit

        if( (uxBits & BIT_0)  ==  BIT_0){
            bt_status = LINK;

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
                    if(WFlag==0){
                    WFlag=1;
                    Wtmp = 0;
                    }else{
                        WFlag=0;
                        DisB=0;
                    }
                    break;

                case KEY_B_EVENT_LONG:
                    esp_restart();
                break;

                    case  KEY_B_EVENT_THREE:
                         Light_Neop_On_Off();
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
                if(WFlag==0){
                WFlag=1;
                Wtmp = 0;
                }else{
                    WFlag=0;
                    DisB=0;
                }
                break;

                case  KEY_B_EVENT_THREE:
                         Light_Neop_On_Off();
                    break;

                case KEY_B_EVENT_LONG:
                    esp_restart();
                break;


                case KEY_C_EVENT_SHORT:
                        volume_increase(&volume);
                    break;

                default:
                    break;
            }
                bt_status = UNLINK;
        }
    }
}


void SDKeyScan(void){
  if(KeyMode == M_SD){
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
                volume=50;
                volume_down(&volume);
                get_file(NEXT);
                audio_pipeline_run(SD_pipeline);
                play_pause = PLAY;
               }
               else if(play_pause ==PLAY){  
                    NextTmp = 1;
                    audio_pipeline_pause(SD_pipeline);
                    play_pause = PAUSE;
                    printf("pause\n");
                    NextTmp = 0;
                }else{
                    audio_pipeline_resume(SD_pipeline);
                    play_pause = PLAY;
                     printf("play\n");
                }
            break;  

        case KEY_B_EVENT_LONG:
               esp_restart();
            break;

        case KEY_B_EVENT_DOUBLE:
            if(WFlag==0){
                WFlag=1;
                Wtmp = 0;
            }else{
                WFlag=0;
                DisB=0;
            }
            
            break;

        case  KEY_B_EVENT_THREE:
            Light_Neop_On_Off();
        break;

        case KEY_C_EVENT_SHORT:
                volume_increase(&volume);
                printf("volume:%d\n",volume);
            break;

        case KEY_C_EVENT_LONG:
                
            break;

        case KEY_C_EVENT_DOUBLE:
        
                audio_pipeline_resume(SD_pipeline);
                audio_pipeline_terminate(SD_pipeline);
                get_file(NEXT);
                printf( "next song\n");
                audio_pipeline_run(SD_pipeline);
                play_pause = PAUSE;
                play_pause = PLAY;
            break;

        default:

            break;
    }



    
  }
}

void SPKeyScan(void){
  if(KeyMode == M_SPEAK){
    switch (KeyRead())
    {

        case KEY_A_EVENT_SHORT:
                volume_down(&volume);
                printf("volume:%d\n",volume);
        break;

        case KEY_B_EVENT_LONG:
              esp_restart();
            break;

        case KEY_B_EVENT_DOUBLE:
            if(WFlag==0){
                WFlag=1;
                Wtmp = 0;
            }else{
                WFlag=0;
                DisB=0;
            }
            break;

        case  KEY_B_EVENT_THREE:
            Light_Neop_On_Off();
        break;

        case KEY_C_EVENT_SHORT:
                volume_increase(&volume);
                printf("volume:%d\n",volume);
        break;



        default:

            break;
    }
  }
}


void selC(void){

    if(Stmp == 2)
    {
        if(sel_mode==1){
            xEventGroupSetBits(xEventGroup,BIT_1_SD);
            KeyMode=M_SD;
            play_pause = RUN;
            printf("BIT_1_SD\n");
            
        }
        else if (sel_mode==2)
        {
            xEventGroupSetBits(xEventGroup,BIT_2_BT);
            KeyMode=M_BT;
            printf("BIT_2_BT\n");
            
        }else if(sel_mode==3)
        {
            xEventGroupSetBits(xEventGroup,BIT_3_SP);
            KeyMode = M_SPEAK;
        }
        Stmp = 3;
    }
}

uint8_t Stmp = 0;
void SelectMode(void){

    if(KeyMode==0){
        switch (KeyRead())
        {
            case KEY_A_EVENT_SHORT:
                    sel_mode--;
                    if(sel_mode<=0){
                        sel_mode=1;
                    }
                    printf("sel_mode:%d\n",sel_mode);
                break;

            case KEY_B_EVENT_SHORT:
               
                if(Stmp==1)
                {
                    Stmp =2;
                }else {
                    Stmp = 1;
                }
               printf("Stmp:%d\n",Stmp);
                break;

            case  KEY_B_EVENT_THREE:
                 Light_Neop_On_Off();
            break;

            case KEY_B_EVENT_LONG:
                    esp_restart();
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

        selC();

    }
}


void TaskSelect(void){

    uxBits = xEventGroupWaitBits(xEventGroup,BIT_1_SD|BIT_2_BT|BIT_3_SP,pdFALSE,pdTRUE,NULL );          
    if( (uxBits & BIT_1_SD)  ==  BIT_1_SD){
        SD_task_create();
        dht12_task_create();
        xEventGroupClearBits(xEventGroup,BIT_1_SD);
    }else if((uxBits & BIT_2_BT)  ==  BIT_2_BT){
        BT_player_task_create();
        dht12_task_create();
        xEventGroupClearBits(xEventGroup,BIT_2_BT);
    }else if((uxBits & BIT_3_SP)  ==  BIT_3_SP)
    {
        speaker_tast_create();
        dht12_task_create();
        xEventGroupClearBits(xEventGroup,BIT_3_SP);
    }
 
}
