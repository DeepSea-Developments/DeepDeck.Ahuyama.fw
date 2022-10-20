idf_component_register(SRCS "src/internet.c" "src/wifi.c" "src/portal.c" "src/spiffs.c" "src/vpp_system.c"
                       INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES main spiffs nvs_flash app_update lwip mdns esp_http_server
                       REQUIRES parson
                       )