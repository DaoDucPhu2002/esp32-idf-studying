#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
typedef struct
{
    uint16_t pin;
    uint8_t btn_current;
    uint8_t btn_last;
    uint8_t btn_filter;
    uint8_t is_debouncing;
    uint32_t time_deboune;
    uint32_t time_start_press;
    uint8_t is_press_timeout;

} Button_Typedef_t;

extern "C"
{
    typedef void (*Button_Handle_Callback_t)(Button_Typedef_t *);
}
void button_init(Button_Typedef_t *ButtonX, uint16_t pin);
void button_handle(Button_Typedef_t *ButtonX);

#endif