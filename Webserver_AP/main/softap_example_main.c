#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#define TAG "web_server"

/* Maximum number of scanned Wi-Fi networks */
#define MAX_SCAN_RESULTS 10

/* HTML template for displaying the Wi-Fi network list */
const char *html_template = "<html><body><h1>Wi-Fi Networks</h1>"
                            "<table>"
                            "<tr><th>SSID</th><th>Channel</th><th>RSSI</th></tr>"
                            "%s"
                            "</table></body></html>";

/* Request handler for the root URL */
esp_err_t root_get_handler(httpd_req_t *req)
{
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 200,
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    wifi_ap_record_t *ap_list = malloc(ap_count * sizeof(wifi_ap_record_t));
    esp_wifi_scan_get_ap_records(&ap_count, ap_list);

    char rows[500];
    memset(rows, 0, sizeof(rows));

    for (int i = 0; i < ap_count; i++)
    {
        char row[100];
        snprintf(row, sizeof(row), "<tr><td>%s</td><td>%d</td><td>%d</td></tr>", ap_list[i].ssid, ap_list[i].primary, ap_list[i].rssi);
        strcat(rows, row);
    }

    char html_response[600];
    snprintf(html_response, sizeof(html_response), html_template, rows);

    httpd_resp_send(req, html_response, strlen(html_response));

    free(ap_list);

    return ESP_OK;
}

/* HTTP server configuration */
httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL};

/* Start the HTTP server */
httpd_handle_t start_web_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the server
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Register URI handlers
        httpd_register_uri_handler(server, &root_uri);
        return server;
    }

    ESP_LOGE(TAG, "Error starting server!");
    return NULL;
}

/* Stop the HTTP server */
void stop_web_server(httpd_handle_t server)
{
    httpd_stop(server);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    httpd_handle_t server = start_web_server();
    if (server == NULL)
    {
        ESP_LOGE(TAG, "Failed to start web server!");
        return;
    }

    ESP_LOGI(TAG, "Web server started!");

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    stop_web_server(server);
}
