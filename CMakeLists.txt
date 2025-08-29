cmake_minimum_required(VERSION 3.28)
project(SmallVkEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE engine)
set(RENDERER renderer)

include(FetchContent)

FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.0
        GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(${RENDERER})
add_subdirectory(${ENGINE})




