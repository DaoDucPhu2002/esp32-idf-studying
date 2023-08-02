#include "main.h"

#define TAG_SENSOR "Sensor"
typedef enum
{
    EOLINE_SENSOR_STATE,
    INSEN_SENSOR_STATE,
    NORMAL_SENSOR_STATE,
} Sensor_State;

int get_state_sensor(int channel);
