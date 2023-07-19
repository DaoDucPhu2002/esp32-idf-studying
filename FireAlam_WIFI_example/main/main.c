
#include "main.h"

/*Config wifi*/

// static char *auth_mode_type(wifi_auth_mode_t auth_mode)
// {
//     char *type[] = {"OPEN",
//                     "WEP",
//                     "WPA PSK",
//                     "WPA2 PSK",
//                     "WPA WPA2 PSK",
//                     "MAX"};
//     return type[auth_mode];
// }

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

/*Webserver handler*/
const char *html_template =
    "<html>"
    "<head>"
    "<title>ESP32 AP Scan</title>"
    "<style>"
    "table {border-collapse: collapse; width: 100%;}"
    "th, td {border: 1px solid black; padding: 8px; text-align: left;}"
    "</style>"
    "</head>"
    "<body>"
    "<h2>ESP32 AP Scan</h2>"
    "<table>"
    "<tr>"
    "<th>SSID</th>"
    "<th>Channel</th>"
    "<th>RSSI</th>"
    "</tr>"
    "%s" // Placeholder for table rows
    "</table>"
    "</body>"
    "</html>";

esp_err_t get_handler(httpd_req_t *req)
{
    char *buf = NULL;
    size_t buf_len = 0;
    // Generate HTML
    char table_rows[500];
    wifi_ap_record_t *wifi_info_records = wifi_scan();
    uint16_t num_ap_records = sizeof(wifi_info_records);
    memset(table_rows, 0, sizeof(table_rows));
    for (size_t i = 0; i < num_ap_records; i++)
    {
        char row[100];
        snprintf(row, sizeof(row), " <tr><td>%s</td><td>%d</td><td>%d</td></tr> \n",
                 wifi_info_records[i].ssid, wifi_info_records[i].primary, wifi_info_records[i].rssi);
        strcat(table_rows, row);
    }
    buf_len = snprintf(NULL, 0, html_template, table_rows) + 1;
    buf = (char *)malloc(buf_len);
    snprintf(buf, buf_len, html_template, table_rows);
    // send HTTP response with the generated HTML page
    httpd_resp_send(req, buf, strlen(buf));
    // free memory
    free(buf);
    return ESP_OK;
}
// HTTP server URI handler structure
httpd_uri_t get_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL,
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    // start the server
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // register URI Handlers
        httpd_register_uri_handler(server, &get_uri);
    }
    ESP_LOGE(TAG, "Error starting server !");
    return NULL;
}
esp_err_t stop_webserver(httpd_handle_t server)
{
    return httpd_stop(server);
}
void disconnect_ap_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

// using event wifi
void connect_ap_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}
void wifi_init_softap(void)
{

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    if (strlen(WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}
void app_main(void)
{
    // Initialize NVS
    httpd_handle_t server = NULL;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_ap_handler, &server));
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_ap_handler, &server));
}
