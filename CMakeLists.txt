cmake_minimum_required(VERSION 4.2)
project(lowLatencyProjects)

set(CMAKE_CXX_STANDARD 23)

# Runtime profiling flags
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")

add_subdirectory(order_book)
add_subdirectory(experiments)
add_subdirectory(spsc_queue)