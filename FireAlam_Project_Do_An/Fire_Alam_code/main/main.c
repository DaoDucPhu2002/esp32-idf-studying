#include "main.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "Alam_Handler.h"
#include "wifi_config.h"
#include "webserver.h"
#include "mqtt.h"

Alam_Handler_t alarm_data;
uint8_t ESP_ID[8];
uint8_t sdata[17];
int byteLoad = 0;

void data_process()
{
    byteLoad = 0;
    for (int i = 0; i < 6; i++)
    {
        sdata[byteLoad++] = ESP_ID[i]; // 6
    }
    sdata[byteLoad++] = GetRSSI_Wifi(); // 7
    sdata[byteLoad++] = alarm_data.is_line_not_use;
    sdata[byteLoad++] = alarm_data.Status; // 8
    for (int i = 0; i < 8; i++)
    {
        sdata[byteLoad++] = alarm_data.line_status[i]; // 16
    }
    // for (int i = 0; i < 16; i++)
    // {
    //     printf("%2x - ", sdata[i]);
    // }
    // printf(". \n");
    printf("Line NotUse: %2x", alarm_data.is_line_not_use);
}

void alam_config_gpio()
{
    gpio_config_t io_config;
    io_config.pin_bit_mask = (1 << 25) | (1ULL << 33) | (1ULL << 32) | (1 << EOLINE) | (1 << INSEN) |
                             (1 << LED_GREEN) | (1 << LED_RED) | (1 << LED_WIFI) | (1 << BEEP) | (1 << LAMP1) | (1 << OUT1) | (1 << OUT2);
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_config);

    gpio_config_t io_input;
    io_input.pin_bit_mask = (1 << EOLINE) | (1 << INSEN) | (1 << BTN_WF_CONF);
    io_input.mode = GPIO_MODE_INPUT;
    io_input.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_input.pull_up_en = GPIO_PULLUP_DISABLE;
    io_input.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_input);
}
void LedStatus(uint64_t cnt)
{
    if (alarm_data.Status == ALARM_STATUS)
    {
        if (cnt % 10 < 5)
        {
            gpio_set_level(LED_GREEN, 1);
            gpio_set_level(LED_RED, 0);
        }
        else
        {
            gpio_set_level(LED_GREEN, 1);
            gpio_set_level(LED_RED, 1);
        }
    }
    else if (alarm_data.Status == NORMAL_STATUS)
    {
        if (cnt % 10 < 5)
        {
            gpio_set_level(LED_GREEN, 1);
            gpio_set_level(LED_RED, 1);
        }
        else
        {
            gpio_set_level(LED_GREEN, 0);
            gpio_set_level(LED_RED, 1);
        }
    }
    else if (alarm_data.Status == FAIL_LINE)
    {
        if (cnt % 10 < 5)
        {
            gpio_set_level(LED_GREEN, 1);
            gpio_set_level(LED_RED, 0);
        }
        else
        {
            gpio_set_level(LED_GREEN, 0);
            gpio_set_level(LED_RED, 1);
        }
    }
    if (is_connected_to_wifi() == 1)
    {
        if (cnt % 10 < 5)
        {
            gpio_set_level(LED_WIFI, 1);
        }
        else
        {
            gpio_set_level(LED_WIFI, 0);
        }
    }
    else
    {
        gpio_set_level(LED_WIFI, 1);
    }
}
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    alam_config_gpio();
    alarm_data.is_line_not_use = 0xff;
    uint64_t count = 0;

    xTaskCreate(wifi_manager, "DAO DUC PHU", 4084, NULL, 3, NULL);

    while (1)
    {
        if (count % 10 == 0)
        {

            check_alam();
            Overall_status();

            Alam_check_out();
            data_process();

            send_data_to_server(sdata);
            printf("%X\n", sdata);
        }
        LedStatus(count);
        count++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
