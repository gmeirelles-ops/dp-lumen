add_library(dp_protocol STATIC
    telemetry_codec.c
)
target_include_directories(dp_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dp_protocol PUBLIC dp_domain)
