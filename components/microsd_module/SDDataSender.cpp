#include "SDDataSender.hpp"

static uint8_t dataBuffer[8192];

void sendDataFromSd(void *pv)
{
    size_t fileOffset = 0;
    size_t dataLen = 0;
    while (sdCardInterface.readCborFromSd(dataBuffer, sizeof(dataBuffer), &dataLen, fileOffset))
    {
        bool httpSuccess = httpClient.sendTelemetryData(dataBuffer, dataLen);
        if (httpSuccess)
        {
            fileOffset += sizeof(uint32_t) + dataLen;
        }
        else
        {
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    ESP_LOGI("SD_DATA_SENDER", "No data left in the SD. Exiting.");
    vTaskDelete(nullptr);
}

void startDataSenderTask()
{
    xTaskCreate(sendDataFromSd, "DATA_SENDER_SD", 4096, nullptr, 3, nullptr);
}