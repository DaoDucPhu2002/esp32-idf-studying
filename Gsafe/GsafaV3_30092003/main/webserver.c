#include "webserver.h"
#include "esp_http_server.h"
#include "esp_log.h"

httpd_handle_t server = NULL;
#define TAG "WebServer"
/*Handle Callback*/
static http_server_handle http_get_handle = NULL;
static http_server_handle http_post_handle = NULL;
static http_server_handle http_get_info_handle = NULL;

/*Extern HTML File */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

/*URL Varialbe*/
char url_data[100];

/*=======================================================
                        Webserver Handle
=========================================================*/
static httpd_req_t *get_req;
void http_send_response(char *data, int len)
{
    httpd_resp_send(get_req, (const char *)data, len);
}
/*HTTP GET Handler*/
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Location") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Location", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Location: %s", buf);
        }
        free(buf);
    }
    //  const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}

static const httpd_uri_t home = {
    .uri = "/home",
    .method = HTTP_GET,
    .handler = wifi_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = NULL};

/* An HTTP POST handler */
static esp_err_t post_handler(httpd_req_t *req)
{
    char buf[100];
    int len = httpd_req_recv(req, buf, 100);
    http_post_handle(buf, len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t wifi = {
    .uri = "/wifi_get_pass",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};
/*GET Wifi_infor*/
static esp_err_t get_wifi_info(httpd_req_t *req)
{
    get_req = req;
    http_get_info_handle("Wifi get info", 14);
    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}
static const httpd_uri_t get_wifi = {
    .uri = "/wifi_info",
    .method = HTTP_GET,
    .handler = get_wifi_info,
    .user_ctx = NULL};
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/home", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    else if (strcmp("/wifi_get_pass", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &wifi);
        httpd_register_uri_handler(server, &get_wifi);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void http_get_set_callback(void *cb)
{
    if (cb)
    {
        http_get_handle = cb;
    }
}

void http_post_set_callback(void *cb)
{
    if (cb)
    {
        http_post_handle = cb;
    }
}
void http_get_wifi_infor_callback(void *cb)
{
    if (cb)
    {
        http_get_info_handle = cb;
    }
}