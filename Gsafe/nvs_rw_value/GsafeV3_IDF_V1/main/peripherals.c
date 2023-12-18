#include "peripherals.h"
/*User Variable*/
uint32_t timeAlarmOn, timeAlarmOff, timeActiveAlarm, timeAlarm, timeCheckAlarm;
uint32_t timeLoopAlarm, timeLoopStatus, timeLoopBAT, timeAutoBat, numSendData;
/*User Structure*/
struct gsafe_user_data_t
{
    //  uint8_t alarmStatus;   //  1 not use
    uint16_t batVoltage; //  2
    uint8_t batPercent;  //  1
    uint8_t pwStatus;    //  1
    uint8_t Status;      //  1
    uint8_t LEDStatus;   //  1
    int8_t rssi4G;       //  1
    int8_t rssiWF;       //  1
                         // uint16_t sensorEOL;    //  2 //not use
    // uint16_t sensorStatus; //  2 //not use
    uint8_t lineStatus[numLine];
    uint16_t lineNotUse;
    uint8_t holdStatus[numLine];                                                             // xác nhận trạng thái của sensor // chốt status
    uint8_t keySensor[numLine], /*timeLow[numLine], timeHigh[numLine], */ timeHold[numLine]; // not use time low time high//KeySensor giu trang thai de kich hoat  chuong
    bool enableBell;                                                                         //  1
    bool powerBAT;
    uint8_t startCheckAlarm;
    uint16_t timeCounter;
    uint16_t VolLowPwBAT;
    int8_t lowRssiWiFi; /*keyTest; */ // not use test key
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
    LOG_DEBUG("UART", "UART config Done");
}
void gpio_init(void)
{
    gpio_config_t io_config = {};
    io_config.pin_bit_mask = (1 << SA0) | (1 << SA1) | (1 << SA2) | (1 << SA3) |
                             (1 << RL1) | (1 << RL2) | (1 << BELL) | (1 << LAMP) |
                             (1 << VBAT_EN) | (1 << V_BOOST) | (1 << BEEP) |
                             (1 << LED_WF) | (1 << LEDG) | (1 << LEDR);
    io_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_config);

    io_config.pin_bit_mask = (1 << EOLINE) | (1 << INSEN);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_config);
    LOG_DEBUG("GPIO", "GPIO config Done");
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
    LOG_DEBUG("ADC", "ADC config Done");
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
            LOG_DEBUG("SenSor", "Sensor %d not use", line);
        }
        else
        {
            if (gpio_get_level(INSEN) == 0)
            {
                user_data.lineStatus[line] = 0;
                LOG_DEBUG("Sensor", "Sensor %d INSEN", line);
            }
            else
            {
                user_data.lineStatus[line] = gpio_get_level(EOLINE) + gpio_get_level(INSEN) * 2;
                //  LOG_DEBUG("Sensor", "Sensor %d on: %d", line, user_data.lineStatus[line]);
            }
        }
        // LOG_DEBUG("SENSO", "%d is %d", line, user_data.lineStatus[line]);
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
        // LOG_DEBUG("VBAT", "%d volt", user_data.batVoltage);
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
            //  LOG_DEBUG("CHARGE", "ON");
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
        // Ktra line k hoat dong
        if (bitRead(user_data.lineNotUse, line))
        {
            user_data.holdStatus[line] = NOT_USE;
            // Ngat canh bao khu vuc nhung line k su dung
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
                    // đếm số lần báo cháy rồi kích hoạt
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
    LOG_DEBUG("Button", "ADC: %d", sw_adc_value);
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
    // LOG_DEBUG("CheckAlarm", "********************");

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
        LOG_DEBUG("CheckAlarm", "Stt = Alarm");
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
        if (check_RF() | (get_keypad() == KEY_BELL))
        {
            if (keyBell == 0)
            {
                LOG_DEBUG("CheckAlarm", "OFF BELL");

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
    else if (user_data.Status == ENDOFLINE)
    {
        // user_data.enableBell = 1;
        LOG_DEBUG("CheckAlarm", "OFF BELL");

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
    LOG_DEBUG("DATA", "%s", data_send);
}
int check_RF()
{
    uint16_t vadc_RF = adc1_get_raw(ADC_RF);
    if (vadc_RF > 1000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
uint8_t devieceStatus = 0;
uint8_t is_wifi_connect;
uint8_t is_connect_net4G;
uint8_t is_wifi_ap_mode;
void LEDStatus_Handle(uint16_t cnt)
{
    if (user_data.Status == NORMAL)
    {
        LOG_DEBUG("LED", "NORMAL");

        LEDR_ON;
        LEDG_OFF;
    }
    if (user_data.Status == ALARM)
    {
        LOG_DEBUG("LED", "ALARM");

        LEDG_ON;
        LEDR_OFF;
    }
    if (user_data.Status == ENDOFLINE)
    {
        LOG_DEBUG("LED", "ENDOFLINE");

        LEDG_ON;
        if (cnt % 2)
        {
            LEDR_TOGGLE;
        }
    }
    if (!devieceStatus)
    {
        if (cnt % 5 == 0)
        {
            LEDG_TOGGLE;
        }
        if (is_wifi_connect)
        {

            devieceStatus = 1;
            BEEP_OFF;
        }
        else if (is_connect_net4G && (user_data.Status != ALARM))
        {
            if (BUZZER_EN)
            {
                if ((cnt / 5) % 20 == 0)
                {
                    BEEP_TOGGLE;
                }
                else
                {
                    BEEP_OFF;
                }
            }
        }
    }
    else if (devieceStatus)
    {
        if (!is_wifi_ap_mode)
        {
            if ((user_data.rssiWF < -30) && (user_data.rssiWF > -80))
            {
                int x = (100 + user_data.rssiWF) / 4;
                if (cnt % 20 < x)
                {
                    LED_WF_ON;
                }
                else
                {
                    LED_WF_OFF;
                }
            }
            else
            {
                if (cnt % 20 < 1)
                {
                    LED_WF_ON;
                }
                else
                {
                    LED_WF_OFF;
                }
            }
        }
    }
}
uint8_t key_press;
uint16_t num_task = 0;
void taskStatus(void *xParameters)
{

    for (;;)
    {
        num_task++;
        Peripheral_data_processing();
        checkLineStatus();
        if (user_data.startCheckAlarm)
        {
            checkAlarm();
        }
        else
        {

            BELL_OFF;
            LAMP_OFF;
            RL1_OFF;
            RL2_OFF;
        }
        if (num_task % 10 == 0)
        {
            data_to_send();
            key_press = get_keypad();
            VBAT_ON;
            LOG_DEBUG("Status", "stt %d", user_data.Status);
        }
        if ((user_data.startCheckAlarm == 0) && (num_task % 50) == 0)
        {
            user_data.startCheckAlarm = 1;
        }
        user_data.relayStatus = (uint8_t)(gpio_get_level(LAMP) << 3 | gpio_get_level(BELL) << 2 | gpio_get_level(RL1) << 1 | gpio_get_level(RL1));
        LEDStatus_Handle(num_task);
        vTaskDelay(TIME_CHECK_STATUS / portTICK_PERIOD_MS);
    }
}
uint16_t adc_ACOK;
void InitIO()
{
    gpio_init();
    uart_init();
    adc_init();
    LAMP_OFF;
    BELL_ON;
    RL1_OFF;
    RL2_OFF;
    VBAT_OFF;
    LEDR_OFF;
    LEDG_OFF;
    adc_ACOK = 4000;
    user_data.enableBell = 1;
    user_data.Status = NORMAL;
    user_data.powerBAT = 0;
    user_data.lowRssiWiFi = -80;
    user_data.zoneArlam = 0;
    user_data.zoneWarning = 0;
    /*setime: Lưu vào EEPROM*/
    timeAlarmOn = 3000;
    timeAlarmOff = 2000;
    timeAlarm = timeAlarmOn + timeAlarmOff;
    timeActiveAlarm = 3000;
    user_data.lineNotUse = 0;
}