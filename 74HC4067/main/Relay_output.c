#include "Relay_output.h"

void set_relay_output(uint8_t channel, uint8_t value)
{
    gpio_set_level(channel, value);
    ESP_LOGI(TAG_RELAY, "Relay %d is %s", channel, value ? "ON" : "OFF");
}
void set_all_relay()
{
    set_relay_output(RL1, 1);
    set_relay_output(RL2, 1);
    set_relay_output(RL3, 1);
    set_relay_output(RL4, 1);
}
void reset_all_relay()
{

    set_relay_output(RL1, 0);
    set_relay_output(RL2, 0);
    set_relay_output(RL3, 0);
    set_relay_output(RL4, 0);
}