#include "WifiManager.hpp"
#include "esp_log.h"

static const char *TAG = "WiFiManager";

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
WiFiManager::WiFiManager(const std::string &ssid, const std::string &password)
    : ssid_(ssid), password_(password)
{
    wifi_event_group_ = xEventGroupCreate();
}

// -----------------------------------------------------------------------------
// Event handler
// -----------------------------------------------------------------------------
void WiFiManager::eventHandler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    WiFiManager *self = static_cast<WiFiManager *>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retryCount_ < MAX_RETRY)
        {
            esp_wifi_connect();
            retryCount_++;
            ESP_LOGI(TAG, "Retrying connection to the AP...");
        }
        else
        {
            xEventGroupSetBits(self->wifi_event_group_, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount_ = 0;
        xEventGroupSetBits(self->wifi_event_group_, WIFI_CONNECTED_BIT);
    }
}

// -----------------------------------------------------------------------------
// Initialize Wi-Fi stack
// -----------------------------------------------------------------------------
void WiFiManager::init()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &WiFiManager::eventHandler,
                                                        this,
                                                        nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &WiFiManager::eventHandler,
                                                        this,
                                                        nullptr));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid_.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password_.c_str(), sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized in STA mode.");
}

// -----------------------------------------------------------------------------
// Connect to the Wi-Fi network
// -----------------------------------------------------------------------------
bool WiFiManager::connect()
{
    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid_.c_str());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group_,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected successfully to SSID: %s", ssid_.c_str());
        return true;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGW(TAG, "Failed to connect to SSID: %s", ssid_.c_str());
        return false;
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected Wi-Fi event");
        return false;
    }
}
