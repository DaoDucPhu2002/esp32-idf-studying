#include "wifi_config.h"
#include "webserver.h"
/*Wifi Info Access Point Mode*/
#define AP_SSID "DaoDucPhu"
#define AP_PASS "12345678"
#define AP_MAX_CONN 4
#define AP_CHANNEL 1
/*AP record*/
wifi_ap_record_t *wifi_info;
uint16_t ap_count;
/*retry sta num*/
static int retry_num = 0;
#define MAXIMUM_RETRY 10

/*Provision type variable */
provisioned_type_t provasion_type = PROVISIONED_ACCESS_POINT;
/*Event Group Handler*/
static EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const int HTTP_POST_DONE_BIT = BIT2;
static const int WIFI_SCAN_DONE_BIT = BIT3;

static const char *TAG = "WIFI_MANAGER";
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
    char json_string[200];
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
        if (retry_num < MAXIMUM_RETRY)
        {
            esp_wifi_connect();

            retry_num++;
        }
        else
        {
            ESP_LOGI(TAG, "Connected wifi is false!!!");
            ESP_LOGI(TAG, "Erase wifi data form flash");
            esp_wifi_restore();
            ESP_LOGI(TAG, "The device will reboot after 5 seconds...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            esp_restart();
        }
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        retry_num = 0;
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
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
    ESP_LOGI(TAG, "Found %d Wi-Fi networks:", ap_count);
    for (int i = 0; i < ap_count; i++)
    {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", wifi_info[i].ssid, wifi_info[i].rssi);
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

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             AP_SSID, AP_PASS, AP_CHANNEL);
}
void wifi_manager(void)
{
    bool provisioned = is_provisioned();
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

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
        if (provasion_type == PROVISIONED_SMART_CONFIG)
        {

            ESP_ERROR_CHECK(esp_wifi_start());
            ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
            smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
            xEventGroupWaitBits(s_wifi_event_group, ESPTOUCH_DONE_BIT, false, true, portMAX_DELAY);

            /*Smart Config Done*/
            ESP_LOGI(TAG, "smartconfig over");
        }
        else if (provasion_type == PROVISIONED_ACCESS_POINT)
        {
            // scan wifi
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
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, " Connected wifi");
}
