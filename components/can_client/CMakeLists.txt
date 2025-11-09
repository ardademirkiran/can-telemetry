idf_component_register(
    SRCS "CANClient.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer
)
