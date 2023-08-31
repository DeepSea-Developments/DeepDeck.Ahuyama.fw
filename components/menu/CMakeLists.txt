set(component_srcs "src/menu.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver"
                       REQUIRES "ble" "rotary_encoder" "rgb_led" "nvsfuncs")