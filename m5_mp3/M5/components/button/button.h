#pragma once
#include "freertos/FreeRTOS.h"

#define KEY_EVENT_NULL                      0x0000

#define KEY_A_EVENT_SHORT                    0x0001
#define KEY_A_EVENT_LONG                     0x0002
#define KEY_A_EVENT_DOUBLE                   0x0003

#define KEY_B_EVENT_SHORT                    0x0004
#define KEY_B_EVENT_LONG                     0x0005
#define KEY_B_EVENT_DOUBLE                   0x0006

#define KEY_C_EVENT_SHORT                    0x0007
#define KEY_C_EVENT_LONG                     0x0008
#define KEY_C_EVENT_DOUBLE                   0x0009

uint16_t KeyRead(void);