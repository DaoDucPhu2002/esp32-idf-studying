#include <stdio.h>

#include "sdkconfig.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/gpio.h>

#define LED1 19
#define LED2 18
#define LED3 17
#define LED4 16

#define LatchPin 25
#define ClockPin 27
#define DataPin 26

uint8_t so[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

/*=================  IC 74HC595 Handle  ================*/
void Seg_Handle(uint8_t data)
{

    uint8_t temp;
    gpio_set_level(ClockPin, 0);
    for (int i = 0; i < 8; i++)
    {
        temp = data;
        temp = data & 0x80;
        if (temp == 0x80)
        {
            gpio_set_level(DataPin, 1);
        }
        else
        {
            gpio_set_level(DataPin, 0);
        }
        data = data << 1;
        gpio_set_level(ClockPin, 0);
        gpio_set_level(ClockPin, 1);
    }
    gpio_set_level(LatchPin, 1);
    gpio_set_level(LatchPin, 0);
}

void Quet_LED(int number)
{
    // led1
    gpio_set_level(LED2, 1);
    gpio_set_level(LED3, 1);
    gpio_set_level(LED4, 1);
    gpio_set_level(LED1, 0);
    Seg_Handle(so[number / 1000]);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    // led2
    gpio_set_level(LED1, 1);
    gpio_set_level(LED3, 1);
    gpio_set_level(LED4, 1);
    gpio_set_level(LED2, 0);

    Seg_Handle(so[(number % 1000) / 100]);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // led1
    gpio_set_level(LED1, 1);
    gpio_set_level(LED2, 1);
    gpio_set_level(LED4, 1);
    gpio_set_level(LED3, 0);

    Seg_Handle(so[((number % 1000) % 100) / 10]);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // led1
    gpio_set_level(LED1, 1);
    gpio_set_level(LED2, 1);
    gpio_set_level(LED3, 1);
    gpio_set_level(LED4, 0);

    Seg_Handle(so[((number % 1000) % 100) % 10]);
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
/* Config GPIO PIN*/
void GPIO_Init()
{
    /* Config GPIO OUTPUT*/
    gpio_config_t GPIO_config = {};
    GPIO_config.pin_bit_mask = (1 << 15) | (1 << LED1) | (1 << LED2) | (1 << LED3) | (1 << LED4) | (1 << LatchPin) | (1 << DataPin) | (1 << ClockPin);
    GPIO_config.mode = GPIO_MODE_OUTPUT;
    GPIO_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    GPIO_config.pull_up_en = GPIO_PULLUP_DISABLE;
    GPIO_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&GPIO_config);
}
void app_main(void)
{
    GPIO_Init();
    while (1)
    {
        Quet_LED(1234);
    }
}
