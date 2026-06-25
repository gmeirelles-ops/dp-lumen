add_library(dp_drivers STATIC
    board_gpio.c
    board_adc.c
    debounce.c
)
target_include_directories(dp_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dp_drivers PUBLIC dp_platform)
