#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "ota_app.h"

static const char *TAG = "APP_OTA";

static char http_url_update_firmware[70] = "http://test.api.lctech.vn/firmware/esp32/OTA";
esp_http_client_config_t config = {
    .url = http_url_update_firmware,
    .skip_cert_common_name_check = true,
    .keep_alive_enable = true,
};

uint8_t old_duty = 0;

#define OTA_URL_SIZE 256

esp_err_t addHeader(esp_http_client_handle_t http_client)
{

    esp_http_client_set_header(http_client, "x-ESP32-STA-MAC", "10521C761794");
    esp_http_client_set_header(http_client, "x-ESP32-version", "0.0");

    return ESP_OK;
}

void start_ota(void)
{
    ESP_LOGI(TAG, "Starting OTA");

    esp_http_client_config_t *pconfig = &config;
    pconfig->cert_pem = NULL;
    pconfig->skip_cert_common_name_check = true;

    esp_https_ota_config_t ota_config = {
        .http_config = pconfig,
        .http_client_init_cb = addHeader,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (https_ota_handle == NULL)
    {
        ESP_LOGE(TAG, "OTA_HANDLE_NULL");
    }

    // init other object for get length
    esp_http_client_handle_t __http_client = esp_http_client_init(&config);
    if (__http_client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        goto END;
    }
    /* ADD header*/
    esp_http_client_set_header(__http_client, "x-ESP32-STA-MAC", "10521C761794");
    esp_http_client_set_header(__http_client, "x-ESP32-version", "0.0");

    err = esp_http_client_open(__http_client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        goto END;
    }

    int length_image_firmware = esp_http_client_fetch_headers(__http_client);
    ESP_LOGI(TAG, "Length Image : %d", length_image_firmware);

    while (1)
    {

        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        {
            break;
        }
        int process_len = esp_https_ota_get_image_len_read(https_ota_handle);
        uint8_t new_duty = (process_len * 100 / length_image_firmware); // % download

        if (new_duty != old_duty)
        {
            printf("%d %%\n", new_duty);
            old_duty = new_duty;
        }
    }

    esp_err_t ota_finish_err = esp_https_ota_finish(https_ota_handle);

    if (ota_finish_err == ESP_OK)
    {
        esp_restart();
    }
    else
    {
    END:
        ESP_LOGE(TAG, "Firmware upgrade failed");
        vTaskDelete(NULL);
    }
}
