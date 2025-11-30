idf_component_register(
    SRCS "SDInterface.cpp" "SDDataSender.cpp"
    INCLUDE_DIRS "."
    REQUIRES fatfs sdmmc driver http_client globals
)