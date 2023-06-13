idf_component_register(SRCS  "spiffs.c" "main.c" "deepdeck_tasks.c" "keymap.c" "matrix.c" "keypress_handles.c" "gesture_handles.c" "server.c" "wifi_handles.c"
                    INCLUDE_DIRS "."
                    REQUIRES "OtaWebServer" "spiffs" "mdns" "esp_http_server" "mqtt_dd" "battery" "ble" "oled" "nvsfuncs" "plugins" "rotary_encoder" "rgb_led" "menu" "bus" "apds9960" "uuid_master" ) 


                    spiffs_create_partition_image(www_0 ../spiffs_image FLASH_IN_PROJECT)
