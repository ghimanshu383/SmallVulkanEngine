//
// Created by ghima on 27-08-2025.
//
#include <memory>

#include "Core/MainWindow.h"
#include "Core/Constants.h"

int main() {
    std::shared_ptr<vk::MainWindow> mainWindow = std::make_shared<vk::MainWindow>(vk::Constants::WINDOW_WIDTH,
                                                                                  vk::Constants::WINDOW_HEIGHT,
                                                                                  "Small Vulkan Engine");
    mainWindow->RenderWindow();
}