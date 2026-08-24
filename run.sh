#!/usr/bin/bash

# Interactive shell in the build image, with ESP-IDF already exported.
# Note: flashing from inside the container needs the serial device passed
# through, e.g. --device=/dev/ttyUSB0 on Linux. On macOS, flash from the host.
docker run --rm -i -t deepdeck:latest /bin/bash
