#pragma once

enum class CANClientStatus
{
    REQUEST_SENT,
    FIRST_FRAME_RECEIVED,
    MORE_MULTI_FRAME,
};