idf_component_register(SRCS "wifi_config.c" "main.c" "webserver.c" "mqtt.c" "nvs_app.c"
                    INCLUDE_DIRS ".")
set(req json_parser json_generator)