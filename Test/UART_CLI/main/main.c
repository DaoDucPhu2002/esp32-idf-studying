#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM 3 // so ki tu giong nhau lien tiep; vd 000 hay 111 aaa;

#define UART_BUFF_SIZE (1024)
#define RD_BUFF_SIZE UART_BUFF_SIZE
#define TAG_RX "UART_RX"

static QueueHandle_t uart0_queue; // khoi tao  buff cho uart ; vi uart se luu ki tu vao queue

void UART_RX_Task(void)

{

    uart_event_t event; // khoi tao event
    size_t buffer_size;
    uint8_t *data_rx = (uint8_t *)malloc(RD_BUFF_SIZE); // khoi tao buff cho uart;
    for (;;)
    {
        ESP_LOGI(TAG_RX, "UART Read Data");
        // chờ khi có event
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(data_rx, RD_BUFF_SIZE);
            switch (event.type)
            {
            case UART_DATA:
                
                break;

            default:
                break;
            }
        }
    }
}

void UART_TX_Task(void)
{

    for (;;)
    {
        ESP_LOGI("UART_TX", "UART Send Data");
        // pass
    }
}

void app_main(void)
{
}
