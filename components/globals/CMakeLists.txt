idf_component_register(
    SRCS "Globals.cpp"
    INCLUDE_DIRS "."
    REQUIRES can_client livedata microsd_module http_client cbor_utils monitor microsd_module nvs_interface
)
