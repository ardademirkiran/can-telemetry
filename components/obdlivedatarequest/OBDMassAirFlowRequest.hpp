#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDMassAirFlowRequest : public OBDLiveDataRequest
{
public:
    u_int8_t pid() const override { return 0x10; }
    std::string key() const override { return "maf"; }

    double parseResponse(const uint8_t *data, size_t length) const override
    {
        if (length < 3)
            return -1.0;
        int A = data[3];
        int B = data[4];
        return ((A * 256) + B) / 100;
    }
};
