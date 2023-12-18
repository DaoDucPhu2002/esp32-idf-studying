#include "EC200.h"

#define PWRKEY_GPIO GPIO_NUM_32
#define UART_SIM_NUM UART_NUM_2

#define TAG_EC200 "EC200"
#define BUFF_SIZE 300
static bool send_command(const char *cmd)
{

    if (uart_write_bytes(UART_SIM_NUM, cmd, strlen(cmd)) != -1)
    {
        ESP_LOGI(TAG_EC200, "send command:%s :true\n", cmd);
        return true;
    }
    return false;
}
static bool send_and_check_response(const char *command, const char *expected_response, char *response, size_t response_size)
{
    send_command(command);
    int len = uart_read_bytes(UART_SIM_NUM, response, response_size - 1, 1000 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        response[len] = '\0'; // Đảm bảo null-terminated string
        ESP_LOGI("response", "%s", response);

        return strstr(response, expected_response) != NULL;
    }
    return false;
}

void init_4G()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PWRKEY_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_SIM_NUM, &uart_config);
    uart_set_pin(UART_SIM_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_SIM_NUM, BUFF_SIZE, 0, 0, NULL, 0);
}

bool power_on()
{

    const TickType_t timeout_ms = 300; // Thời gian timeout (300ms)
    int retry_power = 0;
    int is_power_on = 0;
    TickType_t start_time = xTaskGetTickCount(); // Thời điểm bắt đầu timeout

    while (1)
    {
        gpio_set_level(PWRKEY_GPIO, 1); // Nhấn nút PWRKEY
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(PWRKEY_GPIO, 0); // Nhả nút PWRKEY

        char response[20];
        int len = uart_read_bytes(UART_SIM_NUM, response, sizeof(response), timeout_ms / portTICK_PERIOD_MS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (len > 0)
        {
            response[len] = '\0'; // Kết thúc chuỗi
            ESP_LOGI("UART", "Received response: %s", response);

            if (strstr(response, "RDY") != NULL)
            {
                retry_power = 0;
                return true;
            }
            if (strstr(response, "POWERED DOWN") != NULL)
            {
                retry_power++;
                printf("%d\n", retry_power);
            }
            if (retry_power > 5)
            {
                // return false;
            }
        }

        // Kiểm tra timeout
        // TickType_t current_time = xTaskGetTickCount();
        // if ((current_time - start_time) * portTICK_PERIOD_MS >= timeout_ms)
        // {
        //     // Timeout
        //     // return false;
        // }
    }

    return true;
}

char *GetIMEI()
{
    char *data = (char *)malloc(BUFF_SIZE);
    if (!data)
    {
        ESP_LOGE(TAG_EC200, "Memory allocation failed");
        return NULL;
    }

    if (send_and_check_response("AT+CGSN\r\n", "\r\n", data, BUFF_SIZE))
    {
        const char *imei_start = strstr(data, "\r\n");
        if (imei_start)
        {
            imei_start += 2; // Bỏ qua "\r\n"
            const char *imei_end = strstr(imei_start, "\r\n");

            if (imei_end)
            {
                int imei_len = imei_end - imei_start;
                char *imei = (char *)malloc(imei_len + 1);
                if (imei)
                {
                    strncpy(imei, imei_start, imei_len);
                    imei[imei_len] = '\0';
                    ESP_LOGI("IMEI_SIM", "%s", imei);
                    free(data);
                    return imei;
                }
            }
        }
    }

    free(data);
    return NULL;
}

int8_t GetRSSI()
{
    char *data = (char *)malloc(BUFF_SIZE);
    if (!data)
    {
        ESP_LOGE(TAG_EC200, "Memory allocation failed");
        return 0;
    }

    if (send_and_check_response("AT+CSQ\r\n", "OK", data, BUFF_SIZE))
    {
        const char *rssi_start = strstr(data, ": ") + 2;
        if (rssi_start)
        {
            const char *rssi_end = strstr(rssi_start, ",");
            if (rssi_end)
            {
                int rssi_len = rssi_end - rssi_start;
                if (rssi_len > 0 && rssi_len < 3)
                {
                    char rssi[3];
                    strncpy(rssi, rssi_start, rssi_len);
                    rssi[rssi_len] = '\0';
                    int rssi_value = atoi(rssi);
                    free(data);
                    ESP_LOGI(TAG_EC200, "RSSI_SIM: %d", rssi_value);
                    return (int8_t)rssi_value;
                }
            }
        }
    }

    free(data);
    return 0;
}

int MQTT_Open(char *mqtt_server, int mqtt_port, char *esp_id, char *topic_srv, char *user, char *passwd)
{
    char data[BUFF_SIZE];
    char response[BUFF_SIZE];

    if (!send_and_check_response("AT+QMTCFG=\"recv/mode\",0,0,1\r\n", "OK", response, sizeof(response)))
    {
        return 0;
    }
    snprintf(data, sizeof(data), "AT+QMTOPEN=0,\"%s\",%d\r\n", mqtt_server, mqtt_port);
    if (!send_and_check_response(data, "+QMTOPEN: 0,0", response, sizeof(response)))
    {
        return 0;
    }
    // them user+pass
    snprintf(data, sizeof(data), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n", esp_id, user, passwd);
    if (!send_and_check_response(data, "+QMTCONN: 0,0,0", response, sizeof(response)))
    {
        return 0;
    }

    return 1;
}

int MQTT_Publish(char *subcribe_srv, char *topic_srv, char *mqtt_data, unsigned int length)
{
    char data[BUFF_SIZE];
    char response[BUFF_SIZE];

    snprintf(data, sizeof(data), "AT+QMTSUB=0,1,\"%s\",0\r\n", subcribe_srv);
    if (!send_and_check_response(data, "+QMTSUB: 0,1,0", response, sizeof(response)))
    {
        return 0;
    }

    snprintf(data, sizeof(data), "AT+QMTPUBEX=0,0,0,0,\"%s\",%d\r\n", topic_srv, length);
    if (!send_and_check_response(data, ">", response, sizeof(response)))
    {
        return 0;
    }

    send_command(mqtt_data);
    printf("%s\n", mqtt_data);
    int len = uart_read_bytes(UART_SIM_NUM, response, BUFF_SIZE, 1000 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        ESP_LOGI("response", "%s", response);
        if (strstr(response, "SEND OK") == NULL)
        {
            return 0;
        }
    }

    return 1;
}

bool power_off()
{
    char response[128];

    send_command("AT+QPOWD\r\n");
    int len = uart_read_bytes(UART_SIM_NUM, (uint8_t *)response, sizeof(response) - 1, 300 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        response[len] = '\0'; // Kết thúc chuỗi
        ESP_LOGI("UART", "Received response: %s", response);

        if (strstr(response, "NORMAL POWER DOWN") != NULL)
        {
            ESP_LOGI("UART", "Module powered off");
            return true;
        }
    }
    return false;
}
bool restart_module()
{
    power_off();
    bool is_power_on = power_on();
    while (!is_power_on)
    {
        is_power_on = power_on();
    }
    return true;
}
