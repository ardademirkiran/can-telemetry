#include "esp_http_client.h"
#include "CBORUtils.hpp"
#pragma once

class TelemetryHTTPClient
{
public:
    bool sendTelemetryData(uint8_t *dataCborBuffer, size_t dataSize);
    bool initClient();
    bool cleanUp();
    TelemetryHTTPClient(CBORUtils *cborUtils);

private:
    esp_http_client_handle_t clientHandle_;
    uint8_t HTTPCBORBuffer_[16384];
    CBORUtils *cborUtils_;
    static constexpr const char *TAG = "HTTP_CLIENT";
};