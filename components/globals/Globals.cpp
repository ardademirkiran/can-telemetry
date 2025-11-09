#include "Globals.hpp"

CANClient canClient = CANClient();
LiveDataCollector liveDataCollector = LiveDataCollector(&canClient);
