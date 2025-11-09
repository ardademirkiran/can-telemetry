// OBDLiveDataRequest.hpp
#pragma once
#include <string>
#include <vector>

class OBDLiveDataRequest
{
public:
    virtual ~OBDLiveDataRequest() = default;
    virtual std::string key() const = 0;
    virtual uint8_t pid() const = 0;
    virtual double parseResponse(const uint8_t *data, size_t length) const = 0;
};
