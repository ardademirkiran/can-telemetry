#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDEngineCoolantTempRequest : public OBDLiveDataRequest
{
public:
  uint8_t pid() const override { return 0x05; }
  std::string key() const override { return "engineCoolantTemp"; }

  double parseResponse(const std::vector<uint8_t> &data) const override
  {
    return data[2] - 40.0;
  }
};
