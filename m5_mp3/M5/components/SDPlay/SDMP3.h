#pragma once
void SD_task_create(void);
extern TaskHandle_t xSD_TaskHandle;
extern audio_element_handle_t  mp3_decoder;
extern audio_element_handle_t rsp_handle;
void SDPipeStop(void);
void sdPlayInit(void);
void scan_mp3_file(void);
extern esp_periph_handle_t sdcard_handle;