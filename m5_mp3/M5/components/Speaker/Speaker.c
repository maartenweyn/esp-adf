#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
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
#include "esp_system.h"
#include "esp_log.h"
#include "tftspi.h"
#include "tft.h"
#include "wm8978.h"
#include "btn.h"
#include "Speaker.h"
#include "neopixel.h"

#include "BT_play.h"
#include "esp_peripherals.h"
#include "bluetooth_service.h"

// typedef struct _machine_neopixel_obj_t {
//     rmt_channel_t channel;
//     int gpio_num;
//     pixel_settings_t px;
// } neopixel_obj_t;

// neopixel_obj_t* nodeNeopixel;

// void ledSetAll(uint32_t color) {
//     color = color << 8;
//     for(uint8_t i = 0; i < 12; i++) {
//         np_set_pixel_color(&nodeNeopixel->px, i, color);
//     }
//     np_show(&nodeNeopixel->px, nodeNeopixel->channel);
// }

// static neopixel_obj_t* usr_neopixel_init(int gpio_num, uint16_t num, uint16_t rmtchan) {
//     neopixel_obj_t* neopixel_n = (neopixel_obj_t *)malloc(sizeof(neopixel_obj_t));

//     neopixel_n->px.timings.mark.level0 = 1;
// 	neopixel_n->px.timings.mark.level1 = 0;
// 	neopixel_n->px.timings.space.level0 = 1;
// 	neopixel_n->px.timings.space.level1 = 0;
// 	neopixel_n->px.timings.reset.level0 = 0;
// 	neopixel_n->px.timings.reset.level1 = 0;
// 	neopixel_n->px.timings.mark.duration0 = 12,

//     neopixel_n->px.nbits = 24;
//     neopixel_n->px.timings.mark.duration1 = 14;
//     neopixel_n->px.timings.space.duration0 = 7;
//     neopixel_n->px.timings.space.duration1 = 16;
//     neopixel_n->px.timings.reset.duration0 = 600;
//     neopixel_n->px.timings.reset.duration1 = 600;

//     neopixel_n->channel = rmtchan;
//     neopixel_n->gpio_num = gpio_num;
//     neopixel_n->px.pixel_count = num;
//     neopixel_n->px.pixels = (uint8_t *)malloc((neopixel_n->px.nbits/8) * neopixel_n->px.pixel_count);
//     neopixel_n->px.brightness = 50;
//     sprintf(neopixel_n->px.color_order, "GRBW");

//     if (neopixel_init(neopixel_n->gpio_num, rmtchan) != ESP_OK) {
//         printf("neopixel init error\r\n");
//         return ;
//     } 

//     printf("neopixel init ok...\r\n");

//     np_clear(&neopixel_n->px);
//     np_show(&neopixel_n->px, neopixel_n->channel);
//     return neopixel_n;
// }


static const char *SPEAK_TAG = "Speak";
static const char *EVENT_TAG = "asr_event";

typedef enum {
    WAKE_UP = 1,
    OPEN_THE_LIGHT,
    CLOSE_THE_LIGHT,
    VOLUME_INCREASE,
    VOLUME_DOWN,
    PLAY,
    PAUSE,
    MUTE,
    PLAY_LOCAL_MUSIC,
} asr_event_t;

static void speakReg(void *p){

    gpio_config_t io_conf;

    io_conf.pin_bit_mask = 1UL << IR_IN;   
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);


    io_conf.pin_bit_mask = 1UL << IR_OUT; 
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
    gpio_set_level(IR_OUT, 1);


    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(SPEAK_TAG, ESP_LOG_INFO);
    esp_log_level_set(EVENT_TAG, ESP_LOG_INFO);

    ESP_LOGI(SPEAK_TAG, "Initialize SR handle");
#if CONFIG_SR_MODEL_WN4_QUANT
    const esp_sr_iface_t *model = &esp_sr_wakenet4_quantized;
#else
    const esp_sr_iface_t *model = &esp_sr_wakenet3_quantized;
#endif
    model_iface_data_t *iface = model->create(DET_MODE_95);
    int num = model->get_word_num(iface);
    for (int i = 1; i <= num; i++) {
        char *name = model->get_word_name(iface, i);
        ESP_LOGI(SPEAK_TAG, "keywords: %s (index = %d)", name, i);
    }
    float threshold = model->get_det_threshold_by_mode(iface, DET_MODE_95, 1);
    int sample_rate = model->get_samp_rate(iface);
    int audio_chunksize = model->get_samp_chunksize(iface);
    ESP_LOGI(EVENT_TAG, "keywords_num = %d, threshold = %f, sample_rate = %d, chunksize = %d, sizeof_uint16 = %d", num, threshold, sample_rate, audio_chunksize, sizeof(int16_t));
    // int16_t *buff = (int16_t *)malloc(audio_chunksize * sizeof(short));
    int16_t buff[2048]={0};
    if (NULL == buff) {
        ESP_LOGE(EVENT_TAG, "Memory allocation failed!");
        model->destroy(iface);
        model = NULL;
        return;
    }

    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_reader, filter, raw_read;

    ESP_LOGI(EVENT_TAG, "[ 2.0 ] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(EVENT_TAG, "[ 2.1 ] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.2 ] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 48000;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = 16000;
    rsp_cfg.dest_ch = 1;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(EVENT_TAG, "[ 2.3 ] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(EVENT_TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    ESP_LOGI(EVENT_TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[SR]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);

    ESP_LOGI(EVENT_TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    while (1) {
                raw_stream_read(raw_read, (char *)buff, audio_chunksize * sizeof(short));
                int keyword =0;
                keyword = model->detect(iface, (int16_t *)buff);
            switch (keyword) {
                case WAKE_UP:
                    ESP_LOGI(SPEAK_TAG, "Wake up");
                    break;
                case OPEN_THE_LIGHT:
                    ESP_LOGI(SPEAK_TAG, "Turn on the light");
                    break;
                case CLOSE_THE_LIGHT:
                    ESP_LOGI(SPEAK_TAG, "Turn off the light");
                    break;
                case VOLUME_INCREASE:
                    ESP_LOGI(SPEAK_TAG, "volume increase");
                        volume += 13;
                        volume_increase(&volume);
                        printf("%s volume:%d\n",SPEAK_TAG,volume);
                    break;
                case VOLUME_DOWN:
                    
                    volume -= 13;
                    volume_down(&volume);
                    printf("%s volume:%d\n",SPEAK_TAG,volume);
                    break;
                case PLAY:
                    ESP_LOGI(SPEAK_TAG, "play");
                    periph_bluetooth_play(bt_periph);
                    break;
                case PAUSE:
                    ESP_LOGI(SPEAK_TAG, "pause");
                    periph_bluetooth_pause(bt_periph);
                    break;
                case MUTE:
                    ESP_LOGI(SPEAK_TAG, "mute");
                    volume = 1;
                    WM8978_SPKvol_Set(volume);
                    break;
                case PLAY_LOCAL_MUSIC:
                    ESP_LOGI(SPEAK_TAG, "play local music");
                    break;
                default:
                    ESP_LOGD(SPEAK_TAG, "Not supported keyword");
                    break;
            }
    }
}

TaskHandle_t xSP_TaskHandle = NULL;
void speaker_tast_create(void){
    xTaskCreatePinnedToCore(speakReg,  "speakReg_task", 7 * 1024, NULL,5, &xSP_TaskHandle,0);
}