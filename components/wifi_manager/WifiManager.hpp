#ifndef WIFI_MANAGER_HPP
#define WIFI_MANAGER_HPP

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string>

class WiFiManager
{
public:
    WiFiManager(const std::string &ssid, const std::string &password);
    void init();
    bool connect();

private:
    std::string ssid_;
    std::string password_;

    EventGroupHandle_t wifi_event_group_;
    static constexpr int WIFI_CONNECTED_BIT = BIT0;
    static constexpr int WIFI_FAIL_BIT = BIT1;

    static void eventHandler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data);

    static inline int retryCount_ = 0;
    static constexpr int MAX_RETRY = 5;
};

#endif // WIFI_MANAGER_HPP
