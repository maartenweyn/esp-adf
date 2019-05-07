#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define ACK_VAL     0x0              /*!< I2C ack value */
#define NACK_VAL    0x1              /*!< I2C nack value */
#define WRITE_BIT   I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT    I2C_MASTER_READ  /*!< I2C master read */
#define ADDR        0x5C

static const char * TAG = "DHT12";
static const i2c_config_t i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000
};

esp_err_t dht12_init() {
    int res;
    res = i2c_param_config(I2C_NUM_1, &i2c_cfg);
    res |= i2c_driver_install(I2C_NUM_1, i2c_cfg.mode, 0, 0, 0);
    if(res != ESP_OK){
        ESP_LOGE(TAG, "DHT12 I2C Init error ");
    }
    return res;
}

void dataRead(float *tmpIn, float *humIn) {
    uint8_t temp_h=0,temp_l=0,humi_h,humi_l,crc_tmp,tmp;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADDR << 1 | WRITE_BIT, ACK_VAL);
    vTaskDelay(10 / portTICK_RATE_MS);
    i2c_master_write_byte(cmd, 0x00, ACK_VAL);

    vTaskDelay(30 / portTICK_RATE_MS);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADDR << 1 | READ_BIT, ACK_VAL);
    vTaskDelay(10 / portTICK_RATE_MS);    
    i2c_master_read_byte(cmd, &humi_h, ACK_VAL);
    i2c_master_read_byte(cmd, &humi_l, ACK_VAL);
    i2c_master_read_byte(cmd, &temp_h, ACK_VAL);
    i2c_master_read_byte(cmd, &temp_l, ACK_VAL);
    i2c_master_read_byte(cmd, &crc_tmp, NACK_VAL);
    
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    tmp = (uint8_t)(temp_h + temp_l + humi_h + humi_l);
		
	if(tmp == crc_tmp)
	{
		float temp = temp_h + temp_l / 10;
		float humi = humi_h + humi_l / 10;
			
        *tmpIn = temp;
        *humIn = humi;
    }
	else
    {
        ESP_LOGI(TAG, "temp_h: %d", temp_h);
        ESP_LOGI(TAG, "temp_l: %d", temp_l);
        ESP_LOGI(TAG, "humi_h: %d", humi_h);
        ESP_LOGI(TAG, "humi_l: %d", humi_l);
        ESP_LOGI(TAG, "crc value: %d", crc_tmp);
            
        ESP_LOGI(TAG, "CRC Error");
        *tmpIn = 0;
        *humIn = 0;
	}

    return ret;
} 