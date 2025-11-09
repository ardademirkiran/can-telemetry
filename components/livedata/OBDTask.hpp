#pragma once
#include <memory>

class OBDLiveDataRequest;

struct OBDTask
{
    std::shared_ptr<OBDLiveDataRequest> request;
    u_int32_t priorityMargin;
    u_int32_t priorityTick;
};
