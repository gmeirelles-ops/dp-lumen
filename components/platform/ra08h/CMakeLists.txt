add_library(dp_platform STATIC
    platform_init.c
    platform_tick.c
    platform_sleep.c
    platform_uart.c
)
target_include_directories(dp_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/components/drivers
)

if(DP_LUMEN_PLATFORM_STUB)
    target_compile_definitions(dp_platform PUBLIC DP_LUMEN_PLATFORM_STUB=1)
endif()
