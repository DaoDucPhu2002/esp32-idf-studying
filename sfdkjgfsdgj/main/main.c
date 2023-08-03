#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_AP_SSID "YourAPSSID"
#define WIFI_AP_PASSWORD "YourAPPassword"
#define WIFI_STA_SSID "YourNetworkSSID"
#define WIFI_STA_PASSWORD "YourNetworkPassword"

static const char *TAG = "wifi_sta_ap";
static EventGroupHandle_t wifi_event_group;

const int WIFI_AP_CONNECTED_BIT = BIT0;
const int WIFI_STA_CONNECTED_BIT = BIT1;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_AP_STACONNECTED:
    ESP_LOGI(TAG, "Station connected to AP");
    xEventGroupSetBits(wifi_event_group, WIFI_AP_CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_START:
    ESP_LOGI(TAG, "Station mode started");
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "Connected to network. IP address: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    xEventGroupSetBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    ESP_LOGI(TAG, "Disconnected from network");
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

void wifi_init_sta_ap()
{
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config_ap = {
      .ap = {
          .ssid = WIFI_AP_SSID,
          .password = WIFI_AP_PASSWORD,
          .ssid_len = 0,
          .max_connection = 4,
          .authmode = WIFI_AUTH_WPA2_PSK,
      },
  };

  wifi_config_t wifi_config_sta = {
      .sta = {
          .ssid = WIFI_STA_SSID,
          .password = WIFI_STA_PASSWORD,
      },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_ap));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config_sta));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "Switched to STA + AP mode");
}

void app_main()
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

  wifi_init_sta_ap();

  while (1)
  {
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_AP_CONNECTED_BIT | WIFI_STA_CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_AP_CONNECTED_BIT)
    {
      ESP_LOGI(TAG, "AP connected");
      // Thực hiện các hoạt động liên quan đến kết nối AP thành công
    }

    if (bits & WIFI_STA_CONNECTED_BIT)
    {
      ESP_LOGI(TAG, "STA connected");
      // Thực hiện các hoạt động liên quan đến kết nối STA thành công
    }
  }
}
