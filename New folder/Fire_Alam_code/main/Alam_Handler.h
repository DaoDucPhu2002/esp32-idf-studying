#ifndef __ALAM_HANDLER_H
#define __ALAM_HANDLER_H

#include "main.h"

typedef struct
{
    uint8_t esp_id[12];
    uint8_t Status;
    uint8_t line_status[8];
    uint8_t line_status_current[8];
    uint8_t line_status_last[8];
    uint64_t time_start_status[8];
    uint8_t is_time_start[8];
    uint8_t is_line_not_use;
} Alam_Handler_t;

void Overall_status();
void check_alam();
void select_line(uint8_t channel);
void Alam_check_out();
#endif