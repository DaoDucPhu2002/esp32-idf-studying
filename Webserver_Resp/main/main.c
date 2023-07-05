#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <sys/param.h>
#include "esp_netif.h"

#include "esp_tls_crypto.h"
#include <esp_http_server.h>

#define Tag "Web Server"
#define NAMESPACE "Wifi SSID and Password"
/*=============================================
        EEPROM Handler
===============================================*/
esp_err_t init_nvs()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
    }
    return ret;
}
void wifi_write_data_to_eeprom(const char *ssid_value, const char *password_value)
{
    nvs_handle_t nvs_handler;
    esp_err_t ret;
    ret = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handler);
    if (ret != ESP_OK)
    {
        return;
    }
    ret = nvs_set_i32(nvs_handler, ssid_value, password_value);
    if (ret != ESP_OK)
    {
        // error handler
    }
    nvs_close(nvs_handler);
}

int32_t wifi_read_data_form_eeprom(const char *data_key)
{
    nvs_handle_t read_handle;
    esp_err_t ret;
    int32_t data_Value;
    ret = nvs_open(NAMESPACE, NVS_READONLY, &read_handle);
    if (ret != ESP_OK)
    {
        return 0;
    }
    ret = nvs_get_i32(read_handle, data_key, &data_Value);
    if (ret != ESP_OK)
    {
        return 0;
    }
    nvs_close(read_handle);
    return data_Value;
}
esp_err_t wifi_config()
{
    
}

void wifi_scan(void)
{
}

/* ============================================

================================================*/

static void
wifi_event_handle(void *arg, esp_event_base_t event_base, int32_t event_id, void *even_data)
{
}

void app_main(void)
{
}