#!/usr/bin/bash

# Build the firmware in a container, using the ESP-IDF version this project
# targets. See the Dockerfile for why this does not need a local toolchain.
docker build -t deepdeck:latest .
