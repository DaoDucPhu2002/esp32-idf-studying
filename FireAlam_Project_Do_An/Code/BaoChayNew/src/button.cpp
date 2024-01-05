#include "button.h"
#include "Arduino.h"

void __attribute__((weak)) btn_pressing_callback(Button_Typedef_t *ButtonX)
{
    // NULL
}
void __attribute__((weak)) btn_press_short_callback(Button_Typedef_t *ButtonX)
{
    // NULL
}
void __attribute__((weak)) btn_release_callback(Button_Typedef_t *ButtonX)
{
    // NULL
}
void __attribute__((weak)) btn_press_timeout_callback(Button_Typedef_t *ButtonX)
{
    // NULL
}

void button_handle(Button_Typedef_t *ButtonX)
{
    //------------------ Loc nhieu ------------------------
    uint8_t sta = digitalRead(ButtonX->pin);
    if (sta != ButtonX->btn_filter)
    {
        ButtonX->btn_filter = sta;
        ButtonX->is_debouncing = 1;
        ButtonX->time_deboune = millis();
    }
    //------------------ Tin hieu da xac lap------------------------
    if (ButtonX->is_debouncing && (millis() - ButtonX->time_deboune >= 15))
    {
        ButtonX->btn_current = ButtonX->btn_filter;
        ButtonX->is_debouncing = 0;
    }
    //---------------------Xu li nhan nha------------------------
    if (ButtonX->btn_current != ButtonX->btn_last)
    {
        if (ButtonX->btn_current == 0) // nhan xuong
        {
            ButtonX->is_press_timeout = 1;
            btn_pressing_callback(ButtonX);
            ButtonX->time_start_press = millis();
        }
        else // nha nut
        {
            if (millis() - ButtonX->time_start_press <= 1000)
            {
                btn_press_short_callback(ButtonX);
            }
            btn_release_callback(ButtonX);
            ButtonX->is_press_timeout = 0;
        }
        ButtonX->btn_last = ButtonX->btn_current;
    }
    //-------------Xu li nhan giu----------------
    if (ButtonX->is_press_timeout && (millis() - ButtonX->time_start_press >= 3000))
    {
        btn_press_timeout_callback(ButtonX);
        ButtonX->is_press_timeout = 0;
    }
}
void button_init(Button_Typedef_t *ButtonX, uint16_t GPIO_Pin)
{
    ButtonX->pin = GPIO_Pin;
    pinMode(GPIO_Pin, INPUT);
}
