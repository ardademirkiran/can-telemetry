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
                                      uint8_t *buffer,
                                      size_t bufferSize,
                                      size_t *outSize);

    void build_cbor_payload(const std::vector<Snapshot> &snapshotList, uint8_t *buffer, size_t bufferSize, size_t *outSize);

private:
    void encodeSnapshot(const Snapshot &snap, CborEncoder *parent);
};