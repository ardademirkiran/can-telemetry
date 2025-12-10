#include "LiveDataCollectorMonitor.hpp"
#include "CANTrafficMonitor.hpp"
#include "SDDataSender.hpp"
#include "Monitors.hpp"

SDDataSender sdDataSender = SDDataSender();
CollectorMonitor collectorMonitor = CollectorMonitor();
CANTrafficMonitor canTrafficMonitor = CANTrafficMonitor(&sdDataSender);