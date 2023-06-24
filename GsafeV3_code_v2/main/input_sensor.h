#ifndef INPUT_SENSOR_H
#define INPUT_SENSOR_H

#include "main.h"

#define TAG_SENSOR "Sensor"
typedef enum
{
    EOLINE_SENSOR_STATE,
    INSEN_SENSOR_STATE,
    NORMAL_SENSOR_STATE,
} Sensor_State;

uint8_t get_state_sensor(int channel);
void sensor_handle(void);

#endif
