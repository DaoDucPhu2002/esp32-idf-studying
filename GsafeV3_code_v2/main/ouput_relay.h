#ifndef OUPUT_RELAY_H
#define OUTPUT_RELAY_H
#include "main.h"

void set_channel_relay(uint8_t channel);
void reset_channel_relay(uint8_t channel);
void set_all_channel_relay(void);
void reset_all_channel_relay(void);
void bell_mode(uint8_t mode);

#endif