# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp_hw_support) 
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "uuid.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()

target_compile_options(${COMPONENT_TARGET} PRIVATE -Wall -Wstrict-prototypes -DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE)

