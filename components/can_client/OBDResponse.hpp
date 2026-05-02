#pragma once

#include "OBDResponseStatus.hpp"
#include <cstring>
#include <vector>

class OBDResponse
{
private:
    OBDResponseStatus status;
    uint32_t id;
    uint8_t pid;
    std::vector<uint8_t> payload_vector;
    uint16_t length;

public:
    OBDResponse()
    {
        status = OBDResponseStatus::OBD_ERROR;
        id = 0;
        pid = 0;
        length = 0;
    }

    OBDResponse(OBDResponseStatus s, uint32_t ecuId, uint8_t p, std::vector<uint8_t> payload, uint16_t len)
        : status(s), id(ecuId), pid(p), payload_vector(payload), length(len) {}

    OBDResponseStatus getStatus() const { return status; }
    uint32_t getId() const { return id; }
    uint8_t getPid() const { return pid; }
    uint8_t getLength() const { return length; }
    const std::vector<uint8_t> getData() const { return payload_vector; }

    void setStatus(OBDResponseStatus s) { status = s; }
    void setId(uint32_t ecuId) { id = ecuId; }
    void setPid(uint8_t p) { pid = p; }
};