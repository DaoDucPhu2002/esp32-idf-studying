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

#define TAG "Wifi Manager"
#define EEPROM_NAMESPACE "DATA WIFI"
/*=======================================================
                    Event Group
=======================================================*/
EventGroupHandle_t wifi_manager_events;
const int WIFI_MANAGER_READY = BIT0;           /*Read EPPROM and Check if the EPPROM is Empty*/
const int WIFI_MANAGER_SET_AP_MODE = BIT1;     /*Set AP mode*/
const int WIFI_MANAGER_SET_STA_MODE = BIT2;    /*Set STA mode*/
const int WIFI_MANAGER_STA_CONN = BIT3;        /*STA connected*/
const int WIFI_MANAGER_STA_FAIL = BIT4;        /*STA Connect Fail*/
const int WIFI_MANAGER_SCAN_MODE = BIT5;       /*Scan mode is on*/
const int WIFI_MANAGER_SCAN_DONE = BIT6;       /*Scan done*/
const int WIFI_MANAGER_WEBSERVER_START = BIT7; /*Webserver start*/
/*Struct sta wifi config*/
typedef struct sta_ssid_pass_t
{
    char wifi_ssid[32];
    char wifi_password[64];
} sta_ssid_pass_t;

#define EXAMPLE_ESP_WIFI_SSID "DaoDucPhu"
#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4
#define MAXIMUM 20
#define WIFI_SSID "Ba Anh Em"
#define WIFI_PASS "11111111"
#define EXAMPLE_ESP_MAXIMUM_RETRY 10
typedef struct scan_ssid_pass_t
{
    wifi_ap_record_t *ap_record;
    uint8_t ap_count;
} scan_ssid_pass_t;
static int s_retry_num = 0;

static void wifi_ap_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
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
            // xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        //    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_manager_events, WIFI_MANAGER_STA_CONN);
    }
}

static void wifi_sta_config()
{

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

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

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,

        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

static void wifi_ap_config()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_ap_handler, NULL, NULL));
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
    // set bit WIFI_MANAGER_SET_AP_MODE
    xEventGroupSetBits(wifi_manager_events, WIFI_MANAGER_WEBSERVER_START);
}

static scan_ssid_pass_t wifi_scan_config()
{
    scan_ssid_pass_t scan_done;
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    uint16_t temp = MAXIMUM;
    wifi_ap_record_t ap_info[MAXIMUM];
    uint16_t ap_count;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&temp, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    scan_done.ap_count = ap_count;
    scan_done.ap_record = ap_info;
    for (int i = 0; (i < MAXIMUM) && (i < ap_count); i++)
    {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }
    // set bit scan done
    xEventGroupSetBits(wifi_manager_events, WIFI_MANAGER_SCAN_DONE);
    return scan_done;
}
/*EPPROM Handle*/
int8_t *read_data_form_eeprom(uint8_t *data_key)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(EEPROM_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "EEPROM: Failed to open NVS Handler.Error:%s", esp_err_to_name(err));
        return;
    }
    int8_t *data = NULL;
    size_t required_size;
    err = nvs_ get_str(nvs_handle, data_key, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "EEPROM: Data not found in EEPROM");
        return;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to read data from EEPROM. Error: %s", esp_err_to_name(err));
        return;
    }
    data = malloc(required_size);
    if (data == NULL)
    {
        ESP_LOGI(TAG, "EEPROM: Memory allocation failed");
        return;
    }
    err = nvs_get_str(nvs_handle, data_key, data, &required_size);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to read data from EEPROM. Error: %s", esp_err_to_name(err));
        free(data);
        return;
    }
    ESP_LOGI(TAG, "Data read form EEPROM: %s", data);
    nvs_close(nvs_handle);
    return data;
}

void write_data_form_eeprom(int8_t *data_key, int8_t *data)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        // Error dose not find the key
        ESP_LOGI(TAG, "EEPROM: Data not found in EEPROM");
        xEventGroupSetBits(wifi_manager_events, WIFI_MANAGER_SET_AP_MODE);
        return;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to write data from EEPROM. Error: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(nvs_handle, data_key, data);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to write data to EEPROM. Error: %s\n", esp_err_to_name(err));
        return;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to commit data to EEPROM. Error: %s\n", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Data written to EEPROM successfully\n");
    // Set Sta connect wifi
    xEventGroupSetBits(wifi_manager_events, WIFI_MANAGER_SET_STA_MODE);
    nvs_close(nvs_handle);
}
esp_err_t wifi_manager()
{
    wifi_scan_config();
}
/*============================================
                    webserver
==============================================*/
esp_err_t wifi_list_ap_handler(httpd_req_t *req)
{
    if (strcmp(req->uri, "/wifi-list"))
    {
    }
}
void app_main(void)
{
    wifi_manager_events = xEventGroupCreate();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_sta_config();

    // /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    //  * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    // EventBits_t bits = xEventGroupWaitBits(wifi_manager_events,
    //                                        ,
    //                                        pdFALSE,
    //                                        pdFALSE,
    //                                        portMAX_DELAY);

    // if (bits & WIFI_CONNECTED_BIT)
    // {
    //     ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
    //              EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    // }
    // else if (bits & WIFI_FAIL_BIT)
    // {
    //     ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
    //              EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "UNEXPECTED EVENT");
    // }
}
