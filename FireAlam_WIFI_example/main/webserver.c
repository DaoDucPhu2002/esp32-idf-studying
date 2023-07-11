#include "webserver.h"

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
    wifi_ap_record_t wifi_info_records[] = wifi_scan();
    uint16_t num_ap_records = sizeof(wifi_info_records) / sizeof(wifi_info_records[0]);
    memset(table_rows, 0, sizeof(table_rows));
    for (size_t i = 0; i < num_ap_records; i++)
    {
        char row[100];
        snprintf(row, sizeof(row), "<tr><td>%s</td><td>%d</td><td>%d</td><a href=\" # \">connect</a></tr>",
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
void connect_ap_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

// using event wifi