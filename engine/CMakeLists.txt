add_executable(${ENGINE}
        include/MainWindow.h
        src/MainWindow.cpp
        src/EntryPoint.cpp
        include/Constants.h
        src/Constants.cpp
        include/Components/Component.h
        src/Components/Component.cpp
        include/Entity/GameObject.h
        src/Entity/GameObject.cpp
        include/Components/MeshComponent.h
        include/Entity/Scene.h
        src/Entity/Scene.cpp
        src/Components/MeshComponent.cpp
)

target_include_directories(${ENGINE} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${ENGINE} PUBLIC
        ${RENDERER}
)