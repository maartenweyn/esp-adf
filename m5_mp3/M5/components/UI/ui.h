#pragma once
#include "freertos/FreeRTOS.h"

extern  uint8_t master_jpg_start[] asm("_binary_master_jpg_start");
extern  uint8_t master_jpg_end[]   asm("_binary_master_jpg_end");

extern  uint8_t master_bmp_start[] asm("_binary_master_bmp_start");
extern  uint8_t master_bmp_end[]   asm("_binary_master_bmp_end");

extern  uint8_t A1_jpg_start[] asm("_binary_A1_jpg_start");
extern  uint8_t A1_jpg_end[]   asm("_binary_A1_jpg_end");

extern  uint8_t A2_jpg_start[] asm("_binary_A2_jpg_start");
extern  uint8_t A2_jpg_end[]   asm("_binary_A2_jpg_end");

extern  uint8_t B1_jpg_start[] asm("_binary_B1_jpg_start");
extern  uint8_t B1_jpg_end[]   asm("_binary_B1_jpg_end");

extern  uint8_t B2_jpg_start[] asm("_binary_B2_jpg_start");
extern  uint8_t B2_jpg_end[]   asm("_binary_B2_jpg_end");

extern  uint8_t C1_jpg_start[] asm("_binary_C1_jpg_start");
extern  uint8_t C1_jpg_end[]   asm("_binary_C1_jpg_end");

extern  uint8_t C2_jpg_start[] asm("_binary_C2_jpg_start");
extern  uint8_t C2_jpg_end[]   asm("_binary_C2_jpg_end");

extern  uint8_t play_jpg_start[] asm("_binary_play_jpg_start");
extern  uint8_t play_jpg_end[]   asm("_binary_play_jpg_end");

extern  uint8_t pause_jpg_start[] asm("_binary_pause_jpg_start");
extern  uint8_t pause_jpg_end[]   asm("_binary_pause_jpg_end");

extern  uint8_t prev_jpg_start[] asm("_binary_prev_jpg_start");
extern  uint8_t prev_jpg_end[]   asm("_binary_prev_jpg_end");

extern  uint8_t next_jpg_start[] asm("_binary_next_jpg_start");
extern  uint8_t next_jpg_end[]   asm("_binary_next_jpg_end");

extern  uint8_t prev1_jpg_start[] asm("_binary_prev1_jpg_start");
extern  uint8_t prev1_jpg_end[]   asm("_binary_prev1_jpg_end");

extern  uint8_t next1_jpg_start[] asm("_binary_next1_jpg_start");
extern  uint8_t next1_jpg_end[]   asm("_binary_next1_jpg_end");

// extern  uint8_t next1_bmp_start[] asm("_binary_next1_bmp_start");
// extern  uint8_t next1_bmp_end[]   asm("_binary_next1_bmp_end");

extern  uint8_t bt_linked_jpg_start[] asm("_binary_bt_linked_jpg_start");
extern  uint8_t bt_linked_jpg_end[]   asm("_binary_bt_linked_jpg_end");

extern  uint8_t bt_unlink_jpg_start[] asm("_binary_bt_unlink_jpg_start");
extern  uint8_t bt_unlink_jpg_end[]   asm("_binary_bt_unlink_jpg_end");

extern  uint8_t hum_jpg_start[] asm("_binary_hum_jpg_start");
extern  uint8_t hum_jpg_end[]   asm("_binary_hum_jpg_end");

extern  uint8_t tmp_jpg_start[] asm("_binary_tmp_jpg_start");
extern  uint8_t tmp_jpg_end[]   asm("_binary_tmp_jpg_end");

extern  uint8_t vol1_jpg_start[] asm("_binary_vol1_jpg_start");
extern  uint8_t vol1_jpg_end[]   asm("_binary_vol1_jpg_end");

extern  uint8_t vol2_jpg_start[] asm("_binary_vol2_jpg_start");
extern  uint8_t vol2_jpg_end[]   asm("_binary_vol2_jpg_end");

extern  uint8_t vol3_jpg_start[] asm("_binary_vol3_jpg_start");
extern  uint8_t vol3_jpg_end[]   asm("_binary_vol3_jpg_end");

extern  uint8_t vol4_jpg_start[] asm("_binary_vol4_jpg_start");
extern  uint8_t vol4_jpg_end[]   asm("_binary_vol4_jpg_end");

extern  uint8_t vol5_jpg_start[] asm("_binary_vol5_jpg_start");
extern  uint8_t vol5_jpg_end[]   asm("_binary_vol5_jpg_end");

extern  uint8_t vol6_jpg_start[] asm("_binary_vol6_jpg_start");
extern  uint8_t vol6_jpg_end[]   asm("_binary_vol6_jpg_end");

extern  uint8_t vol7_jpg_start[] asm("_binary_vol7_jpg_start");
extern  uint8_t vol7_jpg_end[]   asm("_binary_vol7_jpg_end");

extern  uint8_t vol8_jpg_start[] asm("_binary_vol8_jpg_start");
extern  uint8_t vol8_jpg_end[]   asm("_binary_vol8_jpg_end");

extern uint8_t bt_status;
extern uint8_t UiReady;

#define SPI_BUS TFT_HSPI_HOST
void UI_Task_Create(void);
void DisplayVolume(uint8_t vol);
void lcdInit(void) ;