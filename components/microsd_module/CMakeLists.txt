idf_component_register(
    SRCS "SDCardInterface.cpp" "SDDataSender.cpp"
    INCLUDE_DIRS "."
    REQUIRES fatfs sdmmc driver http_client globals nvs_interface
)