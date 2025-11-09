#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDEngineRPMRequest : public OBDLiveDataRequest
{
public:
    uint8_t pid() const override { return 0x0C; }
    std::string key() const override { return "rpm"; }

    double parseResponse(const uint8_t *data, size_t length) const override
    {
        if (length < 4)
            return -1.0;
        int A = data[3];
        int B = data[4];
        return ((A * 256) + B) / 4.0;
    }
};
