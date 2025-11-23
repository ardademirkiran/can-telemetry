#include "CBORUtils.hpp"

void CBORUtils::encodeSnapshot(const Snapshot &snap, CborEncoder *parent)
{
    CborEncoder itemMap;
    cbor_encoder_create_map(parent, &itemMap, 6);

    cbor_encode_text_stringz(&itemMap, "timestamp");
    cbor_encode_uint(&itemMap, snap.timestamp);

    cbor_encode_text_stringz(&itemMap, "rpm");
    cbor_encode_float(&itemMap, snap.rpm);

    cbor_encode_text_stringz(&itemMap, "speed");
    cbor_encode_float(&itemMap, snap.speed);

    cbor_encode_text_stringz(&itemMap, "engineCoolantTemp");
    cbor_encode_float(&itemMap, snap.engineCoolantTemp);

    cbor_encode_text_stringz(&itemMap, "maf");
    cbor_encode_float(&itemMap, snap.maf);

    cbor_encode_text_stringz(&itemMap, "engineLoad");
    cbor_encode_float(&itemMap, snap.engineLoad);

    cbor_encoder_close_container(parent, &itemMap);
}

void CBORUtils::convertCollectedDataIntoCBOR(const std::vector<Snapshot> &snapshotList,
                                             uint8_t *buffer,
                                             size_t bufferSize,
                                             size_t *outSize)
{
    CborEncoder encoder, array;

    cbor_encoder_init(&encoder, buffer, bufferSize, 0);
    cbor_encoder_create_array(&encoder, &array, CborIndefiniteLength);

    for (const auto &snap : snapshotList)
        encodeSnapshot(snap, &array);

    cbor_encoder_close_container(&encoder, &array);

    *outSize = cbor_encoder_get_buffer_size(&encoder, buffer);
}

void CBORUtils::build_cbor_payload(const std::vector<Snapshot> &snapshotList, uint8_t *buffer, size_t bufferSize, size_t *outSize)
{
    CborEncoder encoder, root, array;

    cbor_encoder_init(&encoder, buffer, bufferSize, 0);

    cbor_encoder_create_map(&encoder, &root, 2);

    cbor_encode_text_stringz(&root, "deviceId");
    cbor_encode_text_stringz(&root, "123456789");

    cbor_encode_text_stringz(&root, "dataList");
    cbor_encoder_create_array(&root, &array, CborIndefiniteLength);

    for (const auto &snap : snapshotList)
        encodeSnapshot(snap, &array);

    cbor_encoder_close_container(&root, &array);
    cbor_encoder_close_container(&encoder, &root);

    *outSize = cbor_encoder_get_buffer_size(&encoder, buffer);
}
