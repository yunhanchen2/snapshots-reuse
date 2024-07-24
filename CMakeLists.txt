cmake_minimum_required(VERSION 3.16)

project(MyProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    include
)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "include/*.h")
add_library(${PROJECT_NAME} ${SRC_FILES})

file(GLOB_RECURSE DOC_FILES "doc/*.txt")
add_custom_target(docs SOURCES ${DOC_FILES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

set(EXECUTABLE_NAME "main")
add_executable(${EXECUTABLE_NAME} "src/main.cpp")
target_link_libraries(${EXECUTABLE_NAME} ${PROJECT_NAME})