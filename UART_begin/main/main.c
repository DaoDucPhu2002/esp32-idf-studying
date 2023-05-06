#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/uart.h>
#include <esp_log.h>
// config uart

#define BUFF_SIZE (1024 * 2)
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
    uart_driver_install(UART_NUM_0, BUFF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char *logName, const char *data)
{
    /*
        LogName: Logging Name  ten nhat ki
    */
    const int len = strlen(data);                                // do dai chuoi data
    const int txBytes = uart_write_bytes(UART_NUM_0, data, len); // ham gui du lieu va tra ve so byte da gui
    ESP_LOGI(logName, "Da ghi %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
    // ham gui len Hello World sau moi 1s
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO); // Dat muc uu tien cho the RX_TASK_TAG
    while (1)
    {
        sendData(TX_TASK_TAG, "DAO DUC PHU");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_Task(void *arg)
{
    // cai dat nhat ki
    static const char *RX_TASK_TAG = "RX_TASK";
    uint8_t *data = (uint8_t)malloc(BUFF_SIZE + 1);
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_0, data, BUFF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'  ", rxBytes, data);
            // ESP_LOG_BUFFER_HEXDUMP();
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}

void app_main(void)
{
    uart_init();
    xTaskCreate(rx_Task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}
