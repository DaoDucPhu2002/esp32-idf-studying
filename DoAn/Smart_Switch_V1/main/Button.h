#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

typedef enum
{
    BUTTON_RELEASE_STATE,
    BUTTON_PRESS_STATE,
    BUTTON_PRESS_SHORT_STATE,
    BUTTON_PRESS_TIMEOUT_STATE,
} button_enven_handle;
typedef void (*input_callback_t)(void);
typedef struct
{
    // GPIO
    gpio_num_t gpio_pin;
    // Handle
    uint8_t btn_current;
    uint8_t btn_last;
    uint8_t btn_filter;
    uint8_t is_debouncing;
    uint32_t time_debounce;
    uint32_t time_start_press;
    uint8_t is_press_timeout;
    button_enven_handle state;
    /*Callback_Handle*/
    input_callback_t Press_callback;
    input_callback_t PressShort_Callback;
    input_callback_t PressTimeout_Callback;
    input_callback_t Release_Callback;
} Button_Handel_t;

void button_Handle(Button_Handel_t *buttonX);
void button_init(Button_Handel_t *buttonX, gpio_num_t gpio_pin);