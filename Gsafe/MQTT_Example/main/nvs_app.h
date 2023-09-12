#ifndef __NVS_APP_H
#define __NVS_APP_H

#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
esp_err_t app_flash_set_int(char *key, int value);
esp_err_t app_flash_set_str(char *key, char *str);
esp_err_t app_flash_get_int(char *key, int *value);
esp_err_t app_flash_get_str(char *key, char *str, int *len);

#endif