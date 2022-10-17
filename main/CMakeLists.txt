idf_component_register(SRCS "main.c" "keymap.c" "matrix.c" "keypress_handles.c"
                    INCLUDE_DIRS "."
                    REQUIRES "wifi_and_portal" "mqtt_dd" "battery" "ble" "oled" "nvsfuncs" "plugins" "espnowmk32" "rotary_encoder" "rgb_led" "menu")
