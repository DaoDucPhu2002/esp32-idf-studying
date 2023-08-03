#include "peripherals.h"
/*User Variable*/
uint32_t timeAlarmOn, timeAlarmOff, timeActiveAlarm, timeAlarm, timeCheckAlarm;
uint32_t timeLoopAlarm, timeLoopStatus, timeLoopBAT, timeAutoBat, numSendData;
/*User Structure*/
struct gsafe_user_data_t
{
    uint8_t alarmStatus;   //  1
    uint16_t batVoltage;   //  2
    uint8_t batPercent;    //  1
    uint8_t pwStatus;      //  1
    uint8_t Status;        //  1
    uint8_t LEDStatus;     //  1
    int8_t rssi4G;         //  1
    int8_t rssiWF;         //  1
    uint16_t sensorEOL;    //  2
    uint16_t sensorStatus; //  2
    uint8_t lineStatus[numLine];
    uint16_t lineNotUse;
    uint8_t holdStatus[numLine];
    uint8_t keySensor[numLine], timeLow[numLine], timeHigh[numLine], timeHold[numLine];
    bool enableBell; //  1
    bool powerBAT;
    uint8_t startCheckAlarm;
    uint16_t timeCounter;
    uint16_t VolLowPwBAT;
    int8_t lowRssiWiFi, keyTest;
    uint8_t relayStatus;
    uint32_t zoneArlam;
    uint32_t zoneWarning;
};

struct gsafe_user_data_t user_data;
/*ADC*/
uint16_t vbat;
uint16_t adcACOK;
uint8_t num_of_same_vbat = 0;
/*Key bell*/
bool keyBell;
/*Data sent to server*/
uint8_t data_send[numBytes];
/*Init*/
void uart_init()
{
    uart_config_t uart_ec200 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_NUM_2, BUFF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_ec200);
    uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
