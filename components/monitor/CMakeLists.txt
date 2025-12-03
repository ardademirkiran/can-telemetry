idf_component_register(
    SRCS "CANTrafficMonitor.cpp" "LiveDataCollectorMonitor.cpp"
    INCLUDE_DIRS "."
    REQUIRES can_client globals microsd_module
)
