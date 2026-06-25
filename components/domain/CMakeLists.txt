add_library(dp_domain STATIC
    battery_band.c
    button_policy.c
    luminaire_sm.c
)
target_include_directories(dp_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dp_domain PUBLIC dp_drivers)
