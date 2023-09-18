#include "Alam_Handler.h"
#include "main.h"
#include "esp_timer.h"
Alam_Handler_t alarm_data;
static uint64_t get_current_time()
{
    return esp_timer_get_time() / 1000;
}
static void select_line(uint8_t channel)
{

    gpio_set_level(S0, channel & 0x01);
    gpio_set_level(S1, (channel >> 1) & 0x01);
    gpio_set_level(S2, (channel >> 2) & 0x01);
    gpio_set_level(S3, (channel >> 3) & 0x01);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    if (alarm_data.is_line_not_use[channel] == 0)
    {
        alarm_data.line_status_current[channel] = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;
    }
}

void check_alam()
{
    for (int i = 0; i < 8; i++)
    {
        check_alam();
        if (alarm_data.is_line_not_use[i] != 0)
        {

            if (alarm_data.line_status_current[i] == ALARM && alarm_data.line_status_current[i] != alarm_data.line_status_last[i])
            {
                alarm_data.time_start_status[i] = get_current_time();
                alarm_data.line_status_last[i] = alarm_data.line_status_current[i];
            }

            if (alarm_data.line_status_current != ALARM)
            {
                alarm_data.time_start_status[i] = 0;
                alarm_data.line_status_last[i] = alarm_data.line_status_current[i];
                alarm_data.line_status[i] = alarm_data.line_status_current[i];
            }

            if (alarm_data.time_start_status[i] > 5000)
            {
                alarm_data.line_status[i] = ALARM;
            }
        }
        else
        {
            alarm_data.line_status[i] = NOT_USE;
        }
    }
}
void Overall_State()
{
    for (int i = 0; i < 8; i++)
    {
        if (alarm_data.line_status[i] == ALARM)
        {
            alarm_data.Status = ALARM;
            return;
        }
        if (alarm_data.line_status[i] == FAIL_LINE)
        {
            alarm_data.Status = FAIL_LINE;
            return;
        }
        alarm_data.Status = NORMAL;
    }
}