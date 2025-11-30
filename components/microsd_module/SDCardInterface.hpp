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
    void initSdcard();
    void appendCborToSd(uint8_t *buffer, size_t len);
    bool readCborFromSd(uint8_t *buffer, size_t len, size_t *outSize, long offset);
};
