#pragma once
#include "OBDTask.hpp"
#include "OBDPriorityQueue.hpp"
#include "CANClient.hpp"
#include "CollectorStatus.hpp"
#include "Snapshot.hpp"
#include <vector>
#include "CBORUtils.hpp"
#include "SDCardInterface.hpp"

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
    void saveSnapshots(void *pv);
    void saveSnapshot();
    void collectData();
    CANClient *canClient_;
    OBDPriorityQueue requestQueue_;
    TaskHandle_t collectorHandle_;
    TaskHandle_t snapshotSaverHandle_;
    TaskHandle_t mapPrinterHandle_;
    TaskHandle_t dataSenderHandle_;
    Snapshot snap;
    CBORUtils cborUtils_;
    SDCardInterface sdCardInterface_;
    uint8_t dataCBORBuffer[8192];
    uint8_t HTTPCBORBuffer[16384];

    std::vector<Snapshot> snapshotList_;

    const char *deviceId = "123456789";
};
