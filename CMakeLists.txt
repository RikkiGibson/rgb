cmake_minimum_required(VERSION 2.8)
project(rgb)
set(CC cc)
set(CXX CC)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories("${PROJECT_SOURCE_DIR}")

add_executable(rgb ${PROJECT_SOURCE_DIR}/rgb.cpp)

# SDL
# TODO: make this cross-platform
set_target_properties(rgb PROPERTIES LINK_FLAGS "-L/usr/local/lib -lSDLmain -lSDL -Wl,-framework,Cocoa")

# Link Boost if desired
# find_package(Boost 1.66 COMPONENTS filesystem)
# target_link_libraries(rgb ${Boost_LIBRARIES})
