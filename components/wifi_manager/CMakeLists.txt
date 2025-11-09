idf_component_register(
    SRCS "WifiManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        esp_wifi
        esp_event
        nvs_flash
        freertos
        esp_netif
)
