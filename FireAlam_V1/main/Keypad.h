#include "main.h"

#define TAG_BTN "BUTTON"

typedef enum
{
    BUTTON_RELEASE,              // 3.3v
    MODE_BUTTON_PRESS,           //~1.65V
    BELL_BUTTON_PRESS,           //~2.24V
    CHECK_BUTTON_PRESS,          // 2.51V
    MODE_AND_CHECK_BUTTON_PRESS, // 1.53V
} KeyPad_State;

KeyPad_State get_keypad_state(void);
void keypad_handle(void);