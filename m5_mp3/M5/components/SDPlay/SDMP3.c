/* Control with a touch pad playing MP3 files from SD Card

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"
#include "ui.h"
#include "btn.h"
#include "BT_play.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <errno.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ff.h"

extern esp_err_t sdcard_unmount(void);
static const char *TAG = "SDCARD_MP3_CONTROL_EXAMPLE";


DIR* dir;
char name[30]={0};
char ** mp3_file =  NULL;

uint8_t Mp3FileCount = 0; 
#define MP3_COUNT_MAX  100

#define CURRENT 0
#define NEXT    1

audio_pipeline_handle_t SD_pipeline;
audio_element_handle_t  mp3_decoder;
esp_periph_set_handle_t set;
audio_element_handle_t rsp_handle;
audio_event_iface_handle_t evt;
esp_periph_handle_t sdcard_handle;
TaskHandle_t xSD_TaskHandle = NULL;
audio_element_handle_t i2s_stream_writer_sd;

FILE *get_file(int next_file)
{
    static FILE *file;  
    static int file_index = 0;

    if (next_file != CURRENT) {
        // advance to the next file
        if (++file_index > Mp3FileCount - 1) {
            file_index = 0;
        }
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
        ESP_LOGI(TAG, "[ * ] File index %d", file_index);
    }
    // return a handle to the current file
    if (file == NULL) {
        file = fopen(mp3_file[file_index], "r");
        if (!file) {
            ESP_LOGE(TAG, "Error opening file");
            return NULL;
        }
    }
    return file;
}

void scan_mp3_file(void)
{

    static uint8_t i =0 ; 
    mp3_file = (char **)malloc(sizeof(char *) * MP3_COUNT_MAX);

    if (NULL == mp3_file) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        return;
    }

    dir = opendir("/sdcard");
    for(;;){
        struct dirent* de = readdir(dir);       
        if (!de) {
            break;
        }

        int *p = NULL ;
        p = strstr(de->d_name,".MP3");

        if(p != NULL){
            sprintf(name,"/sdcard/%s",de->d_name);
                mp3_file[i] = (char*)malloc(strlen(name)+1);

                if(NULL==mp3_file[i]){
                    ESP_LOGE(TAG, " Memory allocation failed!");
                    return;
                }

                strcpy(mp3_file[i],name);
                memset(name,0,30);

                p = NULL;

                if(i>=MP3_COUNT_MAX){
                    ESP_LOGE(TAG, "The song is full Memory allocation failed!");
                    return;
                }
                Mp3FileCount++;
                i++;
        }
    }

    printf("MP3 file count :%d \n",Mp3FileCount);
    closedir(dir);
}

/*
 * Callback function to feed audio data stream from sdcard to mp3 decoder element
 */
static int my_sdcard_read_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_len = fread(buf, 1, len, get_file(CURRENT));
    if (read_len == 0) {
        read_len = AEL_IO_DONE;
    }
    return read_len;
}


