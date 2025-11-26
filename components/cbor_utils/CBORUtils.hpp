#pragma once

#include <stdio.h>
#include <string.h>
#include "Snapshot.hpp"
#include "cbor.h"
#include <vector>

class CBORUtils
{
public:
    void convertCollectedDataIntoCBOR(const std::vector<Snapshot> &snapshotList,
                                      uint8_t *dataBuffer,
                                      size_t dataBufferSize,
                                      size_t *outSize);

    void build_cbor_payload(uint8_t *httpBuffer, uint8_t *dataBuffer, size_t httpBufferSize, size_t dataBufferSize, size_t *outSize);

private:
    void encodeSnapshot(const Snapshot &snap, CborEncoder *parent);
};