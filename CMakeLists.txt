cmake_minimum_required(VERSION 3.18)
project(TestGame2)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)
#add_executable(${PROJECT_NAME} "src/main.cpp")

add_subdirectory(external/glad/cmake glad_cmake)
glad_add_library(glad_gl_core_mx_10 STATIC LOADER API gl:core=1.0)
target_link_libraries(${PROJECT_NAME} glad_gl_core_mx_10)
