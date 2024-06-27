cmake_minimum_required(VERSION 3.10)

project(2pl)

set(SOURCE_FILES lock.cpp 2pl.cpp transaction.cpp database.cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2 -Wall -g")

find_package(Threads REQUIRED)

add_executable(2pl lock.cpp 2pl.cpp transaction.cpp database.cpp)

target_link_libraries(2pl
        Threads::Threads
        )