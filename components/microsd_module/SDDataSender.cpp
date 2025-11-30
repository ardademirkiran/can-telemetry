#include "SDDataSender.hpp"

void SDDataSender::sendDataFromSd(void *pv)
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
    }
}