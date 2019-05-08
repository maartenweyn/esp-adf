#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "audio_mem.h"
#include "mp3_decoder.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "filter_resample.h"
#include "esp_sr_iface.h"
#include "esp_sr_models.h"
#include "tftspi.h"
#include "tft.h"
#include "neopixel.h"
#include "dht12.h"
#include "nvs_flash.h"
#include "esp_peripherals.h"
#include "bluetooth_service.h"
#include "wm8978.h"
#include "btn.h"
#include "DHT12.h"
#include "BT_play.h"

static const char *BT_TAG = "BT_PLAY";  
audio_pipeline_handle_t BT_pipeline;
esp_periph_handle_t bt_periph;
extern EventBits_t uxBits = 0;
uint8_t volume=63;

EventGroupHandle_t xEventGroup = NULL;
void bt_link_event_status(void)
{
    xEventGroup =xEventGroupCreate();
    if( xEventGroup == NULL ){
  		ESP_LOGI(BT_TAG, "Event group creation failed");
  	}
  	else{
 		ESP_LOGI(BT_TAG, "[ 0 ]The event group was created successfully");
  	}
}

static void bt_play_mp3(void *arg){

    audio_element_handle_t bt_stream_reader, i2s_stream_writer;
    audio_event_iface_handle_t evt;
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(BT_TAG, ESP_LOG_DEBUG);

    ESP_LOGI(BT_TAG, "[ 1 ] Create Bluetooth service");
    bluetooth_service_cfg_t bt_cfg = {
        .device_name = "M5-PLAYER-ONE",
        .mode = BLUETOOTH_A2DP_SINK,
    };
    bluetooth_service_start(&bt_cfg);

    ESP_LOGI(BT_TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(BT_TAG, "[ 3 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    BT_pipeline = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(BT_TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(BT_TAG, "[3.2] Get Bluetooth stream");
    bt_stream_reader = bluetooth_service_create_stream();

    ESP_LOGI(BT_TAG, "[3.2] Register all elements to audio pipeline");
    audio_pipeline_register(BT_pipeline, bt_stream_reader, "bt");
    audio_pipeline_register(BT_pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(BT_TAG, "[3.3] Link it together [Bluetooth]-->bt_stream_reader-->i2s_stream_writer-->[codec_chip]");
    audio_pipeline_link(BT_pipeline, (const char *[]) {"bt", "i2s"}, 2);

    ESP_LOGI(BT_TAG, "[ 4 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PHERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(BT_TAG, "[4.2] Create Bluetooth peripheral");
    bt_periph = bluetooth_service_create_periph();

    ESP_LOGI(BT_TAG, "[4.2] Start all peripherals");
    esp_periph_start(set, bt_periph);

    ESP_LOGI(BT_TAG, "[ 5 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(BT_TAG, "[5.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(BT_pipeline, evt);

    ESP_LOGI(BT_TAG, "[5.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(BT_TAG, "[ 6 ] Start audio_pipeline");
    audio_pipeline_run(BT_pipeline);
    ESP_LOGI(BT_TAG, "[ 7 ] Listen for all pipeline events");
    while(1){
            audio_event_iface_msg_t msg;
            esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
            if (ret != ESP_OK) {
                ESP_LOGE(BT_TAG, "[ * ] Event interface error : %d", ret);
                continue;
            }
            
            if (msg.cmd == AEL_MSG_CMD_ERROR) {
                ESP_LOGE(BT_TAG, "[ * ] Action command error: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                        msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
            }

            if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) bt_stream_reader
                && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
                xEventGroupSetBits(xEventGroup,BIT_0);
                audio_element_info_t music_info = {0};
                audio_element_getinfo(bt_stream_reader, &music_info);

                ESP_LOGI(BT_TAG, "[ * ] Receive music info from Bluetooth, sample_rates=%d, bits=%d, ch=%d",
                        music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer, &music_info);
                i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
                continue;
            }

            /* Stop when the Bluetooth is disconnected or suspended */
            if (msg.source_type == PERIPH_ID_BLUETOOTH
                && msg.source == (void *)bt_periph) {
                if (msg.cmd == PERIPH_BLUETOOTH_DISCONNECTED) {
                    xEventGroupClearBits(xEventGroup,BIT_0);
                    ESP_LOGW(BT_TAG, "[ * ] Bluetooth disconnected");
                }
            }
            /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
            if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
                && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int) msg.data == AEL_STATUS_STATE_STOPPED) {
                ESP_LOGW(BT_TAG, "[ * ] Stop event received");
                break;
            }
            vTaskDelay(10/ portTICK_RATE_MS);
    }
}

TaskHandle_t xBT_TaskHandle = NULL;
void BT_player_task_create(void){
    xTaskCreatePinnedToCore(bt_play_mp3,"bt_play_Task",3 * 1024, NULL,1, &xBT_TaskHandle,tskNO_AFFINITY);
}

void  volume_increase(uint8_t *vol){
    if(*vol ==1){
        *vol = 0;
    }

    *vol += 13;
    if(*vol<1||*vol>63){
        *vol=63;
    }
   
    WM8978_SPKvol_Set(*vol);
    vTaskDelay(20 / portTICK_RATE_MS);
}

void volume_down(uint8_t *vol){
    *vol -= 13;
    if(*vol<1||*vol>63){
        *vol=1;
    }
    WM8978_SPKvol_Set(*vol);
    vTaskDelay(20 / portTICK_RATE_MS);
}
