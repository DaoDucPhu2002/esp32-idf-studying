#include "main.h"
#include "mqtt.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "EC200.h"
#include "wifi_config.h"
static const char *TAG_MQTT = "MQTTS_EXAMPLE";

int msg_id;
esp_mqtt_client_handle_t client;

void readMAC()
{

    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
    {

        snprintf(ESP_ID, 13, "%02X%02X%02X%02X%02X%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        LOG_DEBUG("MAC", "%s", ESP_ID);
    }

    for (int i = 0; i < 12; i++)
    {
        txt_subscribe[i + 4] = ESP_ID[i];
    }
    printf("pubic: %s", txt_subscribe);
}

void ReadIMEI()
{
    char *cimei = malloc(16);
    char *IMEI = malloc(15);
    cimei = GetIMEI();
    strncpy(IMEI, cimei, 15);
    printf("IMEI: %s", IMEI);
}
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

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, txt_subscribe, 0);
        esp_mqtt_client_publish(client, topic_server, txt_subscribe, 0, 0, 0);
        LOG_DEBUG(TAG_MQTT, "\nsent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        LOG_DEBUG(TAG_MQTT, "\nMQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, topic_server, "Test data", 0, 0, 0);
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
    readMAC();
    //  ReadIMEI();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtt://test.api.lctech.vn:1884"},

        .credentials = {
            .username = "lctech_rmq",
            .authentication.password = "lctechrmq123@",
            .client_id = ESP_ID,
        },

    };

    LOG_DEBUG(TAG_MQTT, "\n[APP] Free memory: %ld bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void send_data_to_server(uint8_t *data)
{

    msg_id = esp_mqtt_client_publish(client, topic_server, (const char *)data, sizeof(data), 0, 0);
    LOG_DEBUG(TAG_MQTT, "\nsent publish successful, msg_id=%d", msg_id);
}