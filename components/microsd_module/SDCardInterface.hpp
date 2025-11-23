#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

class SDCardInterface
{
public:
    void init_sdcard();
    void write_and_read_example();
    void append_cbor_to_sd(uint8_t *buffer, size_t len);
    void read_cbor_from_sd(uint8_t *buffer, size_t len);
};
