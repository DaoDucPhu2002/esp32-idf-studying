#ifndef HWConfig_h
#define HWConfig_h

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <PubSubClient.h>
#include <time.h>
#include <Ticker.h>
#include <esp_task_wdt.h>
#include <FreeRTOSConfig.h>
// #include <WiFi.h>
#include <WebServer.h>
// #define accessPointButtonPin 4 // Connect a button to this pin

#define DEBUG 0

#define BUZZER_EN 1

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
#endif

// #define _TIMERINTERRUPT_LOGLEVEL_     1

#define LED_D2 2
// #define accessPointLed        LED_D2

#define PIN_BUTTON 0
#define accessPointButtonPin PIN_BUTTON

#define VBAT_FB 39
#define CH_AC 36
#define KEY_IN 35

#define KEY_MODE 1
#define KEY_BELL 2
#define KEY_CHECK 3
#define KEY_M_B 4
#define KEY_M_C 5
#define KEY_B_C 6
#define KEY_M_B_C 7

#define RF_REMOTE 34

#define INSEN 22
#define EOLINE 23
#define SA0 5 // 21   // 21 19 18 5
#define SA1 18
#define SA2 19 // 18
#define SA3 21 // 5

#define PWR_KEY 32 // 25
#define BOOT_EN 15
#define CHARGE 13

#define LAMP 25    // 26
#define BELL 33    // 27
#define RL_OUT1 27 // 14
#define RL_OUT2 26 // 12

#define LEDR 14
#define LEDG 12

#define BEEP 4

#define LAMP_ON digitalWrite(LAMP, HIGH)
#define LAMP_OFF digitalWrite(LAMP, LOW)
#define BELL_ON digitalWrite(BELL, HIGH)
#define BELL_OFF digitalWrite(BELL, LOW)
#define OUT1_ON digitalWrite(RL_OUT1, HIGH)
#define OUT1_OFF digitalWrite(RL_OUT1, LOW)
#define OUT2_ON digitalWrite(RL_OUT2, HIGH)
#define OUT2_OFF digitalWrite(RL_OUT2, LOW)

#define BEEP_ON digitalWrite(BEEP, HIGH)
#define BEEP_OFF digitalWrite(BEEP, LOW)
#define BEEP_TOGGLE digitalWrite(BEEP, digitalRead(BEEP) ^ 0x01)

#define VBAT_ON digitalWrite(BOOT_EN, HIGH)
#define VBAT_OFF digitalWrite(BOOT_EN, LOW)

#define CHARGE_ON digitalWrite(CHARGE, HIGH)
#define CHARGE_OFF digitalWrite(CHARGE, LOW)

#define LED_OFF() digitalWrite(LED_D2, HIGH)
#define LED_ON() digitalWrite(LED_D2, LOW)
#define LED_TOGGLE() digitalWrite(LED_D2, digitalRead(LED_D2) ^ 0x01)

#define LEDR_OFF digitalWrite(LEDR, HIGH)
#define LEDR_ON digitalWrite(LEDR, LOW)
#define LEDR_TOGGLE digitalWrite(LEDR, digitalRead(LEDR) ^ 0x01)
#define LEDG_OFF digitalWrite(LEDG, HIGH)
#define LEDG_ON digitalWrite(LEDG, LOW)
#define LEDG_TOGGLE digitalWrite(LEDG, digitalRead(LEDG) ^ 0x01)

#define ALARM 0x00
#define NOT_USE 0x01
#define ENDOFLINE 0x02
#define NORMAL 0x03

#define PW_ACIN 0x01
#define PW_BAT 0x02
#define PW_CHARG 0x04
#define BAT_OK 0x08

#define dbSerial Serial
#define Serial4G Serial2

#define numLine 16     // 3
#define byteOfLine 1   // 33 //17 33
#define byteOfHeader 7 // 13  //7 Numbyte, WRSSI, 4RSSI, BAT%, PW, Status, NumLine

#define sendID 12   // 13  //7
#define sendIMEI 15 // 13  //7
#define sendZone 8  // 13  //7
#define numBytes (numLine * byteOfLine + byteOfHeader + sendID + sendIMEI + sendZone)
// 7+16+12+15+8=58

#define Serial4G Serial2

#define GSM_NL "\r\n"
#define WIFI_SEND 20
#define accessPointLed 2
#define eepromTextVariableSize 33 // the max size of the ssid, password etc. 32+null terminated
#define eepromBufferSize 200      // have to be > eepromTextVariableSize * (eepromVariables+1) (33 * (5+1))

/* Enable these lines in case you want to change the default Access Point ip: 192.168.4.1. You have to enable the line:
WiFi.softAPConfig(local_ip, gateway, subnet);
on the void initAsAccessPoint too */

void initStart(char rewrite);
void InitIO();
// void mqtt.loop();
void tick();
char h2c(char c);
int convert_u8_hex(uint8_t *raw_data, uint8_t *conv_data, uint16_t data_length);
char *string2char(String ipString);
void debugSerial(String data);
void debugSerial(char *data);
void sendHexPC(uint8_t hex);
void sendToPC();
void sendToPCTest();
void sendTimePC(char *data, uint32_t time);
void sendTimePCln(char *data, uint32_t time);

void beepOut(uint16_t dtime);
bool longPress();

void setupWifiManager(); // Wifi manager Setup

void initAsAccessPoint();
void checkWiFiConnection();
void checkStartWifi();
void playAccessPointLed();
void checkAccessWifi();
void checkAccessMode();

void macRead();
void delayWifi(void);
void WiFi_Connect();
void mqttConnect();
void ota_update();
void sendToSever();
void smartUpdateOTA();
void wifi_mqtt_process();

void readRSSI4G();
void init_4G();
void powerKey4G();
void sendToSever4G();
void autoStatus4G();
bool read_gsm();

int key_press();
int autoPower();
void select_line(int line);
void read_sensor();
void data_process();
void checkLineStatus();
void checkAlarm();

void updateData2Sent();
void LED_Status(uint16_t cnt);
void taskStatus(void *TaskParameters_t);
void taskStatus4G(void *TaskParameters_t);
void ESP_status(uint32_t sta);
// void taskAlarm( void * TaskParameters_t );

uint8_t check_crc_data(uint8_t data[], uint8_t len);
void callback(char *topic, byte *payload, unsigned int length);
void saveSettingsToEEPPROM(char *ssid_, char *pass_);
void readSettingsFromEEPROM(char *ssid_, char *pass_);
void writeEEPROM(int startAdr, int length, char *writeString);
void readEEPROM(int startAdr, int maxLength, char *dest);
void saveStatusToEeprom(byte value);
byte getStatusFromEeprom();
void saveDebugToEeprom(byte value);
byte getDebugFromEeprom();

void GSafe_Setting_Save();
void GSafe_Setting_Read();
uint8_t read_EEPROM_Wifi();
void write_EEPROM_Wifi();
void write_EEPROM_Wifi_default();
// for checkboard mode
// void readSerialCheck();
// void checkMode();

void handle_OnConnect();
void handle_NotFound();
void handleGenericArgs();
String SendHTML(uint8_t st);

#endif