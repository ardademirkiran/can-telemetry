
#include "CANClient.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LiveDataCollector.hpp"
#include "Globals.hpp"
#include "Monitors.hpp"
#include "WifiManager.hpp"
#include "SDCardInterface.hpp"
#include "rtc_driver.h"
#include <time.h>
#include <sys/time.h>

#include <sstream>

static constexpr const char *MAIN_TAG = "MAIN";

extern "C" void app_main()
{

    rtc_initialize_gpio();
    time_dto time_object;
    time_object.sec = 0;
    time_object.min = 47;
    time_object.hour = 21;
    time_object.date = 18;
    time_object.month = 1;
    time_object.year = 26;
    rtc_sync_clock_time(time_object);
    rtc_sync_device_time();

    WiFiManager wifi("NEZIH_DEMIRKIRAN_2.4G", "enesyamanbaba");
    wifi.init();

    if (wifi.connect())
    {
        ESP_LOGI("MAIN", "Wi-Fi connected successfully!");
    }
    else
    {
        ESP_LOGE("MAIN", "Wi-Fi connection failed!");
    }

    ESP_LOGI(MAIN_TAG, "Firmware start. Checking traffic...");
    sdCardInterface.initSdcard();
    canClient.setup_twai();
    httpClient.initClient();

    if (canClient.hasTraffic())
    {
        ESP_LOGI(MAIN_TAG, "Traffic detected, collector is started...");
        liveDataCollector.start();
        collectorMonitor.startTask();
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "No traffic, CAN Monitor is started...");
        canTrafficMonitor.startTask();
    }
}
