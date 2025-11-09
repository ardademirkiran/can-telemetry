#include "LiveDataCollectorMonitor.hpp"
#include "Globals.hpp"
#include "CANTrafficMonitor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static constexpr const char *COLLECTOR_MONITOR_TAG = "COLLECTOR_MONITOR";

void monitorLiveDataCollector(void *pv)
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
                startCANTrafficMonitor();
                vTaskDelete(nullptr);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    vTaskDelete(nullptr);
}

void startLiveDataCollectorMonitor()
{
    xTaskCreate(monitorLiveDataCollector, "LiveDataMonitor", 4096, nullptr, 3, nullptr);
}