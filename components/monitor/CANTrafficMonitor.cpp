#include "CANTrafficMonitor.hpp"
#include "LiveDataCollector.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "OBDResponse.hpp"
#include "Globals.hpp"
#include "SDDataSender.hpp"

CANTrafficMonitor::CANTrafficMonitor(SDDataSender *sdDataSender)
{
    sdDataSender_ = sdDataSender;
}

void CANTrafficMonitor::startTask()
{
    xTaskCreate(
        CANTrafficMonitor::taskEntry,
        "CAN_MONITOR",
        4096,
        this,
        3,
        nullptr);
}

void CANTrafficMonitor::run()
{
    sdDataSender_->isRunning = true;
    sdDataSender_->startTask();
    while (liveDataCollector.status_ == CollectorStatus::KILLED || liveDataCollector.status_ == CollectorStatus::READY)
    {
        ESP_LOGI(CAN_WATCHER_TAG, "CAN Watcher is now running.");
        if (canClient.hasTraffic())
        {
            ESP_LOGI(CAN_WATCHER_TAG, "CAN Watcher detected traffic. Restarting the collector...");
            sdDataSender_->isRunning = false;
            liveDataCollector.start();
            collectorMonitor.startTask();
            vTaskDelete(nullptr);
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
    vTaskDelete(nullptr);
}

void CANTrafficMonitor::taskEntry(void *pv)
{
    CANTrafficMonitor *self = static_cast<CANTrafficMonitor *>(pv);

    self->run();
}
