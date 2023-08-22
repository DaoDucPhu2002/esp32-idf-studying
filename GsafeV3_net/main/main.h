#define DEBUG_EN 1

#define LOG_DEBUG(TAG, format, ...)               \
    do                                            \
    {                                             \
        if (DEBUG_EN)                             \
        {                                         \
            ESP_LOGI(TAG, format, ##__VA_ARGS__); \
        }                                         \
    } while (0)
