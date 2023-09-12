idf_component_register(SRCS "main.c" "webserver.c" "wifi_config.c" "mqtt.c"
                    INCLUDE_DIRS ".")
set(req json_parser json_generator)