cmake_minimum_required(VERSION 3.28.0)

#set(CMAKE_CXX_COMPILER g++-14 )
#add_compile_options(-std=gnu++26)
#set(CMAKE_CXX_STANDARD 26)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(syscall-tracer)

set(CMAKE_CXX_STANDARD 23)
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
