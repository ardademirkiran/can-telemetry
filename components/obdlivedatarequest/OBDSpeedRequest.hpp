#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDSpeedRequest : public OBDLiveDataRequest
{
public:
  uint8_t pid() const override { return 0x0D; }
  std::string key() const override { return "speed"; }

  double parseResponse(const std::vector<uint8_t> &data) const override
  {
    return data[2];
  }
};
