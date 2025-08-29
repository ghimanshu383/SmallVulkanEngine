//
// Created by ghima on 27-08-2025.
//

#ifndef SMALLVKENGINE_GRAPHICS_H
#define SMALLVKENGINE_GRAPHICS_H

#include "Utility.h"

namespace rn {
    class Graphics {
        // Instances
#pragma region Common
        GLFWwindow *mRenderWindow;
#pragma endregion
#pragma region Instance_and_Validations
        VkInstance mInstance;
#pragma endregion
#pragma region Device_and_Queues
        struct Devices {
            VkPhysicalDevice physicalDevice{};
            VkDevice logicalDevice{};
        } mDevices;

        struct QueueFamily {
            std::optional<std::uint32_t> graphicsQueueIndex{};
            std::optional<std::uint32_t> presentationQueueIndex{};

            bool IsValid() {
                return (graphicsQueueIndex.has_value() && presentationQueueIndex.has_value());
            }
        } mQueueFamily;

        VkQueue mGraphicsQueue{};
        VkQueue mPresentationQueue{};
#pragma endregion
#pragma region Surface_and_Swapchain
        VkSurfaceKHR mSurface{};
        struct SwapChainProperties {
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            List<VkSurfaceFormatKHR> surfaceFormats{};
            List<VkPresentModeKHR> presentModes{};
        } mSwapChainProperties;
        List<VkImage> mSwapChainImages{};
        List<VkImageView> mSwapChainImageViews{};
        VkExtent2D mWindowExtent{};
        VkSurfaceFormatKHR mSurfaceFormat{};
        VkPresentModeKHR mPresentMode{};
        VkSwapchainKHR mSwapChain{};
#pragma endregion
#pragma region Pipeline
        VkRenderPass mRenderPass{};
        VkPipeline mPipeline{};
        VkPipelineLayout mPipelineLayout{};
        VkViewport mViewport{};
        VkRect2D mScissors{};
#pragma endregion
#pragma region Draw
#pragma endregion Draw
    public:
        // Functions
#pragma region Common

        explicit Graphics(GLFWwindow *window);

        void InitVulkan();

        ~Graphics();

        void CheckVulkanError(VkResult result, const char *message);

        template<typename T>
        void CheckAvailability(List<const char *> &required, List<T> &available, bool(*callback)(const char *, T)) {
            bool isAvailable = std::all_of(required.begin(), required.end(),
                                           [callback, &available](const char *entryRequired) -> bool {
                                               bool isEntryAvailable = std::any_of(available.begin(), available.end(),
                                                                                   [callback, entryRequired](
                                                                                           T entryAvailable) -> bool {
                                                                                       return (*callback)(entryRequired,
                                                                                                          entryAvailable);
                                                                                   });
                                               if (!isEntryAvailable) {
                                                   LOG_ERROR("ERROR VK_GRAPHICS : Entry Unavailable %s", entryRequired);
                                                   std::exit(EXIT_FAILURE);
                                               }
                                               return isEntryAvailable;
                                           });
        }

#pragma endregion
#pragma region Instance_and_Validations

        void CreateInstance();

        void GetWindowExtensions(List<const char *> &windowExtensions);

        void GetAvailableInstanceLayers(List<VkLayerProperties> &layerProperties);

        VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessenger();

        static bool CompareLayerNames(const char *required, VkLayerProperties layerProperties) {
            if (std::strcmp(required, layerProperties.layerName) == 0) {
                return true;
            } else {

                return false;
            }
        }

#pragma endregion
#pragma region Device_and_Queues

        void GetPhysicalDeviceList(List<VkPhysicalDevice> &physicalDeviceList);

        bool CheckIfDeviceSuitable(VkPhysicalDevice &physicalDevice);

        void FindQueueFamilyIndex(VkPhysicalDevice &physicalDevice);

        void PickPhysicalDeviceAndCreateLogicalDevice();

        void CreateLogicalDevice(VkPhysicalDevice &physicalDevice);

        void GetPhysicalDeviceExtensionProperties(VkPhysicalDevice &physicalDevice,
                                                  List<VkExtensionProperties> &propertiesList);

        static bool ComparePropertyNames(const char *name, VkExtensionProperties property) {
            if (std::strcmp(name, property.extensionName) == 0) {
                return true;
            }
            return false;
        }

#pragma endregion
#pragma region Surface_and_Swapchain

        void GetWindowSurface();

        void GetSurfaceCapabilities();

        size_t GetSwapChainImageCount();

        VkSurfaceFormatKHR ChooseSurfaceFormat();

        VkPresentModeKHR ChoosePresentMode();

        VkExtent2D GetWindowExtents();

        void CreateSwapChain();

        void CreateImageView(VkImage &image, VkImageView &imageView, VkImageAspectFlags imageAspect);

#pragma endregion
#pragma region Pipeline

        void CreateRenderPass();

        void CreatePipeline();

        VkShaderModule CreateShaderModule(const char *filePath);

#pragma endregion
#pragma region Draw
#pragma endregion Draw
    };
}
#endif //SMALLVKENGINE_GRAPHICS_H
