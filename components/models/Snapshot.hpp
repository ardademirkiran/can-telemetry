#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <stdexcept>

class Snapshot
{
public:
    uint64_t timestamp;
    double rpm;
    double speed;
    double engineCoolantTemp;
    double maf;
    double engineLoad;

    Snapshot()
    {
        timestamp = 0;
        rpm = 0;
        speed = 0;
        engineCoolantTemp = 0;
        maf = 0;
        engineLoad = 0;
    }

    void setField(const std::string &key, double value)
    {
        if (key == "timestamp")
        {
            timestamp = value;
        }
        if (key == "rpm")
        {
            rpm = value;
        }
        else if (key == "speed")
        {
            speed = value;
        }
        else if (key == "engineCoolantTemp")
        {
            engineCoolantTemp = value;
        }
        else if (key == "maf")
        {
            maf = value;
        }
        else if (key == "engineLoad")
        {
            engineLoad = value;
        }
    }

    std::string toJson() const
    {
        std::ostringstream ss;
        ss << "{"
           << "\"timestamp\":" << timestamp << ","
           << "\"rpm\":" << rpm << ","
           << "\"speed\":" << speed << ","
           << "\"engineCoolantTemp\":" << engineCoolantTemp << ","
           << "\"maf\":" << maf << ","
           << "\"engineLoad\":" << engineLoad
           << "}";
        return ss.str();
    }
};
