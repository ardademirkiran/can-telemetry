#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdlib>

class OBDEngineLoadRequest : public OBDLiveDataRequest
{
public:
  uint8_t pid() const override { return 0x04; }
  std::string key() const override { return "engineLoad"; }

  double parseResponse(const std::vector<uint8_t> &data) const override
  {
    return (data[2] * 100) / 255;
  }
};
