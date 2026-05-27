#pragma once
#include "OBDTask.hpp"
#include "OBDPriorityQueue.hpp"
#include "CANClient.hpp"
#include "CollectorStatus.hpp"
#include "Snapshot.hpp"
#include <vector>
#include "CBORUtils.hpp"
#include "SDCardInterface.hpp"
#include "http_client.hpp"
#include "atomic"

class CANClient;
class CBORUtils;
class SDCardInterface;
class TelemetryHTTPClient;

class LiveDataCollector
{
public:
    LiveDataCollector(CANClient &canClient, SDCardInterface &SDCardInterface, CBORUtils &cborUtils, TelemetryHTTPClient &httpClient);
    void start();
    void kill();
    void stop();
    void resume();
    bool healthy_;
    CollectorStatus status_;

private:
    CANClient &canClient_;
    CBORUtils &cborUtils_;
    SDCardInterface &sdCardInterface_;
    TelemetryHTTPClient &httpClient_;

    OBDPriorityQueue requestQueue_;
    Snapshot snap;

    TaskHandle_t collectorHandle_;
    TaskHandle_t snapshotSaverHandle_;
    TaskHandle_t mapPrinterHandle_;
    TaskHandle_t dataSenderHandle_;
    TaskHandle_t dtc_collect_enabler_handle_;

    void mapPrinterTask(void *pv);
    void send_collected_snapshots(void *pv);
    void save_snapshot();
    void collect_data();
    void enable_DTC_collect(void *pv);
    void collect_DTC();

    uint8_t dataCBORBuffer[8192];
    uint8_t HTTPCBORBuffer[16384];
    std::atomic<bool> is_dtc_enabled = false;

    std::vector<Snapshot> snapshotList_;

    const char *TAG = "LIVE_DATA_COLLECTOR";

    const char *deviceId = "123456789";
};
