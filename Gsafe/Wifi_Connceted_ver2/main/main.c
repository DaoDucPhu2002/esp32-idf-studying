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

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_smartconfig.h"
#include "wifi_config.h"
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    int is_connect_wifi = 0;
    int failedConnection = 0;
    int maxFailedConnect = 10;
    init_nvs();
    while (1)
    {
        if (is_connect_wifi == 0)
        {

            ESP_LOGI("Wifi", "Connecting to Wifi ...");
            read_wifi_and_connect();
            if (esp_wifi_connect() == ESP_OK)
            {
                printf("Wifi_Connected\n");
                is_connect_wifi = 1;
            }
            else
            {
                failedConnection++;
                printf("Connect Failed\n");
            }
            if (failedConnection > maxFailedConnect)
            {
                printf("Wifi Manager Start\n");
                wifi_manager();
                is_connect_wifi = 1;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
