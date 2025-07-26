idf_component_register(SRCS "wifi_manager.c"
                    INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                    REQUIRES nvs_flash esp_netif esp_wifi esp_event driver freertos lwip)
