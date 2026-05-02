#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDEngineRPMRequest : public OBDLiveDataRequest
{
public:
  uint8_t pid() const override { return 0x0C; }
  std::string key() const override { return "rpm"; }

  double parseResponse(const std::vector<uint8_t> &data) const override
  {
    return ((data[2] * 256) + data[3]) / 4.0;
  }
};
