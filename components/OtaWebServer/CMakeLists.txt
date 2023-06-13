idf_component_register(SRCS "wifi.c" "ota.c"
                    INCLUDE_DIRS include
                    EMBED_TXTFILES "WebAtlantis/index.html"
                    REQUIRES nvs_flash esp_http_server mdns app_update)

 
    