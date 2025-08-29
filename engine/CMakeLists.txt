add_executable(${ENGINE}
        include/MainWindow.h
        src/MainWindow.cpp
        src/EntryPoint.cpp
        include/Constants.h
        src/Constants.cpp
)

target_include_directories(${ENGINE} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${ENGINE} PUBLIC
        ${RENDERER}
)