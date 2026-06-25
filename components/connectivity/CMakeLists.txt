add_library(dp_connectivity STATIC
    lora_radio.c
)
target_include_directories(dp_connectivity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dp_connectivity PUBLIC dp_protocol)
