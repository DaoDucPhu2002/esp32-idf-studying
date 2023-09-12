#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include "esp_netif.h"

#include "esp_tls_crypto.h"
#include <esp_http_server.h>

#define TAG "Wifi Manager"
#define EEPROM_NAMESPACE "DATA WIFI"

#define EXAMPLE_ESP_WIFI_SSID "DaoDucPhu"
#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4

#define EXAMPLE_ESP_MAXIMUM_RETRY 5
/*URL Varialbe*/
char url_data[100];

/*Define event group to STA Mode*/
static EventGroupHandle_t wifi_sta_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
void wifi_sta_config(char *ssid, char *pass);
/*=======================================================
                        String Handle
=========================================================*/
static void
url_decoder(char *str)
{
    char *pstr = str;
    char *buf = str;
    while (*pstr)
    {
        if (*pstr == '+')
        {
            *buf++ = ' ';
        }
        else
        {
            *buf++ = *pstr;
        }
        pstr++;
    }
    *buf = '\0';
}
char *get_ssid(char *str)
{
    char *ssid_ptr = strstr(str, "ssid=");
    if (ssid_ptr != NULL)
    {
        ssid_ptr += strlen("ssid=");

        // Tìm vị trí kết thúc của chuỗi SSID
        char *ssid_end = strchr(ssid_ptr, '&');
        if (ssid_end != NULL)
        {
            // Tính toán độ dài của chuỗi SSID
            int ssid_length = ssid_end - ssid_ptr;

            // Cấp phát bộ nhớ động cho chuỗi SSID và sao chép giá trị
            char *ssid = malloc((ssid_length + 1) * sizeof(char));
            strncpy(ssid, ssid_ptr, ssid_length);
            ssid[ssid_length] = '\0';

            // Giải mã URL (nếu cần)
            url_decoder(ssid);

            return ssid;
        }
    }

    return NULL; // Trường hợp không tìm thấy SSID
}

char *get_password(char *str)
{
    char *password_ptr = strstr(str, "password=");
    if (password_ptr != NULL)
    {
        password_ptr += strlen("password=");

        // Đếm độ dài của password
        int password_length = 0;
        char *password_end = password_ptr;
        while (*password_end != '\0' && *password_end != '&')
        {
            password_end++;
            password_length++;
        }

        // Cấp phát bộ nhớ động cho chuỗi password và sao chép giá trị
        char *password = malloc((password_length + 1) * sizeof(char));
        strncpy(password, password_ptr, password_length);
        password[password_length] = '\0';

        return password;
    }

    return NULL; // Trường hợp không tìm thấy password
}

/*=======================================================
                        Webserver config
=========================================================*/
/*HTTP GET Handler*/
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Location") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Location", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Location: %s", buf);
        }
        free(buf);
    }
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = wifi_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = "<html><body> "
                "<h2>Web Password Page</h2>"
                "<form action='/echo' method='post'>"
                "SSID: <input type='text' name='ssid'><br>"
                "Password: <input type='password' name='password'><br>"
                "<input type='submit' value='Connect'>"
                "</form></body></html> "};

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */

        sprintf(url_data, "%.*s", ret, buf);
        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        char *ssid_data = get_ssid(url_data);
        char *pass_data = get_password(url_data);
        ESP_LOGI(TAG, "SSID = %s", ssid_data);
        ESP_LOGI(TAG, "Pass = %s", pass_data);
        ESP_LOGI(TAG, "====================================");
        if (ssid_data != NULL)
        {
            wifi_sta_config(ssid_data, pass_data);
            ESP_LOGI(TAG, "Conncet to %s ", ssid_data);
            EventBits_t bits = xEventGroupWaitBits(wifi_sta_event_group,
                                                   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                                   pdFALSE,
                                                   pdFALSE,
                                                   portMAX_DELAY);

            if (bits & WIFI_CONNECTED_BIT)
            {
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                         EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
            }
            else if (bits & WIFI_FAIL_BIT)
            {
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                         EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
            }
            else
            {
                ESP_LOGE(TAG, "UNEXPECTED EVENT");
            }
        }
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri = "/echo",
    .method = HTTP_POST,
    .handler = echo_post_handler,
    .user_ctx = NULL};
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    else if (strcmp("/echo", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
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

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

/*=======================================================
                        Wifi config
=========================================================*/
void wifi_ap_config()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_ap_cfg = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },

    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_cfg));

    esp_wifi_start();
    ESP_LOGI(TAG, "wifi init soft ap finished. SSID:%s password: %s ", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

/*=======================================================
                        Connect STA
=========================================================*/

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(wifi_sta_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_sta_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_sta_config(char *ssid, char *pass)
{
    wifi_sta_event_group = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);
    wifi_config_t ap_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
  

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(wifi_sta_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
}
void app_main(void)
{

    static httpd_handle_t server = NULL;
    wifi_ap_config();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    /* Start the server for the first time */
    server = start_webserver();
}
