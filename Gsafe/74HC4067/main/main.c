#include "main.h"
#include "Sensor_input.h"
#include "Relay_output.h"
#include "Keypad.h"
#include "Charger.h"
int sta_sensor[16];
uint16_t vbat, adc_key, adc_rf, adc_charg;

void uart_init()
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

void GPIO_config_test(void)
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
void keypad_adc_config(void)
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

void ADC_task(void *pvParameters)
{
    uint8_t temp = 0;
    KeyPad_State key_pad;
    uint16_t adc_vbat = 0;

    // uint16_t rf_value;
    for (;;)
    {
        temp = temp + 1;
        adc_vbat += adc1_get_raw(ADC_VBAT);

        get_keypad_state();
        adc_charg = adc1_get_raw(ADC_CHARGER);

        if (temp > 10)
        {
            adc_vbat /= 10;
            vbat = adc_vbat / 3 - (adc_vbat - 1800) / 50; // bien toan bo
            printf("%d Vol\n", vbat);
            // printf("Dadc: %d \n", vbat / 10);

            temp = 0;
            adc_vbat = 0;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void Sensor_Handle_task(void *pvParameters)
{
    for (;;)
    {
        for (int i = 0; i < 16; i++)
        {
            sta_sensor[i] = get_state_sensor(i);
            printf("%d, ", sta_sensor[i]);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        printf("\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    GPIO_config_test();
    keypad_adc_config();

    xTaskCreate(Sensor_Handle_task, "Main Task", 2048 * 5, NULL, 6, NULL);
    xTaskCreate(ADC_task, "Keypad Task", 1024 * 5, NULL, 3, NULL);
}
