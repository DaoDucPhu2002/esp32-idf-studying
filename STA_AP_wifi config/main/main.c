#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#define DEFAULT_SCAN_LIST_SIZE 30

static const char *TAG = "wifi_example";
wifi_ap_record_t ap_records[DEFAULT_SCAN_LIST_SIZE];
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "Connected to the AP");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Disconnected from the AP");
        esp_wifi_connect();
    }
    else if (event_id == WIFI_EVENT_SCAN_DONE)
    {
        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);
        printf("Found %d APs:\n", ap_count);

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));
        for (int i = 0; i < ap_count; i++)
        {
            printf("SSID: %s, RSSI: %d\n", ap_records[i].ssid, ap_records[i].rssi);
        }
    }
}

void app_main()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t sta_config = {
        .sta = {
            .ssid = "YOUR_SSID",
            .password = "YOUR_PASSWORD",
            .bssid_set = 0,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Quét mạng WiFi
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
}