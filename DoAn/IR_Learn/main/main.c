#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#define GPIO_PIN 14
// uint64_t time_state[67];
uint64_t last_time;
int i = 0;
bool last_state;
void IRAM_ATTR gpio_interrupt_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    bool current_state = gpio_get_level(gpio_num);
    uint64_t current_time = esp_timer_get_time();
    if (current_state != last_state)
    {

        uint64_t time_xung = current_time - last_time;

        // time_state[i] = time_xung;
        printf("time %lld\n", time_xung);
    }
    last_state = current_state;
    last_time = current_time;
}
void GPIO_INTR_config(int pin)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << GPIO_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_PIN, gpio_interrupt_handler, (void *)GPIO_PIN);
}

void app_main(void)
{
    GPIO_INTR_config(14);
    esp_timer_init();
    while (1)
    {
        printf("Time\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
