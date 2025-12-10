#pragma once
#include "Monitors.hpp"

class CollectorMonitor
{
public:
    void startTask();

private:
    void run();
    static void taskEntry(void *pv);
    const char *COLLECTOR_MONITOR_TAG = "COLLECTOR_MONITOR";
};