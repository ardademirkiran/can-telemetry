#pragma once
#include <stdint.h>
#include "Globals.hpp"

class SDDataSender
{
public:
    void sendDataFromSd(void *pv);

private:
    uint8_t dataBuffer[8192];
};