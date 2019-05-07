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


extern void SD_task_create(void);
void app_main(void)
{  
    /*Display temperature and humidity,bluetooth connection status
      mp3 play and pause,volume level*/
    UI_Task_Create();

    bt_link_event_status();

    //Create bluetooth audio pipeline
    // BT_player_task_create();

    //Keyscan task,Control music playback and adjust the volume
    btn_tast_create();

    //Temperature and humidity collection Task
    // dht12_task_create(); 

    SD_task_create();
    //speaker_tast_create();
    //UIGIF_Task_Create();
}






