add_executable(${ENGINE}
        include/Core/MainWindow.h
        src/Core/MainWindow.cpp
        src/EntryPoint.cpp
        include/Core/Constants.h
        src/Core/Constants.cpp
        include/Components/Component.h
        src/Components/Component.cpp
        include/Entity/GameObject.h
        src/Entity/GameObject.cpp
        include/Components/MeshComponent.h
        include/Entity/Scene.h
        src/Entity/Scene.cpp
        src/Components/MeshComponent.cpp
        include/Core/InputSystem.h
        src/Core/InputSystem.cpp
        include/Core/Camera.h
        src/Core/Camera.cpp
        include/Core/Delegate.h
        include/Components/TextureComponent.h
        src/Components/TextureComponent.cpp
        include/Entity/SkyLight.h
        src/Entity/SkyLight.cpp
        include/Components/TransformComponent.h
        src/Components/TransformComponent.cpp
        include/Components/ModelComponent.h
        src/Components/ModelComponent.cpp
        include/Core/ImguiEditor.h
        src/Core/ImguiEditor.cpp
        include/Core/Logger.h
        src/Core/Logger.cpp
)

target_include_directories(${ENGINE} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ext/assimp/include
)

target_link_libraries(${ENGINE} PUBLIC
        ${RENDERER}
        ${CMAKE_SOURCE_DIR}/ext/assimp/lib/Release/assimp-vc143-mt.lib
)