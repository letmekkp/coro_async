cmake_minimum_required( VERSION 3.14 ) 
project(coro_async)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -Wall")

aux_source_directory(./source SRCLIST)

add_executable(coro_async main.cpp ${SRCLIST})

target_link_libraries(coro_async uring)
