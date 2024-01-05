#include "Alam_Handler.h"
#include "main.h"
#include "esp_timer.h"
extern Alam_Handler_t alarm_data;

static uint64_t get_current_time()
{
    return esp_timer_get_time() / 1000ULL;
}
void select_line(uint8_t channel)
{

    gpio_set_level(S0, channel & 0x01);
    gpio_set_level(S1, (channel >> 1) & 0x01);
    gpio_set_level(S2, (channel >> 2) & 0x01);

    vTaskDelay(2 / portTICK_PERIOD_MS);
    // if ((alarm_data.is_line_not_use << channel) & 0x80 != 0)
    // {
    // }
    alarm_data.line_status_current[channel] = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;
}

void check_alam()
{
    for (int i = 0; i < 8; i++)
    {
        select_line(i);
        if (((alarm_data.is_line_not_use << i) & 0x80) != 0)
        {

            if (alarm_data.line_status_current[i] == NORMAL_STATUS)
            {
                alarm_data.line_status[i] = NORMAL_STATUS;
                alarm_data.is_time_start[i] = 0;
            }
            // check fail line
            if (alarm_data.line_status_current[i] == FAIL_LINE)
            {
                alarm_data.line_status[i] = FAIL_LINE;
                alarm_data.is_time_start[i] = 0;
            }
            if (alarm_data.line_status_current[i] == 1) // ALARM
            {

                if (alarm_data.is_time_start[i] == 0)
                {
                    alarm_data.time_start_status[i] = get_current_time();
                    alarm_data.is_time_start[i] = 1;
                }
                if (get_current_time() - alarm_data.time_start_status[i] >= 5000)
                {
                    alarm_data.line_status[i] = ALARM_STATUS;
                }
            }
        }
        else
        {
            alarm_data.line_status[i] = NOTUSE;
            printf("line %d not use\n", i);
        }
    }
}
void Overall_status()
{
    for (int i = 0; i < 8; i++)
    {
        if (alarm_data.line_status[i] == ALARM_STATUS)
        {
            alarm_data.Status = ALARM_STATUS;
            //  ESP_LOGI("ALAM", "LINE %d FIRE!!!", i);
            return;
        }
        else if (alarm_data.line_status[i] == FAIL_LINE)
        {
            //  ESP_LOGI("ALAM", "SYSTEM ERROR!!!");
            alarm_data.Status = FAIL_LINE;
            continue;
        }
        else if (alarm_data.line_status[i] == NOTUSE)
        {
        }
        else
        {
            alarm_data.Status = NORMAL_STATUS;
        }
    }
}
void Alam_check_out()
{
    if (alarm_data.Status == ALARM_STATUS)
    {
        gpio_set_level(BUZZER, 1);

        gpio_set_level(BEEP, 1);
        gpio_set_level(LAMP1, 1);
        gpio_set_level(OUT1, 1);
        gpio_set_level(OUT2, 1);
        printf("OUT ALL\n");
    }
    else
    {
        gpio_set_level(BUZZER, 0);

        gpio_set_level(BEEP, 0);
        gpio_set_level(LAMP1, 0);
        gpio_set_level(OUT1, 0);
        gpio_set_level(OUT2, 0);
    }
}