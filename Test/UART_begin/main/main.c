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

void app_main(void)
{
    uart_init();
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}
