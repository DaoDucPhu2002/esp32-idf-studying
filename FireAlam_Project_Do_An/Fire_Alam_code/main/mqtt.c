
#include "mqtt.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "wifi_config.h"
#include "json_generator.h"
#include "json_parser.h"

static const char *TAG_MQTT = "MQTTS_EXAMPLE";

extern Button_status_t device;
char *ESP_ID;
char *txt_subscribe = "DaoDucPhu";
char *topic_server = "DaoDucPhu";
int msg_id;
esp_mqtt_client_handle_t client;
mqtt_data_handle_t data_mqtt;
mqtt_data_handle_callback_t data_handle_callback = NULL;
// void readMAC()
// {

//     uint8_t mac[6];
//     if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
//     {

//         snprintf(ESP_ID, 13, "%02X%02X%02X%02X%02X%02X",
//                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//         ESP_LOGI("MAC", "%s", ESP_ID);
//     }

//     for (int i = 0; i < 12; i++)
//     {
//         txt_subscribe[i + 4] = ESP_ID[i];
//     }
//     printf("pubic: %s", txt_subscribe);
// }

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

        msg_id = esp_mqtt_client_subscribe(client, topic_server, 0);

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
        data_handle_callback(event->data, event->data_len);
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

    msg_id = esp_mqtt_client_publish(client, topic_server, (const char *)data, strlen((const char *)data), 0, 0);
    ESP_LOGI(TAG_MQTT, "\nsent publish successful, msg_id=%d", msg_id);
}
/*===================================================*/
/*              JSON Handle                          */
static void flush_str(char *buf, void *priv)
{
    json_gen_test_result_t *result = (json_gen_test_result_t *)priv;
    if (result)
    {
        if (strlen(buf) > sizeof(result->buf) - result->offset)
        {
            printf("Result Buffer too small\r\n");
            return;
        }
        memcpy(result->buf + result->offset, buf, strlen(buf));
        result->offset += strlen(buf);
    }
}
void json_gen_perform_test(json_gen_test_result_t *result, int sta_button1, int sta_button2, uint64_t time_button1, uint64_t time_button2)
{
    char buf[20];
    memset(result, 0, sizeof(json_gen_test_result_t));
    json_gen_str_t jstr;
    json_gen_str_start(&jstr, buf, sizeof(buf), flush_str, result);
    json_gen_start_object(&jstr);
    json_gen_obj_set_int(&jstr, "Status_Button_1", sta_button1);
    json_gen_obj_set_int(&jstr, "Status_Button_2", sta_button2);
    json_gen_obj_set_int(&jstr, "Timer_Button_1", time_button1);
    json_gen_obj_set_int(&jstr, "Timer_Button_2", time_button2);
    json_gen_end_object(&jstr);
    json_gen_str_end(&jstr);
    printf("Gennerated: %s\n", result->buf);
}
void get_mqtt_data(char *data, int len)
{

    jparse_ctx_t jctx;
    int ret = json_parse_start(&jctx, data, len);
    if (ret != OS_SUCCESS)
    {
        ESP_LOGE("JSON PARSER", "Fail");
    }
    if (ret == OS_SUCCESS)
    {
        if (json_obj_get_int(&jctx, "Status_Button_1", &device.sta_button1) == OS_SUCCESS)
        {
            printf("Button1 Sta: %s\n", device.sta_button1 ? "on" : "off");
        }
        if (json_obj_get_int(&jctx, "Status_Button_2", &device.sta_button2) == OS_SUCCESS)
        {
            printf("Button2 Sta: %s\n", (device.sta_button2) ? "on" : "off");
        }
        if (json_obj_get_int64(&jctx, "Timer_Button_1", &device.timer_button1) == OS_SUCCESS)
        {
            printf("Button1 delay: %lld\n", device.timer_button1);
        }
        if (json_obj_get_int64(&jctx, "Timer_Button_2", &device.timer_button2) == OS_SUCCESS)
        {
            printf("Button2 delay: %lld\n", device.timer_button2);
        }
    }
}
void mqtt_set_callback(void *cb)
{

    if (cb)
    {
        data_handle_callback = cb;
    }
}