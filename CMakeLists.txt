cmake_minimum_required(VERSION 3.23)
project(MyStdLib)

include(Dependencies.cmake)

add_subdirectory(src)
add_subdirectory(test)

enable_testing()
