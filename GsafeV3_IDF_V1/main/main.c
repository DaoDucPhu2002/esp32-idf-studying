#include "main.h"
#include "peripherals.h"
#include "network_user.h"
#include "mqtt.h"
#define topic_mqtt "/DaoDucPhu"
extern uint8_t data_send[numBytes];
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    InitIO();
    //  wifi_manager();
    //  mqtt_app_start();

    xTaskCreate(taskStatus, "Check Status", 20000, NULL, 1, NULL);
    // uint16_t task_count_2;
    // while (1)
    // {
    //     /*check OTA*/
    //   //  mqtt_publish_data(topic_mqtt, (char *)data_send);
    //   //  vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
}