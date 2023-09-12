#include <stdio.h>
#include "string.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "FreeRTOS/event_groups.h"
#include "esp_timer.h"
#include "EC200.h"

const char mqttServer[] = "test.mosquitto.org";
const int mqttPort = 1883;
const char mqttUser[] = "lctech_rmq";
const char mqttPassword[] = "lctechrmq123@";
char topicSever4G[] = "DaoDucPhu";
char txt_subscribe[] = "DaoDucPhu";
#define ESP_ID "012345"
void app_main()
{

    init_4G();
    bool is_power_on = power_on();

    int is_connect_mqtt = 0;
    while (1)
    {

        if (is_power_on)
        {

            GetIMEI();
            GetRSSI();

            if (!is_connect_mqtt)
            {
                is_connect_mqtt = MQTT_Open(mqttServer, mqttPort, ESP_ID, topicSever4G, NULL, NULL);
            }

            MQTT_Publish(txt_subscribe, topicSever4G, "DaoDucPhu", 9);
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
      
    }
}
