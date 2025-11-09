#pragma once
#include "OBDTask.hpp"
#include "OBDPriorityQueue.hpp"
#include "CANClient.hpp"
#include "CollectorStatus.hpp"
class LiveDataCollector
{
public:
    LiveDataCollector(CANClient *canClient);
    void start();
    void kill();
    void stop();
    void resume();
    bool healthy_;
    CollectorStatus status_;

private:
    void mapPrinterTask(void *pv);
    void saveSnapshot();
    void collectData();
    CANClient *canClient_;
    OBDPriorityQueue requestQueue_;
    TaskHandle_t collectorHandle_;
    TaskHandle_t snapshotSaverHandle_;
    TaskHandle_t mapPrinterHandle_;
    TaskHandle_t dataSenderHandle_;
};