void gpio_init(void)
{
    gpio_config_t io_config = {};
    io_config.pin_bit_mask = (1 << SA0) | (1 << SA1) | (1 << SA2) | (1 << SA3) | (1 << RL1) | (1 << RL2) | (1 << BELL) | (1 << LAMP) | (1 << VBAT_EN) | (1 << V_BOOST);
    io_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_config);

    io_config.pin_bit_mask = (1 << EOLINE) | (1 << INSEN);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_config);
}
void adc_init(void)
{
    adc1_config_channel_atten(ADC_KEY, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC_CHARGER, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC_RF, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC_VBAT, ADC_ATTEN_DB_11); // DB 11 dai dien ap toi da la 3,3v
    adc1_config_width(ADC_WIDTH_BIT_12);
}
static void Select_line(int line)
{
    if (line < 8)
    {
        line = 7 - line;
    }

    gpio_set_level(SA0, line & 0x01);
    gpio_set_level(SA1, (line >> 1) & 0x01);
    gpio_set_level(SA2, (line >> 2) & 0x01);
    gpio_set_level(SA3, (line >> 3) & 0x01);
    vTaskDelay(2 / portTICK_PERIOD_MS);
}
void Read_Sensor()
{
    for (int line = 0; line < numLine; line++)
    {
        Select_line(line);
        if (bitRead(user_data.lineNotUse, line))
        {
            user_data.lineStatus[line] = NOT_USE;
        }
        else
        {
            if (gpio_get_level(INSEN) == 0)
            {
                user_data.lineStatus[line] = 0;
            }
            else
            {
                user_data.lineStatus[line] = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;
            }
        }
    }
}
void Peripheral_data_processing()
{
    /*Vbat*/
    vbat += adc1_get_raw(ADC_VBAT);
    num_of_same_vbat += 1;
    if (num_of_same_vbat > 10)
    {
        vbat /= 10;
        user_data.batVoltage = vbat / 3 - (vbat - 1800) / 50;
        LOG_DEBUG("VBAT", "%d volt", user_data.batVoltage);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
    adcACOK = adc1_get_raw(ADC_CHARGER);
    Read_Sensor();
    user_data.pwStatus = 0;
    if (adcACOK < 2500)
    {
        user_data.pwStatus |= PW_ACIN;
        if (gpio_get_level(VBAT_EN))
        {
            user_data.pwStatus |= PWM_CHARG;
        }
        if (user_data.batVoltage < 800)
        {
            CHARGE_ON;
        }
    }
    else
    {
        user_data.pwStatus = PW_BAT;
    }
    if (user_data.batVoltage >= 650)
    {
        user_data.pwStatus |= BAT_OK;
    }
}
void checkLineStatus()
{
    for (uint8_t line = 0; line < numLine; line++)
    {
        if (bitRead(user_data.lineNotUse, line))
        {
            user_data.holdStatus[line] = NOT_USE;
            bitClear(user_data.zoneArlam, line);
            bitClear(user_data.zoneWarning, line);
        }
        else
        {
            if (user_data.lineStatus[line] == ALARM)
            {
                if (user_data.keySensor[line] != ALARM)
                {
                    user_data.keySensor[line] = ALARM;
                    user_data.timeHold[line] = 1;
                }
                else
                {
                    user_data.timeHold[line]++;
                    if (user_data.timeHold[line] >= TIME_HOLD_SENSOR)
                    {
                        user_data.timeHold[line] = TIME_HOLD_SENSOR;
                        user_data.holdStatus[line] = ALARM;
                        bitSet(user_data.zoneArlam, line);
                    }
                }
            }
            else if (user_data.lineStatus[line] == ENDOFLINE)
            {
                if (user_data.keySensor[line] != ENDOFLINE)
                {
                    user_data.keySensor[line] = ENDOFLINE;
                    user_data.timeHold[line] = 1;
                }
                else // keySensor ==  ENDOFLINE
                {
                    user_data.timeHold[line]++;
                    if (user_data.timeHold[line] >= TIME_HOLD_SENSOR)
                    {
                        user_data.timeHold[line] = TIME_HOLD_SENSOR;
                        user_data.holdStatus[line] = ENDOFLINE;
                        bitSet(user_data.zoneWarning, line);
                    }
                }
            }
            else
            {

                user_data.timeHold[line] = 0;
                user_data.holdStatus[line] = NORMAL;
                user_data.keySensor[line] = NORMAL;
            }
        }
    }
}
/*mS Function*/
static uint32_t get_ms(void)
{
    return esp_timer_get_time() / 1000;
}
int get_keypad()
{
    uint16_t sw_adc_value = adc1_get_raw(ADC_KEY);
    if (sw_adc_value < 2100)
    {
        LOG_DEBUG("Button", "Pressing Mode Button");
        return KEY_MODE;
    }
    else if (sw_adc_value <= 2600)
    {
        LOG_DEBUG("Button", "Pressing Bell Button");
        return KEY_BELL;
    }
    else if (sw_adc_value <= 3500)
    {
        LOG_DEBUG("Button", "Pressing Check Button");
        return KEY_CHECK;
    }
    else
    {
        return 0;
    }
}
void checkAlarm(void)
{
    int line;
    timeCheckAlarm = get_ms() - timeLoopAlarm;
    if (get_ms() - timeLoopAlarm > timeAlarm)
    {
        timeLoopAlarm = get_ms();
        timeAlarm = timeAlarmOn + timeAlarmOff;
    }
    user_data.Status = NORMAL;
    for (line = 0; line < numLine; line++)
    {
        if (user_data.holdStatus[line] == ENDOFLINE)
        {
            user_data.Status = ENDOFLINE;
        }
    }
    for (line = 0; line < numLine; line++)
    {
        if (user_data.holdStatus[line] == ALARM)
        {
            user_data.Status = ALARM;
        }
    }
    /*Output Relay*/
    if (user_data.Status == ALARM)
    {
        user_data.LEDStatus = 1;
        if (timeCheckAlarm < timeAlarmOn)
        {
            if (user_data.enableBell)
            {
                BELL_ON;
                if (BUZZER_EN)
                {
                    BEEP_ON;
                }
            }
            else
            {
                BELL_OFF;
                BEEP_OFF;
            }
        }
        if (/*RF Remote |*/ (get_keypad() == KEY_BELL))
        {
            if (keyBell == 0)
            {
                user_data.enableBell = !user_data.enableBell;
                keyBell = 1;
                if (user_data.enableBell)
                {
                    BELL_ON;
                    if (BUZZER_EN)
                    {
                        BEEP_ON;
                    }
                }
                else
                {
                    BELL_OFF;
                    BEEP_OFF;
                }
            }
        }
        else
        {
            keyBell = 0;
        }
        LAMP_ON;
        RL1_ON;
        RL2_ON;
    }
    else
    {
        user_data.enableBell = 1;
        user_data.LEDStatus = 2;
        BELL_OFF;
        LAMP_OFF;
        RL1_OFF;
        RL2_OFF;
        BEEP_OFF;
    }
}

void data_to_send()
{
    uint16_t index = 0;
    uint8_t line;
    uint32_t time1s = get_ms() / 1000;
    /*user_data.batPercent = ()*/
    if (user_data.batPercent > 100)
    {
        user_data.batPercent = 100;
    }
    /*add RSSI Wifi*/
    /*.............*/
    data_send[index++] = numBytes;
    for (line = 0; line < 12; line++)
    {
        data_send[index++] = ESP_ID[line];
    }
    for (line = 0; line < sendIMEI; line++)
    {
        data_send[index++] = IMEI[line];
    }
    data_send[index++] = user_data.rssiWF;
    data_send[index++] = user_data.rssi4G;
    data_send[index++] = user_data.pwStatus;
    data_send[index++] = user_data.batPercent;
    data_send[index++] = user_data.Status;
    data_send[index++] = numLine;
    for (line = 0; line < numLine; line++)
    {
        data_send[index++] = user_data.lineStatus[line];
    }
    data_send[index++] = (numSendData >> 24) & 0xff;
    data_send[index++] = (numSendData >> 16) & 0xff;
    data_send[index++] = (numSendData >> 8) & 0xff;
    data_send[index++] = (numSendData)&0xff;
    data_send[index++] = (time1s >> 24) & 0xff;
    data_send[index++] = (time1s >> 16) & 0xff;
    data_send[index++] = (time1s >> 8) & 0xff;
    data_send[index++] = (time1s)&0xff;
    // done
    LOG_DEBUG("DATA", "Data Process Done");
}
