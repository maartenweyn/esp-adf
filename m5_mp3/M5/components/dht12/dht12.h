#pragma once

#include "driver/i2c.h"
esp_err_t dht12_init();
void dataRead(float *tmpIn, float *humIn);

