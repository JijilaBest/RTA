idf_component_register(
    SRCS "main.cpp" "active_look.cpp" "ble_manager.cpp"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash
)