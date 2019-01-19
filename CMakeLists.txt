cmake_minimum_required(VERSION 3.5)
project(grep)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)

add_executable(grep main.cpp mainwindow.cpp file_counter.cpp file_indexer.cpp file_index.cpp)

qt5_use_modules(grep Widgets)
include_directories("${PROJECT_SOURCE_DIR}/cmake-build-debug")
