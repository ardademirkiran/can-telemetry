#include "CANTrafficMonitor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "OBDResponse.hpp"
#include "LiveDataCollectorMonitor.hpp"
#include "Globals.hpp"

static constexpr const char *CAN_WATCHER_TAG = "CAN_WATCHER";

void monitorCANTraffic(void *pv)
{
    while (liveDataCollector.status_ == CollectorStatus::KILLED || liveDataCollector.status_ == CollectorStatus::READY)
    {
        ESP_LOGI(CAN_WATCHER_TAG, "CAN Watcher is now running.");
        if (canClient.hasTraffic())
        {
            ESP_LOGI(CAN_WATCHER_TAG, "CAN Watcher detected traffic. Restarting the collector...");
            liveDataCollector.start();
            startLiveDataCollectorMonitor();
            vTaskDelete(nullptr);
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
    vTaskDelete(nullptr);
}

void startCANTrafficMonitor()
{
    xTaskCreate(monitorCANTraffic, "CANMonitor", 4096, nullptr, 3, nullptr);
}