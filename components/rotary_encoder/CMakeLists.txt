set(component_srcs "src/rotary_encoder_pcnt_ec11.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver"
                       REQUIRES "ble" "keys")