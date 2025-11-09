
#include "CANClient.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LiveDataCollector.hpp"
#include "Globals.hpp"
#include "LiveDataCollectorMonitor.hpp"
#include "CANTrafficMonitor.hpp"
#include "WifiManager.hpp"

#include <sstream>

static constexpr const char *MAIN_TAG = "MAIN";

extern "C" void app_main()
{

    /*WiFiManager wifi("arda", "arda2001");
    wifi.init();

    if (wifi.connect())
    {
        ESP_LOGI("MAIN", "Wi-Fi connected successfully!");
    }
    else
    {
        ESP_LOGE("MAIN", "Wi-Fi connection failed!");
    }*/

    ESP_LOGI(MAIN_TAG, "Firmware start. Checking traffic...");
    canClient.setup_twai();
    if (canClient.hasTraffic())
    {
        ESP_LOGI(MAIN_TAG, "Traffic detected, collector is started...");
        liveDataCollector.start();
        startLiveDataCollectorMonitor();
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "No traffic, CAN Monitor is started...");
        startCANTrafficMonitor();
    }
}
