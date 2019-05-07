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



void app_main(void)
{  
    /*Display temperature and humidity,bluetooth connection status
      mp3 play and pause,volume level*/
    lcdInit();
    UI_Task_Create();

    bt_link_event_status();

    // //Keyscan task,Control music playback and adjust the volume
    btn_tast_create();

    while(1)
    {

        BtKeyScan();     //Bluetooth keyscan
        SDKeyScan();     //SD Card key scan
        SelectMode();
        TaskSelect();

        if(sel_mode==1){
          // printf("SD\n");
        }
        else if (sel_mode==2)
        {
          // printf("BT\n");
        }else if(sel_mode==3)
        {
          // printf("SP\n");
        }

       
      
    }
}






