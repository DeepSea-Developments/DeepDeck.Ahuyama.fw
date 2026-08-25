# Toolchain the firmware is verified against.
#
# The espressif/idf images set IDF_PATH and ship an entrypoint that sources
# export.sh, so the long list of hardcoded tool paths the previous version
# carried is unnecessary - and those paths only ever existed in the v4.4 image,
# so they silently stopped resolving the moment the base image moved.
FROM espressif/idf:release-v5.1

WORKDIR /project

# spiffs_image is not optional: main/CMakeLists.txt feeds it to
# spiffs_create_partition_image() to build the www_0 partition, so a tree
# without it fails to configure. The previous Dockerfile did not copy it.
#
# build/ and sdkconfig are deliberately not copied - a stale sdkconfig from the
# host would override sdkconfig.defaults and silently change the build.
COPY components components
COPY main main
COPY spiffs_image spiffs_image
COPY CMakeLists.txt partitions.csv sdkconfig.defaults ./

# Each RUN is its own shell, so export.sh has to be sourced in the same one
# that invokes the build. The previous version sourced it in a separate layer,
# where it had no effect.
RUN . "$IDF_PATH/export.sh" && idf.py build
