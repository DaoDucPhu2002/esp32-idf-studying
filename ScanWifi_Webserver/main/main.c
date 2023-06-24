#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <sys/param.h>
#include "esp_netif.h"

#include "esp_tls_crypto.h"
#include <esp_http_server.h>

#define Tag "Web Server"

/*==============================================
            Wifi config
=============================================*/
#define WIFI_SSID "Dao Duc Phu"
#define WIFI_PASS "00000000"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 4

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(Tag, "station " MACSTR "join, AID = %d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(Tag, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    wifi_config_t wifi_ap = {
        .ap = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .channel = WIFI_CHANNEL,
            .ssid_len = strlen(WIFI_SSID),
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg =
                {
                    .required = false,
                },
        },
    };
    if (strlen(WIFI_PASS) == 0)
    {
        wifi_ap.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(Tag, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}

/* ====================================================
            Webserver handle
========================================================*/
const char *html_content = "<html><body><h1>Hello from ESP-IDF web server!</h1>"
                           "<form method=\"POST\">"
                           "<input type=\"text\" name=\"input_text\">"
                           "<input type=\"submit\" value=\"Submit\">"
                           "</form>"
                           "</body></html>";
esp_err_t http_index_handle(httpd_req_t *req)
{
    httpd_resp_send(req, html_content, strlen(html_content));
    return ESP_OK;
}
httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = http_index_handle,
    .user_ctx = NULL,

};

esp_err_t root_post_handle(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        ret = httpd_req_recv(req, buf, sizeof(buf));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                continue;
            return ESP_FAIL;
        }
        remaining -= ret;
    }

    // Extract the value of the input_text field
    char *input_value = NULL;
    char *input_name = "input_text";
    int value_len = httpd_req_get_url_query_len(req) + 1;
    if (value_len > 1)
    {
        input_value = malloc(value_len);
        if (httpd_req_get_url_query_str(req, input_value, value_len) == ESP_OK)
        {
            char *value_start = strstr(input_value, input_name);
            if (value_start)
            {
                char *value_end = strchr(value_start, '&');
                if (!value_end)
                    value_end = input_value + value_len - 1;
                *value_end = '\0';
                input_value = value_start + strlen(input_name) + 1;
            }
            else
            {
                free(input_value);
                input_value = NULL;
            }
        }
    }

    // Log the received input value
    ESP_LOGI(Tag, "Received input: %s", input_value ? input_value : "(null)");

    // Send a response back to the client
    const char *response = "Input received!";
    httpd_resp_send(req, response, strlen(response));

    if (input_value)
        free(input_value);

    return ESP_OK;
}

httpd_uri_t post_uri =
    {
        .uri = "/",
        .method = HTTP_POST,
        .handler = root_post_handle,
        .user_ctx = NULL,
};
//======================================================
//          Webserver config
//======================================================
httpd_handle_t webserver_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // Start the Server
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // gui len server index uri
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &post_uri);
        ESP_LOGI(Tag, "Web server is succesfully.");

        return server;
    }
    ESP_LOGI(Tag, "Error Starting Web Server!");
    return NULL;
}
static esp_err_t webserver_stop(httpd_handle_t server)
{
    httpd_stop(server);
    ESP_LOGI(Tag, "Webserver is Stop. ");
    return ESP_OK;
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(Tag, "Stopping webserver");
        if (webserver_stop(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(Tag, "Failed to stop http server");
        }
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(Tag, "Starting webserver");
        *server = webserver_start();
    }
}

void app_main(void)
{
    static httpd_handle_t server = NULL;
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();

    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server);

    ESP_LOGI(Tag, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
}
