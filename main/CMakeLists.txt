set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(
    SRCS "AppMain.cpp"
    INCLUDE_DIRS "."
    REQUIRES can_client obdlivedatarequest livedata freertos driver monitor wifi_manager esp_wifi esp_event freertos microsd_module cbor_utils esp_netif
)