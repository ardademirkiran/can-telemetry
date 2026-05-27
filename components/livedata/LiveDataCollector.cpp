#include "LiveDataCollector.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "OBDLiveDataRequest.hpp"
#include "OBDSpeedRequest.hpp"
#include "OBDEngineRpmRequest.hpp"
#include "OBDEngineLoadRequest.hpp"
#include "OBDEngineCoolantTempRequest.hpp"
#include "OBDMassAirFlowRequest.hpp"
#include "OBDStoredDTCRequest.hpp"
#include <map>
#include <string>
#include "OBDResponse.hpp"
#include "cbor.h"
#include <string.h>
#include "CBORUtils.hpp"
#include "SDCardInterface.hpp"
#include "http_client.hpp"
#include "CANClient.hpp"
#include <sys/time.h>

void LiveDataCollector::mapPrinterTask(void *pv)
{
    while (status_ == CollectorStatus::RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI("SNAPSHOT", "%s, snapshotList: Size: %d", snap.toJson().c_str(), snapshotList_.size());
    }
    vTaskDelete(nullptr);
}

void LiveDataCollector::send_collected_snapshots(void *pv)
{
    while (status_ == CollectorStatus::RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(30000));

        if (!snapshotList_.empty())
        {
            size_t dataCborSize = 0;

            cborUtils_.convertCollectedDataIntoCBOR(snapshotList_, dataCBORBuffer, sizeof(dataCBORBuffer), &dataCborSize);

            bool httpSuccess = httpClient_.sendTelemetryData(dataCBORBuffer, dataCborSize);

            if (!httpSuccess)
            {
                sdCardInterface_.appendCborToSd(dataCBORBuffer, dataCborSize);
            }
            snapshotList_.clear();
        }
    }
}

LiveDataCollector::LiveDataCollector(
    CANClient &canClient,
    SDCardInterface &sdCardInterface,
    CBORUtils &cborUtils,
    TelemetryHTTPClient &httpClient)
    : healthy_(true),
      status_(CollectorStatus::READY),
      canClient_(canClient),
      cborUtils_(cborUtils),
      sdCardInterface_(sdCardInterface),
      httpClient_(httpClient) {}

