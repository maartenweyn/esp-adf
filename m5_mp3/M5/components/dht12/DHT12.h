#pragma once

void dht12_task_create(void);
extern char dataHum[20];
extern char dataTmp[20];
extern uint8_t DisHumTum;

#define DISPLAY_T_H 1
#define UNDISPLAY_T_H 0