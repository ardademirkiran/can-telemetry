idf_component_register(
    SRCS "SDInterface.cpp"
    INCLUDE_DIRS "."
    REQUIRES fatfs sdmmc driver
)