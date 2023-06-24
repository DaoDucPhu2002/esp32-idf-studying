#ifndef KEYPAD_H
#define KEYPAD_H

#include "main.h"
#define TAG_KEYPAD "KEY PAD"
typedef enum
{
    BUTTON_RELEASE,     // 3.3v
    MODE_BUTTON_PRESS,  //~1.65V
    BELL_BUTTON_PRESS,  //~2.24V
    CHECK_BUTTON_PRESS, // 2.51V
                        // MODE_AND_CHECK_BUTTON_PRESS, // 1.53V // không dùng tới
} KeyPad_State;

uint8_t KeyPad_Handle();
void KeyPad_Mode_Callback(void);
void KeyPad_Check_Callback(void);
void KeyPad_Bell_Callback(void);

#endif