void LiveDataCollector::save_snapshot()
{

    while (status_ == CollectorStatus::RUNNING)
    {
        if (healthy_)
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);

            uint64_t timestamp_ms =
                (uint64_t)tv.tv_sec * 1000ULL +
                (uint64_t)tv.tv_usec / 1000ULL;

            snap.setField("timestamp", timestamp_ms);

            ESP_LOGI(TAG, "Saving Snapshot. ts=%llu", timestamp_ms);

            snapshotList_.push_back(snap);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void LiveDataCollector::collect_DTC()
{
    ESP_LOGI(TAG, "-------------REQUESTING DTCS------------");
    vTaskDelay(pdMS_TO_TICKS(100));
    OBDStoredDTCRequest dtc_request{};
    OBDResponse dtc_response = canClient_.send_command(0x03, dtc_request.pid(), 0x7E0);
    if (dtc_response.getStatus() == OBDResponseStatus::OBD_OK)
    {
        std::string dtc_str = dtc_request.parse_returned_dtc_list(dtc_response.getData());
        ESP_LOGI(TAG, "Stored DTC Response: %s", dtc_str.c_str());
        snap.dtc = dtc_str;
    }
    is_dtc_enabled = false;
}

void LiveDataCollector::collect_data()
{
    while (status_ == CollectorStatus::RUNNING)
    {
        std::optional<OBDTask> taskOpt = requestQueue_.pop();
        if (taskOpt)
        {
            OBDTask task = *taskOpt;
            OBDResponse response = canClient_.send_command(0x01, task.request->pid(), 0x7E0);

            if (response.getStatus() == OBDResponseStatus::OBD_OK)
            {
                healthy_ = true;
                double value = task.request->parseResponse(response.getData());
                snap.setField(task.request->key(), value);
                if (is_dtc_enabled)
                    collect_DTC();
                uint32_t now = xTaskGetTickCount();
                task.priorityTick = now + pdMS_TO_TICKS(task.priorityMargin);
            }
            else
            {
                ESP_LOGI(TAG, "No response. State is changed to unhealthy");
                healthy_ = false;
            }
            requestQueue_.push(task);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void LiveDataCollector::stop()
{
    ESP_LOGI(TAG, "Suspending data collector tasks...");
    vTaskSuspend(collectorHandle_);
    ESP_LOGI(TAG, "Suspending data collector tasks...");

    vTaskSuspend(snapshotSaverHandle_);
    ESP_LOGI(TAG, "Suspending data collector tasks...");

    vTaskSuspend(mapPrinterHandle_);
    ESP_LOGI(TAG, "Suspending data collector tasks...");

    vTaskSuspend(dataSenderHandle_);
    ESP_LOGI(TAG, "Suspending data collector tasks...");

    vTaskSuspend(dtc_collect_enabler_handle_);
    ESP_LOGI(TAG, "Suspending data collector tasks...");

    status_ = CollectorStatus::SUSPENDED;
}

void LiveDataCollector::resume()
{
    ESP_LOGI(TAG, "Resuming...");
    healthy_ = true;
    status_ = CollectorStatus::RUNNING;
    vTaskResume(collectorHandle_);
    vTaskResume(snapshotSaverHandle_);
    vTaskResume(mapPrinterHandle_);
    vTaskResume(dataSenderHandle_);
    vTaskResume(dtc_collect_enabler_handle_);
}

void LiveDataCollector::kill()
{
    ESP_LOGI(TAG, "Killing...");
    vTaskDelete(collectorHandle_);
    vTaskDelete(snapshotSaverHandle_);
    vTaskDelete(mapPrinterHandle_);
    vTaskDelete(dataSenderHandle_);
    vTaskDelete(dtc_collect_enabler_handle_);
    requestQueue_.clear();
    snapshotList_.clear();
    snap = Snapshot();
    status_ = CollectorStatus::KILLED;
}

void LiveDataCollector::enable_DTC_collect(void *pv)
{

    while (status_ == CollectorStatus::RUNNING)
    {

        is_dtc_enabled = true;
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
    vTaskDelete(nullptr);
}

void LiveDataCollector::start()
{
    uint32_t now = xTaskGetTickCount();
    OBDTask rpmTask;
    rpmTask.request = std::make_shared<OBDEngineRPMRequest>();
    rpmTask.priorityMargin = 200;
    rpmTask.priorityTick = now;
    requestQueue_.push(rpmTask);

    OBDTask loadTask;
    loadTask.request = std::make_shared<OBDEngineLoadRequest>();
    loadTask.priorityMargin = 200;
    loadTask.priorityTick = now;
    requestQueue_.push(loadTask);

    OBDTask speedTask;
    speedTask.request = std::make_shared<OBDSpeedRequest>();
    speedTask.priorityMargin = 200;
    speedTask.priorityTick = now;
    requestQueue_.push(speedTask);

    OBDTask coolantTask;
    coolantTask.request = std::make_shared<OBDEngineCoolantTempRequest>();
    coolantTask.priorityMargin = 3000;
    coolantTask.priorityTick = now;
    requestQueue_.push(coolantTask);

    OBDTask mafTask;
    mafTask.request = std::make_shared<OBDMassAirFlowRequest>();
    mafTask.priorityMargin = 3000;
    mafTask.priorityTick = now;
    requestQueue_.push(mafTask);

    if (status_ == CollectorStatus::READY || status_ == CollectorStatus::KILLED)
    {
        status_ = CollectorStatus::RUNNING;
        healthy_ = true;

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->collect_data(); }, "scheduler_task", 4096, this, 5, &collectorHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->mapPrinterTask(pv); },
                    "map_printer_task",
                    4096,
                    this,
                    3,
                    &mapPrinterHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->send_collected_snapshots(pv); },
                    "bulk_send",
                    4096,
                    this,
                    3,
                    &dataSenderHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->save_snapshot(); }, "save_snapshot_task", 4096, this, 4, &snapshotSaverHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->enable_DTC_collect(pv); }, "enable_dtc_request", 4096, this, 4,
                    &dtc_collect_enabler_handle_);
        ESP_LOGI(TAG, "LiveDataGatherer started");
    }
}