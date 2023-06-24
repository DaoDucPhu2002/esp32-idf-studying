#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"

#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define UART_SIM UART_NUM_2
#define UART_DEBUG UART_NUM_0
#define BUF_SIZE (1024)
#define SIM_EN 32

void uart2_init()
{
    uart_config_t uart_sim = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,

    };
    uart_driver_install(UART_SIM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    uart_param_config(UART_SIM, &uart_sim);
    uart_set_pin(UART_SIM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_DEBUG, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    uart_param_config(UART_DEBUG, &uart_sim);
    uart_set_pin(UART_DEBUG, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
void IO_init(void)
{
    esp_rom_gpio_pad_select_gpio(SIM_EN);
    gpio_set_direction(SIM_EN, GPIO_MODE_OUTPUT);
}

void sim_power_on(void)
{
    gpio_set_level(SIM_EN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(SIM_EN, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI("SIM", "POWER_ON");
}
void sim_power_off(void)
{
    gpio_set_level(SIM_EN, 0);
    ESP_LOGI("SIM", "POWER_OFF");
}
uint8_t send_Data(int uart, const char *data)
{
    const uint8_t len = strlen(data);
    const uint8_t tx_bytes = uart_write_bytes(UART_SIM, data, len);
    ESP_LOGI("UART", "sendata");
    return tx_bytes;
}
//
void uart_reciever_task(void)
{

    uint8_t data_rx[1000];

    uart_read_bytes(UART_SIM, &data_rx, 1000, 100);

    // ESP_LOGI("RECIVER ", data_rx);
    printf("%s \n", (char *)data_rx);
}

void app_main(void)
{
    IO_init();
    uart2_init();
    sim_power_on();

    while (1)
    {
    }
}
