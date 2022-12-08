idf_component_register(SRCS "main.c" "deepdeck_tasks.c" "keymap.c" "matrix.c" "keypress_handles.c"
                    INCLUDE_DIRS "."
                    REQUIRES "mqtt_dd" "battery" "ble" "oled" "nvsfuncs" "plugins" "espnowmk32" "rotary_encoder" "rgb_led" "menu")
