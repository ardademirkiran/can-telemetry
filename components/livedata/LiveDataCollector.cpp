#include "LiveDataCollector.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "OBDLiveDataRequest.hpp"
#include "OBDSpeedRequest.hpp"
#include "OBDEngineRpmRequest.hpp"
#include "OBDEngineLoadRequest.hpp"
#include "OBDEngineCoolantTempRequest.hpp"
#include "OBDMassAirFlowRequest.hpp"
#include <map>
#include <string>
#include "OBDResponse.hpp"
#include "esp_http_client.h"
#include "cbor.h"
#include <string.h>
#include "CBORUtils.hpp"

static constexpr const char *TAG = "LIVE_DATA_COLLECTOR";

void LiveDataCollector::mapPrinterTask(void *pv)
{
    while (status_ == CollectorStatus::RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI("SNAPSHOT", "%s, snapshotList: Size: %d", snap.toJson().c_str(), snapshotList.size());
    }
    vTaskDelete(nullptr);
}

void LiveDataCollector::sendBulkSnapshots(void *pv)
{
    while (status_ == CollectorStatus::RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(30000));

        if (snapshotList.empty())
        {
            continue;
        }

        // Allocate CBOR buffer
        size_t cborSize = 0;

        // Build CBOR data
        cborUtils_.build_cbor_payload(snapshotList, HTTPCBORBuffer, sizeof(HTTPCBORBuffer), &cborSize);

        ESP_LOGI("CBOR", "CBOR size: %d bytes", cborSize);

        // Send request
        esp_http_client_config_t config = {
            .url = "http://192.168.0.19:8080/telemetry/ingest",
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "application/cbor");
        esp_http_client_set_post_field(client,
                                       (char *)HTTPCBORBuffer,
                                       cborSize);

        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI("BULK", "Upload OK, HTTP %d",
                     esp_http_client_get_status_code(client));
            snapshotList.clear();
        }
        else
        {
            ESP_LOGE("BULK", "Upload failed: %s",
                     esp_err_to_name(err));
            size_t sdCborLen = 0;

            cborUtils_.convertCollectedDataIntoCBOR(snapshotList, SDCBORBuffer, sizeof(SDCBORBuffer), &sdCborLen);
            sdCardInterface_.append_cbor_to_sd(SDCBORBuffer, sdCborLen);
            snapshotList.clear();
        }

        esp_http_client_cleanup(client);
    }
}

LiveDataCollector::LiveDataCollector(CANClient *canClient)
{
    canClient_ = canClient;
    status_ = CollectorStatus::READY;
    healthy_ = true;
}

void LiveDataCollector::saveSnapshot()
{
    while (status_ == CollectorStatus::RUNNING)
    {
        uint32_t ticks = xTaskGetTickCount();
        uint64_t elapsed_ms = (uint64_t)ticks * portTICK_PERIOD_MS;
        snap.setField("timestamp", elapsed_ms);
        ESP_LOGI(TAG, "Saving Snapshot.");
        snapshotList.push_back(snap);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void LiveDataCollector::collectData()
{
    while (status_ == CollectorStatus::RUNNING)
    {
        std::optional<OBDTask> taskOpt = requestQueue_.pop();
        if (taskOpt)
        {
            OBDTask task = *taskOpt;
            OBDResponse response = canClient_->sendCommand(task.request->pid(), 0x7E8);

            if (response.getStatus() == OBDResponseStatus::OBD_OK)
            {
                healthy_ = true;
                double value = task.request->parseResponse(response.getData(), response.getLength());
                snap.setField(task.request->key(), value);
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
    ESP_LOGI(TAG, "Stopping...");

    ESP_LOGI(TAG, "Suspending collectorHandle_ = %p", collectorHandle_);
    vTaskSuspend(collectorHandle_);

    ESP_LOGI(TAG, "Suspending snapshotSaverHandle_ = %p", snapshotSaverHandle_);
    vTaskSuspend(snapshotSaverHandle_);
    vTaskSuspend(mapPrinterHandle_);

    ESP_LOGI(TAG, "Suspending dataSenderHandle_ = %p", dataSenderHandle_);
    vTaskSuspend(dataSenderHandle_);

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
}

void LiveDataCollector::kill()
{
    ESP_LOGI(TAG, "Killing...");
    vTaskDelete(collectorHandle_);
    vTaskDelete(snapshotSaverHandle_);
    vTaskDelete(mapPrinterHandle_);
    vTaskDelete(dataSenderHandle_);
    requestQueue_.clear();
    snapshotList.clear();
    snap = Snapshot();
    status_ = CollectorStatus::KILLED;
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
        sdCardInterface_.init_sdcard();

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->collectData(); }, "scheduler_task", 4096, this, 5, &collectorHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->mapPrinterTask(pv); },
                    "map_printer_task",
                    4096,
                    this,
                    3,
                    &mapPrinterHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->sendBulkSnapshots(pv); },
                    "bulk_send",
                    4096,
                    this,
                    3,
                    &dataSenderHandle_);

        xTaskCreate([](void *pv)
                    { static_cast<LiveDataCollector *>(pv)->saveSnapshot(); }, "save_snapshot_task", 4096, this, 4, &snapshotSaverHandle_);
        ESP_LOGI(TAG, "LiveDataGatherer started");
    }
}