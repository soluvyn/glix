cmake_minimum_required(VERSION 3.10)
project(glix)

add_library(glix SHARED glix.c)
target_link_libraries(glix log)
set_target_properties(glix PROPERTIES LINK_FLAGS_RELEASE "-s")
set_target_properties(glix PROPERTIES LINK_FLAGS_MINSIZEREL "-s")
