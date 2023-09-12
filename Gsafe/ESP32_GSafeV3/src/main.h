#ifndef MAIN_h
#define MAIN_h

#include <Arduino.h>
#include "HWConfig.h"
#include "TEE_EC200.h"
// #include <esp_task_wdt.h>

#define RXD2      (16)
#define TXD2      (17)

#define TIME_MQTT  1000*20

extern EC200 gsm;
extern bool in_smartconfig;
extern struct gsafe_msg_data gdata ;
extern uint8_t checkBoard;
extern uint16_t numTimeSend4G;

#endif