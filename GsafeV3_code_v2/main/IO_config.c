#include "IO_config.h"

void uart2_config()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // cai dat phan luong cua o cung
        .source_clk = UART_SCLK_DEFAULT,
    };

    // set uart pin
    uart_driver_install(UART_NUM_2, BUFF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void io_config(void)
{
    gpio_config_t io_config = {};
    io_config.pin_bit_mask = (1 << SA0) | (1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << RL1) | (1 << RL2) | (1 << RL3) | (1 << RL4) | (1 << VBAT_EN);
    io_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_config);

    io_config.pin_bit_mask = (1 << EOLINE) | (1 << INSEN);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_config);
}
void adc_config(void)
{

    adc1_config_channel_atten(ADC_KEY, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);
    ESP_LOGI("ADC", "ADC config is done");

    adc1_config_channel_atten(ADC_CHARGER, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC_RF, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC_VBAT, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);
}
