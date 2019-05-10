#pragma once
#include "freertos/FreeRTOS.h"
#define BTN_A   39
#define BTN_B   38
#define BTN_C   37
#define BTN  ((1ULL<<BTN_A) | (1ULL<<BTN_B) | (1ULL<<BTN_C))

#define M_SEL 0
#define M_SD  1
#define M_BT  2
#define M_SPEAK  3

extern uint8_t KeyMode;
extern uint8_t Stmp;
void BtnIOInit(void);
extern uint8_t  play_pause;
extern uint8_t key_vaule;
extern int8_t  sel_mode;
extern uint8_t mode;

void SelectMode(void);
void SDKeyScan(void);
void BtKeyScan(void);
void SPKeyScan(void);
void TaskSelect(void);
