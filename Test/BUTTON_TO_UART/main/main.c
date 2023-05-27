#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "string.h"
#include "esp_log.h"
#include "Button.h"

#define KEY1 25
#define KEY2 26
#define KEY3 27

#define UART_NUM UART_NUM_2
#define BUFF_SIZE 1024 * 2

static QueueHandle_t uart2_queuel;
int dem;

Button_Handel btn1, btn2, btn3;
void GPIO_Init()
{
    gpio_config_t GPIO_Config = {};
    GPIO_Config.pin_bit_mask = (1 << KEY1) | (1 << KEY2) | (1 << KEY3);
    GPIO_Config.mode = GPIO_MODE_INPUT;
    GPIO_Config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    GPIO_Config.pull_up_en = GPIO_PULLUP_ENABLE;
    GPIO_Config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&GPIO_Config);
    ESP_LOGI(" GPIO CONFIG ", " GPIO_Done");
}

void UART_init()
{
    uart_config_t uart_2_config =
        {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
    uart_driver_install(UART_NUM, BUFF_SIZE, BUFF_SIZE, 20, &uart2_queuel, 0);
    uart_param_config(UART_NUM, &uart_2_config);
    uart_set_pin(UART_NUM, 16, 17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI("UART CONFIG ", "UART_Done");
}

void senData(int num)
{
    uint8_t data[100];
    sprintf((char *)data, "SO XE O TO TRONG HAM LA: %d \n", num);
    uart_write_bytes(UART_NUM, data, strlen((char *)data));
    ESP_LOGI("WRITE_UART", "Write bytes");
}

void app_main(void)
{
    UART_init();
    GPIO_Init();
    buton_init(&btn1, KEY1);
    buton_init(&btn2, KEY2);
    buton_init(&btn3, KEY3);
    while (1)
    {
        button_Handle(&btn1);
        button_Handle(&btn2);
        button_Handle(&btn3);
        if (btn1.state == 1)
        {
            senData(1);
        }
        if (btn1.state == 2)
        {
            senData(2);
        }
        if (btn1.state == 3)
        {
            senData(3);
        }
    }
}
