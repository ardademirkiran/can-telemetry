#pragma once
#include "CANClient.hpp"
#include "LiveDataCollector.hpp"
#include "SDCardInterface.hpp"
#include "http_client.hpp"
#include "CANTrafficMonitor.hpp"
#include "LiveDataCollectorMonitor.hpp"

extern CANClient canClient;
extern SDCardInterface sdCardInterface;
extern CBORUtils cborUtils;
extern LiveDataCollector liveDataCollector;
extern TelemetryHTTPClient httpClient;