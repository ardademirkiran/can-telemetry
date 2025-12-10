#include "LiveDataCollectorMonitor.hpp"
#include "CANTrafficMonitor.hpp"
#include "Globals.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Globals.hpp"

void CollectorMonitor::run()
{
    while (liveDataCollector.status_ == CollectorStatus::RUNNING)
    {
        ESP_LOGI(COLLECTOR_MONITOR_TAG, "Collector health check.");
        if (!liveDataCollector.healthy_)
        {
            ESP_LOGI(COLLECTOR_MONITOR_TAG, "Collector is not healthy. Trying to save...");
            liveDataCollector.stop();
            ESP_LOGI(COLLECTOR_MONITOR_TAG, "Collector is stopped.");
            vTaskDelay(pdMS_TO_TICKS(1000));
            int retryCount = 0;
            while (retryCount < 3)
            {
                ESP_LOGI(COLLECTOR_MONITOR_TAG, "Trying to detect traffic... Attempt %d", retryCount);
                if (canClient.hasTraffic())
                {
                    ESP_LOGI(COLLECTOR_MONITOR_TAG, "CAN traffic detected, resuming the collector.");
                    liveDataCollector.resume();
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(5000));
                retryCount++;
            }

            vTaskDelay(pdMS_TO_TICKS(5000));
            if (!liveDataCollector.healthy_)
            {
                ESP_LOGI(COLLECTOR_MONITOR_TAG, "CAN not detect traffic, killing the collector...");
                liveDataCollector.kill();
                canTrafficMonitor.startTask();
                vTaskDelete(nullptr);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    vTaskDelete(nullptr);
}

void CollectorMonitor::taskEntry(void *pv)
{
    CollectorMonitor *self = static_cast<CollectorMonitor *>(pv);
    self->run();
}

void CollectorMonitor::startTask()
{
    xTaskCreate(CollectorMonitor::taskEntry,
                "LiveDataMonitor",
                4096,
                this,
                3,
                nullptr);
}