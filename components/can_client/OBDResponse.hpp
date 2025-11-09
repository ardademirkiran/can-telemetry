#pragma once

#include "OBDResponseStatus.hpp"
#include <cstring>

class OBDResponse
{
private:
    OBDResponseStatus status;
    uint32_t id;
    uint8_t pid;
    uint8_t data[8];
    uint8_t length;

public:
    OBDResponse()
    {
        status = OBDResponseStatus::OBD_ERROR;
        id = 0;
        pid = 0;
        length = 0;
        memset(data, 0, sizeof(data));
    }

    OBDResponse(OBDResponseStatus s, uint32_t ecuId, uint8_t p, const uint8_t *d, uint8_t len)
    {
        status = s;
        id = ecuId;
        pid = p;
        length = len;
        memcpy(data, d, len);
    }

    OBDResponseStatus getStatus() const { return status; }
    uint32_t getId() const { return id; }
    uint8_t getPid() const { return pid; }
    uint8_t getLength() const { return length; }
    const uint8_t *getData() const { return data; }

    void setStatus(OBDResponseStatus s) { status = s; }
    void setId(uint32_t ecuId) { id = ecuId; }
    void setPid(uint8_t p) { pid = p; }
    void setData(const uint8_t *d, uint8_t len)
    {
        length = len;
        memcpy(data, d, len);
    }
};