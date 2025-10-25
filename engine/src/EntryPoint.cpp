//
// Created by ghima on 27-08-2025.
//
#include <memory>

#include "Core/th_util.h"
#include "Core/MainWindow.h"
#include "Core/Constants.h"
#include "Core/Logger.h"
#include "foundation/core_assert.h"

void PrintSystemInformation() {
#if PLATFORM_WINDOWS
    TH_LOG_INFO("Platform Windows");
#elif PLATFORM_APPLE
    TH_LOG_INFO("Mac Platform");
#elif PLATFORM_LINUX
    TH_LOG_INFO("LINUX Platform");
#elif PLATFORM_ANDROID
    TH_LOG_INFO("ANDROID Platform");;

#endif
    // Printing the System compiler Information.
#if COMPILER_MSVC
    TH_LOG_INFO("Compiler MSVC");
#elif COMPILER_GCC
    TH_LOG_INFO("Compiler GCC");
#elif COMPILER_CLANG
    TH_LOG_INFO("Compiler CLANG");
#endif
}

void setupAssertionForEngineLib() {
    rn::AssertHandler handler = [](const char *condition, const char *message, const char *file, int line) -> void {
        std::string assertMessage =
                std::string(message) + " (" + file + " " + std::to_string(line) + ") Assert Failed for " + condition;
        vk::Logger::GetInstance()->WriteLog({vk::LogType::ERROR, assertMessage});
        //   TH_DEBUG_BREAK;
    };
    rn::SetAssertHandlerCheck(handler);

}

int main() {
    PrintSystemInformation();
    std::shared_ptr<vk::MainWindow> mainWindow = std::make_shared<vk::MainWindow>(vk::Constants::WINDOW_WIDTH,
                                                                                  vk::Constants::WINDOW_HEIGHT,
                                                                                  "Small Vulkan Engine");
    setupAssertionForEngineLib();
    TH_CHECK(1 == 3, "This is a assert test");
    mainWindow->RenderWindow();
}
