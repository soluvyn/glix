cmake_minimum_required(VERSION 3.10)
project(gpx)

add_library(gpx SHARED gpx.c)
target_link_libraries(gpx log)
set_target_properties(gpx PROPERTIES LINK_FLAGS_RELEASE "-s")
set_target_properties(gpx PROPERTIES LINK_FLAGS_MINSIZEREL "-s")
