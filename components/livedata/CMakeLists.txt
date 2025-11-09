idf_component_register(
    SRCS
        OBDPriorityQueue.cpp
        LiveDataCollector.cpp
    INCLUDE_DIRS
        "."
    REQUIRES obdlivedatarequest can_client)
