#include "ouput_relay.h"
void set_channel_relay(uint8_t channel)
{
    gpio_set_level(channel, 1);
}
void reset_channel_relay(uint8_t channel)
{
    gpio_set_level(channel, 0);
}
void set_all_channel_relay(void)
{
    gpio_set_level(RL1, 1);
    gpio_set_level(RL2, 1);
    gpio_set_level(RL3, 1);
    gpio_set_level(RL4, 1);
}
void reset_all_channel_relay(void)
{
    gpio_set_level(RL1, 0);
    gpio_set_level(RL2, 0);
    gpio_set_level(RL3, 0);
    gpio_set_level(RL4, 0);
}
void bell_mode(uint8_t mode)
{
    if (mode == 1)
    {
        // callback
    }

    if (mode == 2)
    {
        // callback_mode 2
    }

    if (mode == 3)
    {
        // callback_mode 3
    }
}