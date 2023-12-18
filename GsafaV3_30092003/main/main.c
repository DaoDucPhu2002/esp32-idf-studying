#include "main.h"
#include "esp_smartconfig.h"
#include "wifi_config.h"
#include "mqtt.h"
#include "ota_app.h"
#include "EC200.h"

uint32_t numSendData;

uint8_t dataload[58];
uint8_t numBytes = 58;
char IMEI[15];
char ESP_ID[13];
/*count loop*/
uint64_t count_loop;
/*time update OTA*/
uint16_t time_check_ota = 5 * 60; // 5p
uint16_t time_send_data_to_mqtt = 1;
uint16_t time_send_data_to_4G = 5;
/*Variable*/
uint8_t is_connect_wifi = 0;
uint8_t is_connect_mqtt_4G = 0;
uint8_t is_connect_mqtt = 0;

/*Server MQTT*/
const char *mqtt_hostname = "test.api.lctech.vn";
const uint32_t mqtt_port = 1884;
const char *mqtt_user = "lctech_rmq";
const char *mqtt_pass = "lctechrmq123@";
// const char *mqtt_hostname_test = "test.mosquitto.org";
const char *topic_server = "mmm/GSafe_v3/data";
char txt_subscribe[] = "mmm/123456789012/cmd";
char topicSever4G[] = "mmm/GSafe_v3_4g/data";

unsigned long millis()
{
    return (unsigned long)esp_timer_get_time() / 1000ULL;
}
void data_process()
{
    uint16_t index = 0;
    uint8_t line;
    uint32_t time1s = millis() / 1000;
    dataload[index++] = numBytes; // 0
    for (line = 0; line < 12; line++)
        dataload[index++] = ESP_ID[line];
    for (line = 0; line < 15; line++)
        dataload[index++] = IMEI[line];
    dataload[index++] = GetRSSI_Wifi(); // 28
    dataload[index++] = 10;             // GetRSSI();      // 29
    dataload[index++] = 1;              // 30

    dataload[index++] = 1;  // 31// bat percent
    dataload[index++] = 1;  // 32//Status
    dataload[index++] = 16; // 33//NumLine
    for (line = 0; line < 16; line++)
    {
        dataload[index++] = 2;
    }
    dataload[index++] = (numSendData >> 24) & 0xFF; // 48
    dataload[index++] = (numSendData >> 16) & 0xFF; // 49
    dataload[index++] = (numSendData >> 8) & 0xFF;  // 50
    dataload[index++] = (numSendData) & 0xFF;       // 51 numSendData

    dataload[index++] = (time1s >> 24) & 0xFF; // 52
    dataload[index++] = (time1s >> 16) & 0xFF; // 53
    dataload[index++] = (time1s >> 8) & 0xFF;  // 54
    dataload[index++] = (time1s) & 0xFF;       // 55
    printf("process done");
}

void app_main(void)
{

    init_4G();
    readMAC();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    // bool is_power_on = power_on();
    // if (!is_connect_wifi)
    // {
    //     is_connect_wifi = wifi_manager();
    // }
    // if (is_connect_wifi && !is_connect_mqtt)
    // {
    //     mqtt_app_start();
    //     printf("MQTT_Connected\n");
    //     is_connect_mqtt = 1;
    // }

    while (1)
    {
        printf("HELLO");
        vTaskDelay(pdMS_TO_TICKS(10));
        data_process();

        MQTT_Open(mqtt_hostname, mqtt_port, ESP_ID, topicSever4G, mqtt_user, mqtt_pass);
        if (is_connect_wifi && count_loop % time_check_ota == 0)
        {
            // start_ota();
        }
        /*Send data*/

        if (is_connect_wifi && count_loop % time_send_data_to_mqtt == 0)
        {
            send_data_to_server(dataload);
        }
        /*send data to 4G*/
        // if (is_power_on)
        // {
        //     if (count_loop % time_send_data_to_4G == 0 || !is_connect_wifi)
        //     {
        //         MQTT_Publish(txt_subscribe, topicSever4G, (char *)dataload, numBytes);
        //     }
        // }
        // else
        // {
        //     ESP_LOGW("4G", "Not conneted");
        // }
        numSendData++;
        count_loop++;
        vTaskDelay(pdMS_TO_TICKS(10)); // Use FreeRTOS macro to convert milliseconds to ticks
    }
}
