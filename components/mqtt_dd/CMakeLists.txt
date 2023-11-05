set(component_srcs "src/mqtt.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                       REQUIRES "mqtt")

target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-format)