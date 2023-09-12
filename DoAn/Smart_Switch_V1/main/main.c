#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
// #include "driver/timer.h"
#include "esp_log.h"
#include "Button.h"
#include "wifi_config.h"
#include "mqtt.h"
#define ONE_SECOND 1000000
#define KEY1 14
#define KEY2 25
#define LED1 26
#define LED2 27
#define LED_WIFI 17

void Button1_callback();
void Button2_callback();
void timer_of_button_1_callback();
void timer_of_button_2_callback();
#define TAG "Button"
/*Variable*/
int key1_sta = 0;
int key2_sta = 0;
/*Wifi variable*/
int is_connected_wifi = 0;
/*counter loop*/
uint64_t counter_loop = 0;
/*Json*/
json_gen_test_result_t result_test;
/*Button sta*/
Button_status_t device;
/*timer*/
esp_timer_handle_t timer1;
esp_timer_handle_t timer2;
int is_timer1_on = 0;
int is_timer2_on = 0;
int is_timer1_count_done = 0;
int is_timer2_count_done = 0;

const esp_timer_create_args_t timer_of_button1 = {
    .callback = timer_of_button_1_callback,
    .name = "Turn off button 1",
};
const esp_timer_create_args_t timer_of_button2 = {
    .callback = timer_of_button_2_callback,
    .name = "Turn off button 2",
};
void GPIO_init(void)
{
    gpio_config_t key_config = {
        .pin_bit_mask = (1 << KEY1) | (1 << KEY2),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&key_config);
    gpio_config_t output_config = {
        .pin_bit_mask = (1 << LED1) | (1 << LED2) | (1 << LED_WIFI),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&output_config);
    ESP_LOGI(TAG, "config done");
}
void Button1_callback()
{
    key1_sta = !key1_sta;
    device.sta_button1 = key1_sta;
    json_gen_perform_test(&result_test, key1_sta, key2_sta, 0, 0);
    send_data_to_server((uint8_t *)result_test.buf);
}
void Button2_callback()
{
    key2_sta = !key2_sta;
    device.sta_button2 = key2_sta;

    json_gen_perform_test(&result_test, key1_sta, key2_sta, 0, 0);
    send_data_to_server((uint8_t *)result_test.buf);
}

void swich_handle()
{

    if (device.sta_button1 == 1)
    {
        gpio_set_level(LED1, 1);
    }
    else
    {
        gpio_set_level(LED1, 0);
    }

    if (device.sta_button2 == 1)
    {
        gpio_set_level(LED2, 1);
    }
    else
    {
        gpio_set_level(LED2, 0);
    }

    // timer Handle;

    if (device.timer_button1 > 0 && !is_timer1_on)
    {
        device.sta_button1 = 1;
        gpio_set_level(LED1, 1);
        esp_timer_start_once(timer1, device.timer_button1 * ONE_SECOND);
        is_timer1_on = 1;
    }
    if (device.timer_button2 > 0 && !is_timer2_on)
    {
        device.sta_button1 = 1;
        gpio_set_level(LED2, 1);
        esp_timer_start_once(timer2, device.timer_button2 * ONE_SECOND);
        is_timer2_on = 1;
    }
}
void timer_of_button_1_callback()
{
    // turn off LED1 and Response data to MQTT
    gpio_set_level(LED1, 0);
    device.sta_button1 = 0;
    device.timer_button1 = 0;
    is_timer1_on = 0;

    json_gen_perform_test(&result_test, device.sta_button1, device.sta_button2, device.timer_button1, device.timer_button2);
    send_data_to_server((uint8_t *)result_test.buf);
}

void timer_of_button_2_callback()
{
    // turn off LED1 and Response data to MQTT
    gpio_set_level(LED2, 0);
    device.sta_button2 = 0;
    device.timer_button2 = 0;
    is_timer2_on = 0;

    json_gen_perform_test(&result_test, device.sta_button1, device.sta_button2, device.timer_button1, device.timer_button2);
    send_data_to_server((uint8_t *)result_test.buf);
}

// LED Wifi stastus
uint8_t sta_led_wifi;
void Led_wifi_status_handle(uint64_t cnt)
{

    if (is_connected_wifi == 0 && cnt % 10 == 0)
    {
        gpio_set_level(LED_WIFI, sta_led_wifi);
        sta_led_wifi = !sta_led_wifi;
    }
    if (is_connected_wifi == 1)
    {
        int32_t rssi_wifi = GetRSSI_Wifi();
        if ((rssi_wifi < -30) & (rssi_wifi > -80))
        { // blink LED follow Streng of Wifi
            int x = (110 + rssi_wifi);
            if (cnt % 100 < x)
                gpio_set_level(LED_WIFI, 1);
            else
                gpio_set_level(LED_WIFI, 0);
        }
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
    GPIO_init();
    Button_Handel_t button1, button2;
    button1.Press_callback = NULL;
    button1.PressShort_Callback = Button1_callback;
    button1.PressTimeout_Callback = NULL;
    button1.Release_Callback = NULL;

    button2.Press_callback = NULL;
    button2.PressShort_Callback = Button2_callback;
    button2.PressTimeout_Callback = NULL;
    button2.Release_Callback = NULL;

    button_init(&button1, KEY1);
    button_init(&button2, KEY2);
    // reset gia tri ban dau:

    esp_timer_create(&timer_of_button1, &timer1);
    esp_timer_create(&timer_of_button2, &timer2);

    while (1)
    {
        mqtt_set_callback(swich_handle);
        if (is_connected_wifi == 0)
        {
            wifi_manager();
            mqtt_app_start();
            is_connected_wifi = 1;
            device.sta_button1 = 0;
            device.sta_button2 = 0;
            device.timer_button1 = 0;
            device.timer_button2 = 0;
        }
        Led_wifi_status_handle(counter_loop++);
        button_Handle(&button1);
        button_Handle(&button2);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
