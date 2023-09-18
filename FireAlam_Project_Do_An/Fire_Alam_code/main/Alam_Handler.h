#ifndef __ALAM_HANDLER_H
#define __ALAM_HANDLER_H

#include "main.h"
typedef enum
{
    ALARM,
    NOT_USE,
    NORMAL,
    FAIL_LINE,
} Alarm_Status_Typedef_t;
typedef struct
{
    uint8_t esp_id[12];
    Alarm_Status_Typedef_t Status;
    Alarm_Status_Typedef_t line_status[8];
    Alarm_Status_Typedef_t line_status_current[8];
    Alarm_Status_Typedef_t line_status_last[8];
    uint64_t time_start_status[8];
    uint8_t is_line_not_use[8];
} Alam_Handler_t;
Alarm_Status_Typedef_t check_line(uint8_t line);
void check_alam();
#endif