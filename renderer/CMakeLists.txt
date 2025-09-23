add_library(${RENDERER} STATIC
        include/Graphics.h
        src/Graphics.cpp
        include/precomp.h
        include/Utility.h
        include/StaticMesh.h
        src/StaticMesh.cpp
        src/Utility.cpp
        include/imgui/imconfig.h
        include/imgui/imgui.h
        include/imgui/imgui_impl_glfw.h
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        include/imgui/imgui_impl_vulkan.h
        src/imgui/imgui.cpp
        src/imgui/imgui_demo.cpp
        src/imgui/imgui_impl_glfw.cpp
        src/imgui/imgui_impl_vulkan.cpp
        src/imgui/imgui_widgets.cpp
        include/imgui/imgui_internal.h
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_tables.cpp
        include/Texture.h
        src/Texture.cpp
        include/stb_image.h
        include/lights/OmniDirectionalLight.h
        src/lights/OmniDirectionalLight.cpp
        include/lights/ShadowMap.h
        src/lights/ShadowMap.cpp
        include/BlockingQueue.h
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