cmake_minimum_required(VERSION 3.16)

project(Prism LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(external/SDL)

file(GLOB LIBRARY_SOURCES src/*.cpp src/**/*.cpp)

add_library(PrismShared SHARED ${LIBRARY_SOURCES})
target_include_directories(PrismShared PUBLIC include external/SDL/include)

add_library(PrismStatic STATIC ${LIBRARY_SOURCES})
target_include_directories(PrismStatic PUBLIC include external/SDL/include)

target_link_libraries(PrismShared SDL3::SDL3)
target_link_libraries(PrismStatic SDL3::SDL3)

add_subdirectory(example)