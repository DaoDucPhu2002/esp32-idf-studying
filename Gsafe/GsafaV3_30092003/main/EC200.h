#ifndef __EC200_H
#define __EC200_H

#include <stdio.h>
#include "string.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "FreeRTOS/event_groups.h"
#include "esp_timer.h"

#define PWRKEY_PRESS() gpio_set_level(PWRKEY_GPIO, 1)
#define PWRKEY_RELEASE() gpio_set_level(PWRKEY_GPIO, 0)
#define GetTick() esp_timer_get_time() / 1000ULL
void init_4G();
bool power_on();
bool power_off();
bool restart_module();
char *GetIMEI();
int8_t GetRSSI();

uint8_t MQTT_setup(char *esp_id, char *topic_srv, char *mqqtt_data);
int MQTT_Close();
int MQTT_Open(char *mqtt_server, int mqtt_port, char *esp_id, char *topic_srv, char *user, char *passwd);
int MQTT_Publish(char *subcribe_srv, char *topic_srv, char *mqqtt_data, unsigned int length);

#endif