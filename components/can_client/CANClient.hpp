#ifndef CANCLIENT_HPP
#define CANCLIENT_HPP

#include <cstdint>
#include "OBDResponse.hpp"

class CANClient
{
public:
    CANClient();
    ~CANClient();

    void setup_twai();
    bool hasTraffic();
    OBDResponse sendCommand(uint8_t pid, uint32_t ecuId);
};

#endif // CANCLIENT_HPP
