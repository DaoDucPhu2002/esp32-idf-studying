#include "main.h"
#include "Alam_Handler.h"

#include "mqtt.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "wifi_config.h"
#include "json_generator.h"
#include "json_parser.h"

static const char *TAG_MQTT = "MQTTS_EXAMPLE";
extern Alam_Handler_t alarm_data;
extern uint8_t ESP_ID[6];
char *txt_subscribe = "Alam_seting";
char *topic_server = "Alam_noti";
int msg_id;
esp_mqtt_client_handle_t client;
void readMAC()
{

    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
    {
        for (int i = 0; i < 6; i++)
        {
            ESP_ID[i] = mac[i];
        }
    }

    printf("pubic: %s", txt_subscribe);
}
void get_mqtt_data(char *data, int len)
{

    alarm_data.is_line_not_use = data[0];
    printf("Data Line Not Use: %2x\n", alarm_data.is_line_not_use);
}
// void ReadIMEI()
// {
//     char *cimei = malloc(16);
//     char *IMEI = malloc(15);
//     cimei = GetIMEI();
//     strncpy(IMEI, cimei, 15);
//     printf("IMEI: %s", IMEI);
// }
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG_MQTT, "\nEvent dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, txt_subscribe, 0);

        ESP_LOGI(TAG_MQTT, "\nsent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //  msg_id = esp_mqtt_client_publish(client, topic_server, NULL, 0, 0, 0);
        ESP_LOGI(TAG_MQTT, "\nsent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_DATA");
        ESP_LOGI(TAG_MQTT, "\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG_MQTT, "\nDATA=%.*s\r\n", event->data_len, event->data);
        get_mqtt_data(event->data, event->data_len);

        //  data_handle_callback(event->data, event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_MQTT, "\nMQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG_MQTT, "\nLast errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG_MQTT, "\nOther event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{

    readMAC();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtt://test.mosquitto.org:1883",
        },
    };

    ESP_LOGI(TAG_MQTT, "\n[APP] Free memory: %ld bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void send_data_to_server(uint8_t *data)
{

    msg_id = esp_mqtt_client_publish(client, topic_server, (const char *)data, 17, 0, 0);
    ESP_LOGI(TAG_MQTT, "\nsent publish successful, msg_id=%d", msg_id);
}
