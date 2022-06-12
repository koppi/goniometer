cmake_minimum_required(VERSION 3.17)
project(goniometer VERSION 0.0.1)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED YES)
#set(CMAKE_CXX_EXTENSIONS ON)

add_definitions(-DHAVE_SDL -DHAVE_PORTAUDIO)
set(SOURCES goniometer.c common.c portaudio.c)

set(FPHSA_NAME_MISMATCHED 1) # Suppress warnings, see https://cmake.org/cmake/help/v3.17/module/FindPackageHandleStandardArgs.html
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(portaudio)
unset(FPHSA_NAME_MISMATCHED)

include_directories(${SDL2_IMAGE_INCLUDE_DIR})

add_executable(goniometer ${SOURCES})

target_link_libraries(goniometer PRIVATE m SDL2 SDL2::Main ${SDL2_IMAGE_LIBRARY} portaudio)
add_definitions(-DSDL_MAIN_HANDLED)
