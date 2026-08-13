cmake_minimum_required(VERSION 3.16)

add_executable(PrismExamples "src/main.cpp")

target_include_directories(PrismExamples PRIVATE include)
target_link_libraries(PrismExamples PrismStatic)

if(WIN32)
    # SDL3 is built shared in this project; SDL3.dll has to sit next to the exe (or be on PATH)
    # for Windows to find it at load time.
    add_custom_command(TARGET PrismExamples POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3>"
            "$<TARGET_FILE_DIR:PrismExamples>"
    )

    if(PrismVulkan)
        # dxcompiler.dll is loaded at runtime by VulkanShaderCompiler (DxcCreateInstance);
        # same deal.
        get_filename_component(Vulkan_BIN_DIR "${Vulkan_INCLUDE_DIR_RESOLVED}/../Bin" ABSOLUTE)
        add_custom_command(TARGET PrismExamples POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${Vulkan_BIN_DIR}/dxcompiler.dll"
                "$<TARGET_FILE_DIR:PrismExamples>"
        )
    endif()
endif()
