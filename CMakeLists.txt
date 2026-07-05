cmake_minimum_required(VERSION 3.28.0)
project(syscall-tracer)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# executable
add_executable(syscall_tracer
        main.cpp
        ${SOURCE_FILES}
)

target_include_directories(syscall_tracer
        PUBLIC
        include/
)
