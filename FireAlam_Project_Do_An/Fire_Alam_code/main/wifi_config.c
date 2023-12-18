#include "wifi_config.h"
#include "webserver.h"
#include "main.h"
#include "mqtt.h"

#define SMART_CONFIG 0
int is_wifi_connect;
/*Wifi Info Access Point Mode*/
#define AP_SSID "FireAlam_Nhom3"
#define AP_PASS "12345678"
#define AP_MAX_CONN 4
#define AP_CHANNEL 1

#define CONFIG_BUTTON 0
/*AP record*/
wifi_ap_record_t *wifi_info;
uint16_t ap_count;
/*retry sta num*/
static int retry_num = 0;
#define MAXIMUM_RETRY 10

/*Event Group Handler*/
static EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;
// static const int ESPTOUCH_DONE_BIT = BIT1;
static const int HTTP_POST_DONE_BIT = BIT2;
static const int WIFI_SCAN_DONE_BIT = BIT3;
static const int WIFI_CONNECT_FAIL_BIT = BIT4;

static const char *TAG = "WIFI_MANAGER";
/*ssid pass post handle*/
char ssid_data[64];
char pass_data[32];
/*http server */
static httpd_handle_t server = NULL;
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
    char json_string[500];
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
/*=================================
              EVENT LOOP
==================================*/
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_num < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            retry_num++;
        }
        else
        {
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECT_FAIL_BIT);
        }
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        // return connected
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }
// Event_smartconfig
#if SMART_CONFIG
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan Done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found Channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        // Function Handler got ip and password
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        ESP_LOGI(TAG, "Connectting wifi");
    }

#endif
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        server = start_webserver();
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}
/*scan wifi*/
void wifi_scan()
{
    esp_wifi_start();
    esp_wifi_scan_start(NULL, true);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_wifi_scan_get_ap_num(&ap_count);
    wifi_info = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, wifi_info);
    ESP_LOGI(TAG, "Found %d Wi-Fi networks:", ap_count);
    for (int i = 0; i < ap_count; i++)
    {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", wifi_info[i].ssid, wifi_info[i].rssi);
    }
    xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);
}
/*Check AP in flash*/
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
/*AP MODE AND START WEBSERVER*/
void wifi_manager_ap_mode()
{

    wifi_scan();
    xEventGroupWaitBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT, false, true, portMAX_DELAY);

    http_get_wifi_infor_callback(get_info_wifi);
    http_post_set_callback(wifi_data_callback);
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

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             AP_SSID, AP_PASS, AP_CHANNEL);
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
    free(wifi_info);
}
void wifi_manager(void *pvParameter)
{
    // Config GPIO
    gpio_config_t io_config;
    io_config.pin_bit_mask = (1ULL << CONFIG_BUTTON);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_config);

    bool provisioned = is_provisioned();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
#if SMART_CONFIG
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
#endif
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
// bat AP MODE
#if SMART_CONFIG
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
        // xEventGroupWaitBits(s_wifi_event_group, ESPTOUCH_DONE_BIT, false, true, portMAX_DELAY);

        /*Smart Config Done*/
        ESP_LOGI(TAG, "smartconfig over");
#endif
        if (SMART_CONFIG == 0)
        {
            wifi_manager_ap_mode();
        }
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_start());
        mqtt_app_start();
    }
    uint32_t time_press = 0;
    for (;;)
    {

        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | WIFI_CONNECT_FAIL_BIT, true, false, 0);
        if (bits & CONNECTED_BIT)
        {
            is_wifi_connect = 1;
        }
        if (bits & WIFI_CONNECT_FAIL_BIT)
        {
            is_wifi_connect = 0;
        }
        if (gpio_get_level(BTN_WF_CONF) == 0)
        {
            time_press++;
            printf("Button: Press %ld \n", time_press);
        }
        else
        {
            time_press = 0;
        }
        if (time_press > 30)
        {
            printf("Button: AP Mode \n");
            wifi_manager_ap_mode();
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
int is_connected_to_wifi()
{
    return is_wifi_connect;
}

int8_t GetRSSI_Wifi()
{
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        return ap_info.rssi;
    }
    return 0;
}