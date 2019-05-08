#pragma once
#include "freertos/FreeRTOS.h"
#define BTN_A   39
#define BTN_B   38
#define BTN_C   37
#define BTN  ((1ULL<<BTN_A) | (1ULL<<BTN_B) | (1ULL<<BTN_C))

void BtnIOInit(void);
extern uint8_t  play_pause;
extern uint8_t key_vaule;
extern int8_t  sel_mode;
extern uint8_t mode;

void SelectMode(void);
void SDKeyScan(void);
void BtKeyScan(void);
void TaskSelect(void);