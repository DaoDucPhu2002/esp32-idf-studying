#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

static esp_adc_cal_characteristics_t adc_value;
static QueueSetHandle_t uart2_queue;
#define BUFF_SIZE 1024 * 2
void uart_init()
{
    uart_config_t uart_2 =
        {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,

        };
    uart_driver_install(UART_NUM_2, BUFF_SIZE, BUFF_SIZE, 20, &uart2_queue, 0);
    uart_param_config(UART_NUM_2, &uart_2);
    uart_set_pin(UART_NUM_2, 16, 17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void adc_init()
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc_value);
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    // LOGI("UART_CONFIG", );
}

void app_main(void)
{
    adc_init();
    uart_init();
    while (1)
    {
        int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
        uint8_t data[100];
        ESP_LOGI("WRITE_UART", "Write bytes");
        sprintf((char *)data, "ADC_VALUE: %d", adc_value);
        uart_write_bytes(UART_NUM_2, data, strlen((char *)data));
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
