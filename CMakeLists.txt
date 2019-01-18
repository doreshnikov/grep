cmake_minimum_required(VERSION 3.5)
project(fdupes)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)

add_executable(fdupes main.cpp mainwindow.cpp file_counter.cpp duplicates_scanner.cpp)
add_executable(tests tests/tests.cpp duplicates_scanner.cpp)
set_target_properties(tests
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
	EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
	LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)

qt5_use_modules(fdupes Widgets)
qt5_use_modules(tests Widgets)
include_directories("${PROJECT_SOURCE_DIR}/cmake-build-debug")
