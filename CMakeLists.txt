cmake_minimum_required(VERSION 3.16)

project(Prism LANGUAGES C CXX)

option(PrismD3D11 ON "Prims D3D11 backend (Windows only)")
option(PrismVulkan ON "Prims Vulkan backend")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(external/SDL)

if (NOT WIN32)
    set(PrismD3D11 OFF)
endif()

file(GLOB LIBRARY_SOURCES src/*.cpp)

if(PrismD3D11)
    file(GLOB D3D11_SOURCES src/d3d11/*.cpp)
    list(APPEND LIBRARY_SOURCES ${D3D11_SOURCES})
endif()

if(PrismVulkan)
    file(GLOB VULKAN_SOURCES src/vulkan/*.cpp)
    list(APPEND LIBRARY_SOURCES ${VULKAN_SOURCES})
endif()

add_library(PrismShared SHARED ${LIBRARY_SOURCES})
target_include_directories(PrismShared PUBLIC include external/SDL/include)
set_target_properties(PrismShared PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

add_library(PrismStatic STATIC ${LIBRARY_SOURCES})
target_include_directories(PrismStatic PUBLIC include external/SDL/include)
set_target_properties(PrismStatic PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

target_link_libraries(PrismShared SDL3::SDL3)
target_link_libraries(PrismStatic SDL3::SDL3)

if(WIN32)
    target_compile_definitions(PrismShared PRIVATE NOMINMAX)
    target_compile_definitions(PrismStatic PRIVATE NOMINMAX)
endif()

if(PrismD3D11)
    message("Prism_D3D11: ON")
    target_compile_definitions(PrismShared PUBLIC HEXA_PRISM_D3D11)
    target_compile_definitions(PrismStatic PUBLIC HEXA_PRISM_D3D11)

    target_link_libraries(PrismShared d3d11.lib dxgi.lib d3dcompiler.lib)
    target_link_libraries(PrismStatic d3d11.lib dxgi.lib d3dcompiler.lib)
else()
    message("Prism_D3D11: OFF")
endif()

if(PrismVulkan)
    message("Prism_Vulkan: ON")
    target_compile_definitions(PrismShared PUBLIC HEXA_PRISM_VULKAN)
    target_compile_definitions(PrismStatic PUBLIC HEXA_PRISM_VULKAN)

    find_package(Vulkan REQUIRED)
    target_link_libraries(PrismShared Vulkan::Vulkan)
    target_link_libraries(PrismStatic Vulkan::Vulkan)

    add_subdirectory(external/vma)
    target_link_libraries(PrismShared GPUOpen::VulkanMemoryAllocator)
    target_link_libraries(PrismStatic GPUOpen::VulkanMemoryAllocator)

else()
    message("Prism_Vulkan: OFF")
endif()

add_subdirectory(example)