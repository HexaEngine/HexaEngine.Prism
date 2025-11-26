cmake_minimum_required(VERSION 3.16)

add_executable(PrismExamples "src/main.cpp")

target_include_directories(PrismExamples PRIVATE include)
target_link_libraries(PrismExamples PrismStatic)
