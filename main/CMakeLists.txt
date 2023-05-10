idf_component_register(SRCS "main.c" "deepdeck_tasks.c" "keymap.c" "matrix.c" "keypress_handles.c" "gesture_handles.c" "server.c" "wifi_handles.c"
                    INCLUDE_DIRS "."
                    REQUIRES "mdns" "esp_http_server" "mqtt_dd" "battery" "ble" "oled" "nvsfuncs" "plugins" "rotary_encoder" "rgb_led" "menu" "bus" "apds9960" "uuid_master") 
