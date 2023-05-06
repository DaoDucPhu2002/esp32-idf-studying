#ifndef __INPUT_DEV_H
#define __INPUT_DEV_H
typedef (*input_callback_t) (int);

typedef enum {
    LO_TO_HI = 1,
    HI_TO_LO = 2,
    ANY_EDGE = 3 
}   input_int_type_t;

typedef void (*input_int_handle_t) (int pin);
void input_init(gpio_num_t gpio_num,interrupt_type_edle_t type);
void input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void *callback);

#endif