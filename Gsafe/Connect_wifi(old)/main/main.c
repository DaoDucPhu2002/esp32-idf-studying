#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define UART_NUM_0_TX 1  // GPIO17 là TX cho UART 0
#define UART_NUM_0_RX 3  // GPIO16 là RX cho UART 0
#define UART_NUM_2_TX 17 // GPIO4 là TX cho UART 2
#define UART_NUM_2_RX 16 // GPIO5 là RX cho UART 2
#define BUFF_SIZE 1024
void init_uart()
{
    // Cấu hình UART cho UART 0
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

    // Cấu hình UART cho UART 2
    uart_config_t uart2_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART_NUM_0, BUFF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart2_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

// Task để nhận từ UART 0 và gửi qua UART 2
void uart0_to_uart2_task(void *pvParameters)
{
    uint8_t data;
    while (1)
    {
        // Đọc dữ liệu từ UART 0
        uart_read_bytes(UART_NUM_0, &data, 1, portMAX_DELAY);

        // Gửi dữ liệu qua UART 2
        uart_write_bytes(UART_NUM_2, (const char *)&data, 1);

        // In dữ liệu nhận được
        printf("UART0 -> UART2: %c\n", data);
    }
}

// Task để nhận từ UART 2 và gửi qua UART 0
void uart2_to_uart0_task(void *pvParameters)
{
    uint8_t data;
    while (1)
    {
        // Đọc dữ liệu từ UART 2
        uart_read_bytes(UART_NUM_2, &data, 1, portMAX_DELAY);

        // Gửi dữ liệu qua UART 0
        uart_write_bytes(UART_NUM_0, (const char *)&data, 1);

        // In dữ liệu nhận được
        printf("UART2 -> UART0: %c\n", data);
    }
}

void app_main()
{
    init_uart();

    // Tạo task để nhận từ UART 0 và gửi qua UART 2
    xTaskCreate(uart0_to_uart2_task, "uart0_to_uart2_task", 2048, NULL, 10, NULL);

    // Tạo task để nhận từ UART 2 và gửi qua UART 0
    xTaskCreate(uart2_to_uart0_task, "uart2_to_uart0_task", 2048, NULL, 10, NULL);
}
