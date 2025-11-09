#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDSpeedRequest : public OBDLiveDataRequest
{
public:
    uint8_t pid() const override { return 0x0D; }
    std::string key() const override { return "speed"; }

    double parseResponse(const uint8_t *data, size_t length) const override
    {
        if (length < 3)
            return -1.0;
        int A = data[3];
        return A;
    }
};
