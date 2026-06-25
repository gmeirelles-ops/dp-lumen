add_library(dp_diag STATIC uart_diag.c)
target_include_directories(dp_diag PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dp_diag PUBLIC dp_domain dp_platform)
