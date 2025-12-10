#include "Globals.hpp"

CANClient canClient = CANClient();
SDCardInterface sdCardInterface = SDCardInterface();
CBORUtils cborUtils = CBORUtils();
TelemetryHTTPClient httpClient = TelemetryHTTPClient(&cborUtils);
LiveDataCollector liveDataCollector = LiveDataCollector(&canClient, &sdCardInterface, &cborUtils, &httpClient);