// #include "local_mp3.h"
#include "ui.h"
#include "DHT12.h"
#include "btn.h"
#include "BT_play.h"
#include "Speaker.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.h"
#include "SDMP3.h"
#include "dht12.h"

void TaskSelect(void){

    for(;;){
        uxBits = xEventGroupWaitBits(xEventGroup,BIT_1_SD|BIT_2_BT|BIT_3_SP,pdFALSE,pdTRUE,NULL );          
        if( (uxBits & BIT_1_SD)  ==  BIT_1_SD){
            SD_task_create();
            dht12_task_create();
            break;
        }else if((uxBits & BIT_2_BT)  ==  BIT_2_BT){
            BT_player_task_create();
            dht12_task_create();
            break;
        }else if((uxBits & BIT_3_SP)  ==  BIT_3_SP)
        {
            speaker_tast_create();
            dht12_init();
            dht12_task_create();
            break;
        }
    }
}

void app_main(void)
{  
    /*Display temperature and humidity,bluetooth connection status
      mp3 play and pause,volume level*/
    UI_Task_Create();


    bt_link_event_status();

    //Keyscan task,Control music playback and adjust the volume
    btn_tast_create();

    TaskSelect();

    //Create bluetooth audio pipeline
    // BT_player_task_create();

    //Temperature and humidity collection Task
    // dht12_task_create(); 

    // SD_task_create();
    //speaker_tast_create();
    //UIGIF_Task_Create();
}






