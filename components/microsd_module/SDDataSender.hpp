#pragma once
#include <stdint.h>
#include <cstddef>

class SDDataSender
{
public:
    void startTask();
    bool isRunning = false;

private:
    void run();
    static void taskEntry(void *pv);
    const char *SD_DATA_SENDER_TAG = "SD_DATA_SENDER";
    uint8_t dataBuffer_[8192];
    size_t fileOffset_ = 0;
};