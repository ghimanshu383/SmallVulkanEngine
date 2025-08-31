add_library(${RENDERER} STATIC
        include/Graphics.h
        src/Graphics.cpp
        include/precomp.h
        include/Utility.h
        include/StaticMesh.h
        src/StaticMesh.cpp
        src/Utility.cpp
)

target_include_directories(${RENDERER} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        D:\\VulkanSDK\\1.3.283.0\\Include
        ${CMAKE_SOURCE_DIR}/ext/glm

)

target_link_libraries(${RENDERER} PUBLIC
        spdlog
        glfw
        D:\\VulkanSDK\\1.3.283.0\\Lib\\vulkan-1.lib
)

target_precompile_headers(${RENDERER} PUBLIC include/precomp.h)