#pragma once

#include "audio_pipeline.h"
#include "esp_peripherals.h"
void BT_player_task_create(void);
extern audio_pipeline_handle_t BT_pipeline;
esp_periph_handle_t bt_periph;
extern uint8_t volume;
extern EventBits_t uxBits;

extern EventGroupHandle_t xEventGroup;
extern TaskHandle_t xBT_TaskHandle;

#define BIT_0	( 1 << 0 )
#define BIT_1	( 1 << 1 )
#define BIT_2	( 1 << 2 )

void volume_increase(uint8_t *vol);
void volume_down(uint8_t *vol);

#define LINK_BT  1
#define UNLINK_BT 0
