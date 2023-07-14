#!/usr/bin/bash

esptool.py --chip esp32 merge_bin -o build/DeepDeck_single.bin --flash_mode dio --flash_size 4MB 0x1000 build/bootloader/bootloader.bin 0x20000 build/DeepDeck.bin 0x8000 build/partition_table/partition-table.bin 0x15000 build/ota_data_initial.bin 0x317000 build/www_0.bin