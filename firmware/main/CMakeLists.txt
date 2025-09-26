file(GLOB_RECURSE SRC_FILES 
    "main.cpp" 
    "common/*" 
    "component/*" 
    "core/*"
)

idf_component_register(SRCS ${SRC_FILES}
                       PRIV_REQUIRES spi_flash nvs_flash bt
                       INCLUDE_DIRS "")
