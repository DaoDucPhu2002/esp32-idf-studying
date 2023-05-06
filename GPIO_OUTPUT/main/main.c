#include <stdio.h>
#include "sdkconfig.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/gpio.h>

#define pinA 12
#define pinB 14
#define pinC 27
#define pinD 26
#define pinE 25
#define pinF 16
#define pinG 17
#define pinDot 19

uint8_t gpio_seg[8] = {
    pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDot};

uint8_t so[10] = {
    0x3f, 0x06, 0x5B, 0x40, 0x66, 0x6D, 0x7D, 0x07, 0x7f, 0x6f};

void GPIO_init()
{
    gpio_config_t GPIO_config = {};
    ///  GPIO_config.pin_bit_mask = (1 << gpio);
    GPIO_config.pin_bit_mask = (1 << 18) | (1 << pinA) | (1 << pinB) | (1 << pinC) | (1 << pinD) | (1 << pinE) |
                               (1 << pinF) | (1 << pinG) | (1 << pinDot);
    GPIO_config.mode = GPIO_MODE_OUTPUT;
    GPIO_config.pull_up_en = GPIO_PULLUP_DISABLE;
    GPIO_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    GPIO_config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&GPIO_config);
}

void dem_0_9(int number)
{
    uint8_t output = so[number];
    for (int i = 0; i < 8; i++)
    {
        gpio_set_level(gpio_seg[i], (output >> i) & 1);
    }
}
void dem()
{
    for (int i = 0; i < 10; i++)
    {
        dem_0_9(i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void app_main(void)
{
    // config GPIO 18
    GPIO_init();

    // loop
    while (1)
    {

        gpio_set_level(18, 1);
        // for (int i = 0; i < 8; i++)
        // {
        //     gpio_set_level(gpio_seg[i], 0);
        // }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        gpio_set_level(18, 0);
        // for (int i = 0; i < 8; i++)
        // {
        //     gpio_set_level(gpio_seg[i], 1);
        // }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        dem();
    }
}