void SDPipeStop(void){

    // audio_pipeline_terminate(SD_pipeline);

    // audio_pipeline_unregister(SD_pipeline, mp3_decoder);
    // audio_pipeline_unregister(SD_pipeline, i2s_stream_writer_sd);
    // audio_pipeline_unregister(SD_pipeline, rsp_handle);

    // /* Terminate the pipeline before removing the listener */
    // audio_pipeline_remove_listener(SD_pipeline);

    // /* Stop all peripherals before removing the listener */
    // esp_periph_set_stop_all(set);
    // audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    // /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    // audio_event_iface_destroy(evt);

    // /* Release all resources */
    // audio_pipeline_deinit(SD_pipeline);
    // audio_element_deinit(i2s_stream_writer_sd);
    // audio_element_deinit(mp3_decoder);
    // audio_element_deinit(rsp_handle);
    // esp_periph_set_destroy(set);

    // vTaskSuspend(xSD_TaskHandle);
    vTaskDelete(NULL);
    // ESP_LOGI(TAG, "[4.0] Create audio SD_pipeline for playback");
    // audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    // SD_pipeline = audio_pipeline_init(&pipeline_cfg);
    // mem_assert(SD_pipeline);

    // ESP_LOGI(TAG, "[4.1] Create i2s stream to write data to codec chip");
    // i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    // i2s_cfg.i2s_config.sample_rate = 48000;
    // i2s_cfg.type = AUDIO_STREAM_WRITER;
    // i2s_stream_writer_sd = i2s_stream_init(&i2s_cfg);

    // ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    // mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    // mp3_decoder = mp3_decoder_init(&mp3_cfg);
    // audio_element_set_read_cb(mp3_decoder, my_sdcard_read_cb, NULL);

    // ESP_LOGI(TAG, "[4.3] Create resample filter");
    // rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    // rsp_handle = rsp_filter_init(&rsp_cfg);

    // ESP_LOGI(TAG, "[4.4] Register all elements to audio SD_pipeline");
    // audio_pipeline_register(SD_pipeline, mp3_decoder, "mp3");
    // audio_pipeline_register(SD_pipeline, i2s_stream_writer_sd, "i2s");
    // audio_pipeline_register(SD_pipeline, rsp_handle, "filter");

    // ESP_LOGI(TAG, "[4.5] Link it together [my_sdcard_read_cb]-->mp3_decoder-->i2s_stream-->[codec_chip]");
    // audio_pipeline_link(SD_pipeline, (const char *[]) {"mp3", "filter", "i2s"}, 3);

    // ESP_LOGI(TAG, "[5.0] Set up  event listener");
    // audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    // evt = audio_event_iface_init(&evt_cfg);

    // ESP_LOGI(TAG, "[5.1] Listen for all SD_pipeline events");
    // audio_pipeline_set_listener(SD_pipeline, evt);

    // audio_pipeline_run(SD_pipeline);
}


void SD_Play(void *arg)
{
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "[4.0] Create audio SD_pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    SD_pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(SD_pipeline);

    ESP_LOGI(TAG, "[4.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer_sd = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);
    audio_element_set_read_cb(mp3_decoder, my_sdcard_read_cb, NULL);

    ESP_LOGI(TAG, "[4.3] Create resample filter");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_handle = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(TAG, "[4.4] Register all elements to audio SD_pipeline");
    audio_pipeline_register(SD_pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(SD_pipeline, i2s_stream_writer_sd, "i2s");
    audio_pipeline_register(SD_pipeline, rsp_handle, "filter");

    ESP_LOGI(TAG, "[4.5] Link it together [my_sdcard_read_cb]-->mp3_decoder-->i2s_stream-->[codec_chip]");
    audio_pipeline_link(SD_pipeline, (const char *[]) {"mp3", "filter", "i2s"}, 3);

    ESP_LOGI(TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.1] Listen for all SD_pipeline events");
    audio_pipeline_set_listener(SD_pipeline, evt);

    while (1) {
        
        audio_event_iface_msg_t msg;

        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) {
            // Set music info for a new song to be played
            if (msg.source == (void *) mp3_decoder
                && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(mp3_decoder, &music_info);
                ESP_LOGI(TAG, "[ * ] Received music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
                         music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer_sd, &music_info);
                rsp_filter_set_src_info(rsp_handle, music_info.sample_rates, music_info.channels);
                continue;
            }
            // Advance to the next song when previous finishes
            if (msg.source == (void *) i2s_stream_writer_sd
                && msg.cmd == AEL_MSG_CMD_REPORT_STATUS) {
                audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer_sd);
                if (el_state == AEL_STATE_FINISHED) {
                    ESP_LOGI(TAG, "[ * ] Finished, advancing to the next song");
                    audio_pipeline_stop(SD_pipeline);
                    audio_pipeline_wait_for_stop(SD_pipeline);
                    get_file(NEXT);
                    audio_pipeline_run(SD_pipeline);
                }
                continue;
            }
        }
        vTaskDelay(1/ portTICK_RATE_MS);
    }

}


void SD_task_create(void){
         scan_mp3_file();
         xTaskCreatePinnedToCore(SD_Play,  "SD_Play",    5 * 512, NULL, 2, &xSD_TaskHandle,tskNO_AFFINITY);   
}