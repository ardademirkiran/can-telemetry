idf_component_register(
    SRCS
        OBDPriorityQueue.cpp
        LiveDataCollector.cpp
    INCLUDE_DIRS
        "."
    REQUIRES obdlivedatarequest models can_client cbor cbor_utils globals)
