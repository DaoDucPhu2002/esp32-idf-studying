#include "main.h"
#include "mqtt.h"
#include "peripherals.h"
#include "mqtt_client.h"
#include "esp_log.h"
#define TAG_MQTT "MQTTS_EXAMPLE"
#define topic_mqtt "/DaoDucPhu"
extern uint8_t data_send[numBytes];
static esp_mqtt_client_handle_t mqtt_client;

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_mosquitto_org_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_mosquitto_org_crt_end");

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    LOG_DEBUG(TAG_MQTT, "\nEvent dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, topic_mqtt, 0);
        LOG_DEBUG(TAG_MQTT, "\nsent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, topic_mqtt, "DaoDucPhu", 0, 0, 0);
        LOG_DEBUG(TAG_MQTT, "\nsent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:

        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_DATA");
        LOG_DEBUG(TAG_MQTT, "\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
        LOG_DEBUG(TAG_MQTT, "\nDATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            LOG_DEBUG(TAG_MQTT, "\nLast errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        LOG_DEBUG(TAG_MQTT, "\nOther event id:%d", event->event_id);
        break;
    }
}
void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org:1883",
        // .broker.verification.certificate = (const char *)server_cert_pem_start,
        // .credentials = {
        //     .authentication = {
        //         .certificate = (const char *)client_cert_pem_start,
        //         .key = (const char *)client_key_pem_start,
        //     },
        // }
    };

    LOG_DEBUG(TAG_MQTT, "\n[APP] Free memory: %ld bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_publish_data(const char *topic, const char *data)
{
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, data, 0, 0, 0);
    LOG_DEBUG(TAG_MQTT, "Data published, msg_id=%d", msg_id);
}