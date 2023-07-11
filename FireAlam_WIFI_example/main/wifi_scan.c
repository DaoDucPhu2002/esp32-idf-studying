#include "wifi_scan.h"

static char *auth_mode_type(wifi_auth_mode_t auth_mode)
{
    char *type[] = {"OPEN",
                    "WEP",
                    "WPA PSK",
                    "WPA2 PSK",
                    "WPA WPA2 PSK",
                    "MAX"};
    return type[auth_mode];
}

wifi_ap_record_t *wifi_scan()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    uint16_t temp = MAXINUM_AP;
    wifi_ap_record_t ap_info[MAXINUM_AP];
    uint16_t ap_count;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&temp, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    for (int i = 0; (i < MAXINUM_AP) && (i < ap_count); i++)
    {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }
    return ap_info;
}
