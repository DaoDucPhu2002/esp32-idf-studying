/*Sample FreeRTOS*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "input_dev.h"

EventGroupHandle_t xEvenGroup;

#define BIT_EVENT_BUTTON_PRESS (1 << 0)
#define BIT_ENVET_UART_RECV (1 << 1)
TimerHandle_t xTimer[2];
int x = 0;
void vTask1(void *pvParameters)
{
    for (;;)
    {
        // đợi even
        EventBits_t uxBits = xEventGroupWaitBits(
            xEvenGroup,                                   // even group
            BIT_EVENT_BUTTON_PRESS | BIT_ENVET_UART_RECV, // đưa vào even muôn chạy
            pdTRUE,                                       // reset bit về 0 sau khi chạy
            pdFALSE,                                      //  đợi 1 hoặc 2 even vượt qua // true là chờ cả 2
            portMAX_DELAY                                 // thời giamn lấy bit

        );
        if (uxBits & BIT_EVENT_BUTTON_PRESS)
        {
            printf("BUTTON PRESS\n");

            gpio_set_level(18, x);
            x = 1 - x;
        }
        if (uxBits & BIT_ENVET_UART_RECV)
        {
            printf("UART DATA\n");
        }
    }
}
void vTimer_Callback(TimerHandle_t xTimer)
{
    configASSERT(xTimer);
    uint32_t id_time = pvTimerGetTimerID(xTimer);
    if (id_time == 0)
    {
        gpio_set_level(18, x);
        x = 1 - x;
    }
    else if (id_time == 1)
    {
        printf("Test Timer 1\n");
    }
}

void LED_config()
{

    gpio_config_t GPIO_config = {};
    ///  GPIO_config.pin_bit_mask = (1 << gpio);
    GPIO_config.pin_bit_mask = (1 << 18);
    GPIO_config.mode = GPIO_MODE_OUTPUT;
    GPIO_config.pull_up_en = GPIO_PULLUP_DISABLE;
    GPIO_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    GPIO_config.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&GPIO_config);
}
void button_handle(int pin)
{
    if (pin == GPIO_NUM_25)
    {
        xEventGroupSetBits(xEvenGroup, BIT_EVENT_BUTTON_PRESS);
    }
}
void app_main(void)
{
    LED_config();
    xTimer[0] = xTimerCreate("Timer Blink",      // Ten Cua Timer
                             pdMS_TO_TICKS(500), // Thoi gian tre ms doc them o trang chu cua rtos
                             pdTRUE,             // co autoReload khong
                             (void *)0,          // ID cua timer
                             vTimer_Callback);   // ham  calll back cua timer
    xTimer[1] = xTimerCreate("Timer Print Uart", pdMS_TO_TICKS(1000), pdTRUE, (void *)1, vTimer_Callback);
    // xTimerStart(xTimer[0], 0);
    // xTimerStart(xTimer[1], 0);
    xEvenGroup = xEventGroupCreate();
    input_create(25, HI_TO_LO);
    input_set_callback(button_handle);
    xTaskCreate(vTask1, "Task1", 1024, NULL, 4, NULL);
}
