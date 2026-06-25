add_executable(dp-lumen main.c)
target_link_libraries(dp-lumen PRIVATE dp_domain dp_drivers dp_diag dp_platform)
