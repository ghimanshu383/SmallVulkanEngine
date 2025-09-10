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
        RendererContext mRendererContext;
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
        std::uint32_t mCurrentImageIndex;
        List<VkFramebuffer> mFrameBuffers;
        VkCommandPool mCommandPool;
        VkCommandBuffer mCommandBuffer;
        VkSemaphore mGetImageSemaphore;
        VkSemaphore mPresentImageSemaphore;
        VkFence mPresentFinishFence;
        static Map<std::string, class StaticMesh *, std::hash<std::string>> meshObjectList;
        VkDescriptorPool mImguiDescriptorPool;
#pragma endregion Draw
#pragma region Descriptors
        List<VkDescriptorSet> mViewProjectionDescriptorSets{};
        VkDescriptorPool mViewProjectionDescriptorPool{};
        VkDescriptorSetLayout mViewProjectionDescriptorSetLayout{};
        List<VkBuffer> mViewProjectionBuffers{};
        List<VkDeviceMemory> mViewProjectionMemory{};
        // Sampler Descriptor sets;
        VkDescriptorSetLayout mSamplerDescriptorLayout{};
        VkDescriptorPool mSamplerDescriptorPool{};
        List<VkDescriptorSet> mSamplerDescriptorSets{};
#pragma endregion
#pragma region Depth_Buffer
        VkFormat mDepthBufferFormat{};
        List<VkImage> mDepthBufferImages{};
        List<VkImageView> mDepthBufferImageViews{};
        List<VkDeviceMemory> mDepthBufferImageMemory{};
#pragma endregion
#pragma region Texture
        VkSampler mTextureSampler{};
        static Map<std::string, class Texture *, std::hash<std::string>> mTextureMap;
#pragma endregion
    public:
        // Functions
#pragma region Common

        explicit Graphics(GLFWwindow *window);

        void InitVulkan();

        ~Graphics();

        template<typename T>
        void CheckAvailability(List<const char *> &required, List<T> &available, bool(*callback)(const char *, T)) {
            std::all_of(required.begin(), required.end(),
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

        void SetRendererContext() {
            mRendererContext.physicalDevice = mDevices.physicalDevice;
            mRendererContext.logicalDevice = mDevices.logicalDevice;
            mRendererContext.commandPool = mCommandPool;
            mRendererContext.graphicsQueue = mGraphicsQueue;
            mRendererContext.presentationQueue = mPresentationQueue;
            mRendererContext.RegisterMesh = &RegisterMeshObject;
            mRendererContext.UpdateViewAndProjectionMatrix = &SetViewProjection;
            mRendererContext.RegisterTexture = &RegisterTexture;

        }

        static void RegisterMeshObject(std::string &id, class StaticMesh *meshObject) {
            meshObjectList.insert({id, meshObject});
        }

        const RendererContext &GetRendererContext() const { return mRendererContext; };

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

#pragma endregion
#pragma region Pipeline

        void CreateRenderPass();

        void CreatePipeline();

        VkShaderModule CreateShaderModule(const char *filePath);

#pragma endregion
#pragma region Draw


        void CreateSemaphoresAndFences();

        void CreateFrameBuffers();

        void CreateCommandPool();

        void AllocateCommandBuffer();

        void BeginCommand(std::uint32_t currentImageIndex);

        void EndCommand();

        void BeginFrame();

        void Draw();

        void EndFrame();

        void Imgui_vulkan_init();

#pragma endregion Draw
#pragma region Descriptors
        static ViewProjection mViewProjection;

        static void SetViewProjection(ViewProjection &&viewProjection);

        void CreateDescriptorPool();

        void CreateDescriptorLayouts();

        void AllocateDescriptorSets();

        void CreateViewProjectionUniformBuffers();

        void UpdateViewProjectionBuffersUniformBuffers(size_t currentImageIndex);

#pragma endregion
#pragma region Depth_Buffer

        void CreateDepthBufferImages();

        VkFormat
        ChooseSupportedFormats(List<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags formatFeatureFlags);

#pragma endregion
#pragma region Texture

        void CreateTextureDefaultSampler();

        static void RegisterTexture(std::string &textureId, Texture *texture);

        void CreateDefaultTexture(const std::string& defaultTexturePath);


#pragma endregion
    };
}
#endif //SMALLVKENGINE_GRAPHICS_H
