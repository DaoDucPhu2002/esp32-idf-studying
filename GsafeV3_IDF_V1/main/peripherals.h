#ifndef __PERIPHERALS_H
#define __PERIPHERALS_H

#include "main.h"
#include "esp_timer.h"
/*Info Device*/
#define ESP_ID "123456789012"
#define IMEI "" // Get form module sim

/*Define Function */
#define LAMP_ON gpio_set_level(LAMP, 1)
#define LAMP_OFF gpio_set_level(LAMP, 0)
#define BELL_ON gpio_set_level(BELL, 1)
#define BELL_OFF gpio_set_level(BELL, 0)
#define RL1_ON gpio_set_level(RL1, 1)
#define RL1_OFF gpio_set_level(RL1, 0)
#define RL2_ON gpio_set_level(RL2, 1)
#define RL2_OFF gpio_set_level(RL2, 0)

#define CHARGE_ON gpio_set_level(V_BOOST, 1)
#define CHARGE_OFF gpio_set_level(V_BOOST, 0)

#define VBAT_ON gpio_set_level(VBAT_EN, 1)
#define VBAT_OFF gpio_set_level(VBAT_EN, 0)

#define LED_WF_ON gpio_set_level(LED_WF, 1)
#define LED_WF_OFF gpio_set_level(LED_WF, 0)
#define LED_WF_TOGGLE() gpio_set_level(LED_WF, !gpio_get_level(LED_WF))

#define BEEP_ON gpio_set_level(BEEP, 1)
#define BEEP_OFF gpio_set_level(BEEP, 0)
#define BEEP_TOGGLE() gpio_set_level(BEEP, !gpio_get_level(BEEP))

#define LEDR_ON gpio_set_level(LEDR, 1)
#define LEDR_OFF gpio_set_level(LEDR, 0)
#define LEDR_TOGGLE() gpio_set_level(LEDR, !gpio_get_level(LEDR))

#define LEDG_ON gpio_set_level(LEDG, 1)
#define LEDG_OFF gpio_set_level(LEDG, 0)
#define LEDG_TOGGLE() gpio_set_level(LEDG, !gpio_get_level(LEDG))

#define ALARM 0x00
#define NOT_USE 0x01
#define ENDOFLINE 0x02
#define NORMAL 0x03

#define PW_ACIN 0x01
#define PW_BAT 0x02
#define PWM_CHARG 0x04
#define BAT_OK 0x08

/*KEY PAD*/
#define KEY_MODE 1
#define KEY_BELL 2
#define KEY_CHECK 3
#define KEY_M_B 4
#define KEY_M_C 5
#define KEY_B_C 6
#define KEY_M_B_C 7
/*Data*/
#define numLine 16     // 3
#define byteOfLine 1   // 33 //17 33
#define byteOfHeader 7 // 13  //7 Numbyte, WRSSI, 4RSSI, BAT%, PW, Status, NumLine

#define sendID 12   // 13  //7
#define sendIMEI 15 // 13  //7
#define sendZone 8  // 13  //7
#define numBytes (numLine * byteOfLine + byteOfHeader + sendID + sendIMEI + sendZone)
/*Time status*/
#define TIME_CHECK_STATUS 100
#define TIME_CHECK_ALRAM 100
#define TIME_LOOP_WIFI 30000
#define TIME_HOLD_SENSOR (timeActiveAlarm / TIME_CHECK_ALRAM)
#define TIME_WIFI (TIME_LOOP_WIFI / TIME_CHECK_ALRAM)

#define UART_SIM UART_NUM_2
/*Function Handle*/
void uart_init(void);

#endif