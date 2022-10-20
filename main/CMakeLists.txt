idf_component_register(SRCS "main.c" "keymap.c" "matrix.c" "keypress_handles.c"
                    REQUIRES "wifi_and_portal" "mqtt_dd" "battery" "ble" "oled" "nvsfuncs" "plugins" "espnowmk32" "rotary_encoder" "rgb_led" "menu")


spiffs_create_partition_image(www_0 ../spiffs_image FLASH_IN_PROJECT)