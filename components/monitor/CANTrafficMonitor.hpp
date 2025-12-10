#pragma once
#include "Monitors.hpp"

class SDDataSender;

class CANTrafficMonitor
{
public:
    void startTask();
    CANTrafficMonitor(SDDataSender *sdDataSender);

private:
    void run();
    static void taskEntry(void *pv);
    const char *CAN_WATCHER_TAG = "CAN_WATCHER";
    SDDataSender *sdDataSender_;
};
