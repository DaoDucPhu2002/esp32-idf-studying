#include "network_user.h"
#define TAG_WF_CFG "Wifi Config"
/* ==================================
            WiFi config
=====================================*/

/*Wifi connect retry*/
int s_retry_num = 0;
/*AP record*/
wifi_ap_record_t *wifi_info;
uint16_t ap_count;
/*Event Group Handler*/
static EventGroupHandle_t s_wifi_event_group;
/*ssid pass post handle*/
char ssid_data[64];
char pass_data[32];
/*Post ssid and pass*/

void wifi_data_callback(char *data, int len)
{
    printf("%.*s\n", len, data);
    char *pt = strtok(data, "/");
    strcpy(ssid_data, pt);
    pt = strtok(NULL, "/");
    strcpy(pass_data, pt);
    printf("ssid: %s, pwd: %s\n", ssid_data, pass_data);
    // xEventGroupSetBits(xCreatedEventGroup, WIFI_RECV_INFO);
    xEventGroupSetBits(s_wifi_event_group, HTTP_POST_DONE_BIT);
}
/*Get wifi info */
void get_info_wifi(char *data, int len)
{
    char json_string[1000];
    snprintf(json_string, sizeof(json_string), "[");
    for (int i = 0; i < ap_count; i++)
    {

        char temp[100];
        snprintf(temp, sizeof(temp), "{\"ssid\":\"%s\",\"rssi\":%d}", wifi_info[i].ssid, wifi_info[i].rssi);
        strncat(json_string, temp, sizeof(json_string) - strlen(json_string) - 1);

        if (i < ap_count - 1)
        {
            strncat(json_string, ",", sizeof(json_string) - strlen(json_string) - 1);
        }
    }
    strncat(json_string, "]", sizeof(json_string) - strlen(json_string) - 1);

    printf("Chuoi: %s\n", json_string);
    size_t buf_len = strlen(json_string);
    http_send_response(json_string, buf_len);
}
/*Event Handle*/

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
            xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);

            LOG_DEBUG(TAG_WF_CFG, "retry to connect to the AP");
        }
        else
        {
            LOG_DEBUG(TAG_WF_CFG, "Connected wifi is false!!!");
            LOG_DEBUG(TAG_WF_CFG, "Erase wifi data form flash");
            esp_wifi_restore();
            LOG_DEBUG(TAG_WF_CFG, "The device will reboot after 5 seconds...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            esp_restart();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_retry_num = 0;
        LOG_DEBUG(TAG_WF_CFG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }

    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        LOG_DEBUG(TAG_WF_CFG, "station " MACSTR " join, AID=%d",
                  MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        LOG_DEBUG(TAG_WF_CFG, "station " MACSTR " leave, AID=%d",
                  MAC2STR(event->mac), event->aid);
    }
}

bool is_provisioned(void)
{
    bool provisioned = false;
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    assert(sta_netif);

    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    /*Wifi Config Variable*/
    wifi_config_t wifi_config;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
    /*Check Info*/
    if (wifi_config.sta.ssid[0] != 0)
    {
        provisioned = true;
    }
    return provisioned;
}
/*Function Scan wifi handle*/
void wifi_scan()
{
    esp_wifi_start();
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_wifi_scan_get_ap_num(&ap_count);
    wifi_info = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, wifi_info);
    LOG_DEBUG(TAG_WF_CFG, "Found %d Wi-Fi networks:", ap_count);
    for (int i = 0; i < ap_count; i++)
    {
        LOG_DEBUG(TAG_WF_CFG, "SSID: %s, RSSI: %d", wifi_info[i].ssid, wifi_info[i].rssi);
    }
    xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);
}
void wifi_ap_config()
{
    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASS,
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    LOG_DEBUG(TAG_WF_CFG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
              AP_SSID, AP_PASS, AP_CHANNEL);
}
void wifi_manager(void)
{
    bool provisioned = is_provisioned();
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

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

    if (!provisioned)
    {

        wifi_scan();
        xEventGroupWaitBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT, false, true, portMAX_DELAY);
        http_get_wifi_infor_callback(get_info_wifi);
        http_post_set_callback(wifi_data_callback);
        wifi_ap_config();
        server = start_webserver();
        xEventGroupWaitBits(s_wifi_event_group, HTTP_POST_DONE_BIT, false, true, portMAX_DELAY);

        wifi_config_t wifi_config;
        bzero(&wifi_config, sizeof(wifi_config));
        memcpy(wifi_config.sta.ssid, ssid_data, strlen(ssid_data));
        memcpy(wifi_config.sta.password, pass_data, strlen(pass_data));
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    LOG_DEBUG(TAG_WF_CFG, " Connected wifi");
}
/* =============================================
                    Webserver
===============================================*/

httpd_handle_t server = NULL;
#define TAG "WebServer"
/*Handle Callback*/
static http_server_handle http_get_handle = NULL;
static http_server_handle http_post_handle = NULL;
static http_server_handle http_get_info_handle = NULL;

/*Extern HTML File */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/*URL Varialbe*/
char url_data[100];

/*=======================================================
                        Webserver Handle
=========================================================*/
static httpd_req_t *get_req;
void http_send_response(char *data, int len)
{
    httpd_resp_send(get_req, (const char *)data, len);
}
/*HTTP GET Handler*/
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
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
    //  const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}

static const httpd_uri_t home = {
    .uri = "/home",
    .method = HTTP_GET,
    .handler = wifi_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = NULL};

/* An HTTP POST handler */
static esp_err_t post_handler(httpd_req_t *req)
{
    char buf[100];
    int len = httpd_req_recv(req, buf, 100);
    http_post_handle(buf, len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t wifi = {
    .uri = "/wifi_get_pass",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};
/*GET Wifi_infor*/
static esp_err_t get_wifi_info(httpd_req_t *req)
{
    get_req = req;
    http_get_info_handle("Wifi get info", 14);
    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}
static const httpd_uri_t get_wifi = {
    .uri = "/wifi_info",
    .method = HTTP_GET,
    .handler = get_wifi_info,
    .user_ctx = NULL};
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/home", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    else if (strcmp("/wifi_get_pass", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

httpd_handle_t start_webserver(void)
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
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &wifi);
        httpd_register_uri_handler(server, &get_wifi);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void http_get_set_callback(void *cb)
{
    if (cb)
    {
        http_get_handle = cb;
    }
}

void http_post_set_callback(void *cb)
{
    if (cb)
    {
        http_post_handle = cb;
    }
}
void http_get_wifi_infor_callback(void *cb)
{
    if (cb)
    {
        http_get_info_handle = cb;
    }
}
/*=================================
                EC200
===================================*/