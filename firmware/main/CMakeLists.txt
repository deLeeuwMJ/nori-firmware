idf_component_register(SRCS "main.cpp" "display.cpp" "render.cpp" "touch.cpp"
                       PRIV_REQUIRES spi_flash nvs_flash bt
                       INCLUDE_DIRS "")
