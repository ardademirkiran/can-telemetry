#include "SDDataSender.hpp"
#include "Globals.hpp"
#include "esp_log.h"

void SDDataSender::run()
{
    size_t dataLen = 0;
    fileOffset_ = nvsInterface.read_int_data("can_data_file_offset");
    while (sdCardInterface.readCborFromSd(dataBuffer_, sizeof(dataBuffer_), &dataLen, fileOffset_) && isRunning)
    {
        ESP_LOGI(SD_DATA_SENDER_TAG, "SD data sending started.");
        bool httpSuccess = httpClient.sendTelemetryData(dataBuffer_, dataLen);
        if (httpSuccess)
        {
            fileOffset_ += sizeof(uint32_t) + dataLen;
            nvsInterface.write_data("can_data_file_offset", fileOffset_);
        }
        else
        {
            ESP_LOGI(SD_DATA_SENDER_TAG, "An error occured while sending data. Exiting.");
            vTaskDelete(nullptr);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    isRunning = false;
    vTaskDelete(nullptr);
}

void SDDataSender::taskEntry(void *pv)
{
    SDDataSender *self = static_cast<SDDataSender *>(pv);

    self->run();
}

void SDDataSender::startTask()
{
    xTaskCreate(
        SDDataSender::taskEntry,
        "DATA_SENDER_SD",
        4096,
        this,
        3,
        nullptr);
}