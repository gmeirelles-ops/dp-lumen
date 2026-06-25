cmake_minimum_required(VERSION 3.16)

set(DOMAIN_DIR ${CMAKE_SOURCE_DIR}/components/domain)
set(DRIVERS_DIR ${CMAKE_SOURCE_DIR}/components/drivers)
set(PLATFORM_DIR ${CMAKE_SOURCE_DIR}/components/platform/ra08h)

add_library(test_support STATIC
    ${DRIVERS_DIR}/board_gpio.c
    ${PLATFORM_DIR}/platform_tick.c
)
target_include_directories(test_support PUBLIC
    ${DRIVERS_DIR}
    ${PLATFORM_DIR}
    ${DOMAIN_DIR}
)
target_compile_definitions(test_support PUBLIC DP_LUMEN_PLATFORM_STUB=1)

function(add_domain_test name source)
    add_executable(${name} ${source})
    target_include_directories(${name} PRIVATE ${DOMAIN_DIR} ${DRIVERS_DIR})
    target_compile_definitions(${name} PRIVATE DP_LUMEN_PLATFORM_STUB=1)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_domain_test(test_battery_band test_battery_band.c)
target_sources(test_battery_band PRIVATE ${DOMAIN_DIR}/battery_band.c)

add_domain_test(test_button_policy test_button_policy.c)
target_sources(test_button_policy PRIVATE ${DOMAIN_DIR}/button_policy.c)

add_executable(test_luminaire_sm test_luminaire_sm.c
    ${DOMAIN_DIR}/luminaire_sm.c
    ${DOMAIN_DIR}/battery_band.c
    ${DOMAIN_DIR}/button_policy.c
)
target_link_libraries(test_luminaire_sm PRIVATE test_support)
target_include_directories(test_luminaire_sm PRIVATE ${DOMAIN_DIR} ${DRIVERS_DIR} ${PLATFORM_DIR})
target_compile_definitions(test_luminaire_sm PRIVATE DP_LUMEN_PLATFORM_STUB=1)
add_test(NAME test_luminaire_sm COMMAND test_luminaire_sm)
