#pragma once
#include "OBDLiveDataRequest.hpp"
#include <cstdint>

class OBDStoredDTCRequest
{
public:
    std::string key() const
    {
        return "DTC";
    }
    uint8_t pid() const
    {
        return 0x00;
    }

    std::string parse_returned_dtc_list(const std::vector<uint8_t> &payload)
    {
        std::string dtc_list_str = "-";
        for (int i = 2; i < payload.size(); i += 2)
        {
            dtc_list_str.append(decode_dtc(payload[i], payload[i + 1]) + "-");
        }

        return dtc_list_str;
    }

    std::string decode_dtc(uint8_t a, uint8_t b)
    {
        char systems[] = {'P', 'C', 'B', 'U'};

        char first = systems[(a & 0xC0) >> 6];

        uint8_t d1 = (a & 0x30) >> 4;
        uint8_t d2 = a & 0x0F;
        uint8_t d3 = (b & 0xF0) >> 4;
        uint8_t d4 = b & 0x0F;

        std::string result;

        result += first;
        result += '0' + d1;

        auto nibble_to_hex = [](uint8_t n) -> char
        {
            return (n < 10) ? ('0' + n) : ('A' + (n - 10));
        };

        result += nibble_to_hex(d2);
        result += nibble_to_hex(d3);
        result += nibble_to_hex(d4);

        return result;
    }
};