set(component_srcs "src/rgb_led.c" "src/led_strip_rmt_ws2812.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver"
                       REQUIRES nvs_flash nvsfuncs 
                       )