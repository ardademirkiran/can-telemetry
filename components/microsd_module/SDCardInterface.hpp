#pragma once

#include <stdio.h>
#include <string.h>

class SDCardInterface
{
public:
    void initSdcard();
    void appendCborToSd(uint8_t *buffer, size_t len);
    bool readCborFromSd(uint8_t *buffer, size_t len, size_t *outSize, long offset);
};
