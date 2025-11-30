#include "http_client.hpp"
#include "esp_http_client.h"
#include "esp_log.h"

bool TelemetryHTTPClient::sendTelemetryData(uint8_t *dataCborBuffer, size_t dataSize)
{

    if (!clientHandle_)
    {
        return false;
    }

    size_t httpCborSize = 0;

    cborUtils_->build_cbor_payload(HTTPCBORBuffer_, dataCborBuffer, sizeof(HTTPCBORBuffer_), sizeof(dataSize), &httpCborSize);

    esp_http_client_set_post_field(clientHandle_,
                                   (char *)HTTPCBORBuffer_,
                                   httpCborSize);

    esp_err_t err = esp_http_client_perform(clientHandle_);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Telemetry data is sent.");
        return true;
    }
    else
    {
        ESP_LOGI(TAG, "Can not send telemetry data.");
        return false;
    }
}

TelemetryHTTPClient::TelemetryHTTPClient(CBORUtils *cborUtils)
{
    cborUtils_ = cborUtils;
}

bool TelemetryHTTPClient::initClient()
{
    esp_http_client_config_t config = {
        .url = "http://192.178.0.19:8080/telemetry/ingest",
        .timeout_ms = 5000,
        .buffer_size_tx = 2048,
        .keep_alive_enable = true,
    };

    clientHandle_ = esp_http_client_init(&config);
    esp_http_client_set_method(clientHandle_, HTTP_METHOD_POST);
    esp_http_client_set_header(clientHandle_, "Content-Type", "application/cbor");
    return true;
}

bool TelemetryHTTPClient::cleanUp()
{
    if (!clientHandle_)
    {
        return false;
    }

    esp_http_client_cleanup(clientHandle_);
    return true;
}