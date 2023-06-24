#include <stdio.h>
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
//==============================================================
//              scan wifi
//==============================================================
#define DEFAULT_SCAN_LIST_SIZE 10
wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
uint16_t ap_count = 0;
void wifi_scan(void)
{

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(Tag, "Total APs scanned = %u", ap_count);

    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
    {
        ESP_LOGI(Tag, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(Tag, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(Tag, "Channel \t\t%d\n", ap_info[i].primary);
    }
}
/*========================  Wifi config*   ====================*/
#define WIFI_SSID "Dao Duc Phu"
#define WIFI_PASS "00000000"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 4

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        // scan mode

        ESP_LOGI(Tag, "sta start");

        break;
    case WIFI_EVENT_SCAN_DONE:
        // scan co wifi trong eeprom => ket noi => o che do sta
        // scan khong co trong eeprom => return danh sach =>stop => che do ap
        break;
    case WIFI_EVENT_STA_STOP:
        // start AP
        break;
    case WIFI_EVENT_AP_STACONNECTED:
        // webserver true => ket noi wifi
        //  true => sta mode
        //  false => sta mode scan wifi
        break;
    case WIFI_EVENT_AP_STADISCONNECTED:

        break;
    default:
        break;
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
/*=======================================================
                web handle
========================================================*/
// html file

char *generate_table()
{
    wifi_scan();
    static char table[1000];
    memset(table, 0, sizeof(table));
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
    {
        char row[100];
        snprintf(row, sizeof(row), "<tr><td>%d</td><td>%s</td><td>%d</td><a href = \"#\"></a></tr>", i, ap_info[i].ssid, ap_info[i].rssi);
        strcat(table, row);
    }
    return table;
}

//=======================================================
esp_err_t http_scan_wifi_handle(httpd_req_t *req)
{
    const char html_scan_wifi[2000];
    snprintf(&html_scan_wifi, sizeof(html_scan_wifi),
             "<html><body><h1>Data Table</h1>"
             "<table>"
             "<tr><th>Index</th><th>SSID</th><th>Strength</th></tr>"
             "%s"
             "</table></body></html>",
             generate_table());
    httpd_resp_send(req, html_scan_wifi, strlen(html_scan_wifi));
    return ESP_OK;
}
httpd_uri_t scan_wifi_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = http_scan_wifi_handle,
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
        httpd_register_uri_handler(&server, &scan_wifi_uri);
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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    ESP_LOGI(Tag, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
}
