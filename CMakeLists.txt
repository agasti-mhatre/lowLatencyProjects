cmake_minimum_required(VERSION 4.2)
project(lowLatencyProjects)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(order_book)
add_subdirectory(spsc_queue)
add_subdirectory(experiments)