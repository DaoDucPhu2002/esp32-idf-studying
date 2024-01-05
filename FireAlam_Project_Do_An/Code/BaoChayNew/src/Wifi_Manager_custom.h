#ifndef WIFI_MANAGWE_h
#define WIFI_MANAGWE_h
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

void initSPIFFS();
String readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
bool initWiFi();
String processor(const String &var);
void wifimanager(void *pvParameter);
void MQTT_Send(uint8_t *data);
int wifi_getRSSI();
#endif // WIFI_MANAGWE_h
