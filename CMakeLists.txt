cmake_minimum_required(VERSION 3.14)

#Name for the executable
project(tsh)

#Set C++ standard to 17 or higher
set(CMAKE_CXX_STANDARD 17)

#Source file(s)
set(SOURCES sources/tshell.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE stdc++)