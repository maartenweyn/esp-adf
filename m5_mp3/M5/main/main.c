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
#include "neopixelApp.h"

void app_main(void)
{  
    /*Display temperature and humidity,bluetooth connection status
      mp3 play and pause,volume level*/
    lcdInit();
    BtnIOInit();
    InitCommon();

    bt_link_event_status();
    UI_Task_Create();
    Neopixel_Task_Create();

    // //Keyscan task,Control music playback and adjust the volume
    while(1)
    {
        BtKeyScan();     //Bluetooth keyscan
        SDKeyScan();     //SD Card key scan
        SelectMode();
        TaskSelect();

        vTaskDelay(1/portTICK_RATE_MS);
    }
}






