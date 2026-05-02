#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDMassAirFlowRequest : public OBDLiveDataRequest
{
public:
  uint8_t pid() const override { return 0x10; }
  std::string key() const override { return "maf"; }

  double parseResponse(const std::vector<uint8_t> &data) const override
  {
    return ((data[2] * 256) + data[3]) / 100;
  }
};
