//
// Created by ghima on 27-08-2025.
//
#include "Graphics.h"
#include "StaticMesh.h"
#include "Texture.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "lights/OmniDirectionalLight.h"

namespace rn {
#pragma region Common
    Map<std::string, StaticMesh *, std::hash<std::string>> Graphics::meshObjectList = {};
    Map<std::string, Texture *, std::hash<std::string>> Graphics::mTextureMap = {};
    RendererContext Graphics::mRendererContext = {};

    Graphics::Graphics(GLFWwindow *window) : mRenderWindow{window} {
        InitVulkan();
    }

    void Graphics::InitVulkan() {
        CreateInstance();
        GetWindowSurface();
        PickPhysicalDeviceAndCreateLogicalDevice();
        CreateSwapChain();
        CreateDepthBufferImages();
        CreateRenderPass();
        CreatePipeline();
        CreateSemaphoresAndFences();
        CreateFrameBuffers();
        CreateCommandPool();
        AllocateCommandBuffer();
        SetRendererContext();
        Imgui_vulkan_init();

        // Setting up the view and projection matrix descriptor sets
        CreateViewProjectionUniformBuffers();

        CreateDescriptorPool();
        AllocateDescriptorSets();
        CreateDefaultTexture(R"(D:\cProjects\SmallVkEngine\textures\brick.png)");

    }

    Graphics::~Graphics() {
        vkDeviceWaitIdle(mDevices.logicalDevice);

        for (size_t i = 0; i < mViewProjectionBuffers.size(); i++) {
            vkDestroyBuffer(mDevices.logicalDevice, mViewProjectionBuffers[i], nullptr);
            vkFreeMemory(mDevices.logicalDevice, mViewProjectionMemory[i], nullptr);
        }
        auto textureIter = mTextureMap.begin();
        while (textureIter != mTextureMap.end()) {
            Texture *texture = textureIter->second;
            delete texture;
            textureIter++;
        }
        // Just for testing the light make the light in the engine as a game object;
        delete light;
        vkDestroyDescriptorPool(mDevices.logicalDevice, mViewProjectionDescriptorPool, nullptr);
        vkDestroyDescriptorPool(mDevices.logicalDevice, mSamplerDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mDevices.logicalDevice, mViewProjectionDescriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevices.logicalDevice, mSamplerDescriptorLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevices.logicalDevice, mLightsDescriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(mDevices.logicalDevice, mLightDescriptorPool, nullptr);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(mDevices.logicalDevice, mImguiDescriptorPool, nullptr);

        Map<std::string, StaticMesh *, std::hash<std::string>>::iterator iter = meshObjectList.begin();
        while (iter != meshObjectList.end()) {
            StaticMesh *mesh = iter->second;
            delete mesh;
            iter++;
        }
        vkDestroyCommandPool(mDevices.logicalDevice, mCommandPool, nullptr);
        for (VkFramebuffer framebuffer: mFrameBuffers) {
            vkDestroyFramebuffer(mDevices.logicalDevice, framebuffer, nullptr);
        }
        vkDestroySemaphore(mDevices.logicalDevice, mPresentImageSemaphore, nullptr);
        vkDestroySemaphore(mDevices.logicalDevice, mGetImageSemaphore, nullptr);
        vkDestroyFence(mDevices.logicalDevice, mPresentFinishFence, nullptr);


        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            vkDestroyImageView(mDevices.logicalDevice, mSwapChainImageViews[i], nullptr);
            vkDestroyImageView(mDevices.logicalDevice, mDepthBufferImageViews[i], nullptr);
            vkDestroyImage(mDevices.logicalDevice, mDepthBufferImages[i], nullptr);
            vkFreeMemory(mDevices.logicalDevice, mDepthBufferImageMemory[i], nullptr);
        }
        vkDestroyPipeline(mDevices.logicalDevice, mPipeline, nullptr);
        vkDestroySampler(mDevices.logicalDevice, mTextureSampler, nullptr);
        vkDestroyPipelineLayout(mDevices.logicalDevice, mPipelineLayout, nullptr);
        vkDestroyRenderPass(mDevices.logicalDevice, mRenderPass, nullptr);
        vkDestroySwapchainKHR(mDevices.logicalDevice, mSwapChain, nullptr);
        vkDestroyDevice(mDevices.logicalDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        vkDestroyInstance(mInstance, nullptr);
    }

#pragma endregion
#pragma region Instance_and_Validations

    static VKAPI_ATTR VkBool32

    VKAPI_CALL ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                  VkDebugUtilsMessageTypeFlagsEXT type,
                                  const VkDebugUtilsMessengerCallbackDataEXT *callback,
                                  void *userData) {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            LOG_ERROR("VALIDATION_ERROR : {}", callback->pMessage);
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            LOG_WARN("VALIDATION_WARNING : {}", callback->pMessage);
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            LOG_INFO("VALIDATION_INFO : {}", callback->pMessage);
        }

        return VK_FALSE;
    }

    void Graphics::CreateInstance() {
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Small Vulkan Engine Renderer";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "Small Vulkan Engine";
        applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);


        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        List<const char *> instanceExtensions{};
        GetWindowExtensions(instanceExtensions);
        instanceExtensions.push_back("VK_EXT_debug_utils");
        instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

        // Enabling the validation layers;
        List<const char *> requiredLayers = {"VK_LAYER_KHRONOS_validation"};
        List<VkLayerProperties> availableLayers{};
        GetAvailableInstanceLayers(availableLayers);
        // Check for the available layers
        CheckAvailability<VkLayerProperties>(requiredLayers, availableLayers, &Graphics::CompareLayerNames);
        instanceCreateInfo.enabledLayerCount = requiredLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data();
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt = CreateDebugMessenger();
        instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfoExt;

        Utility::CheckVulkanError(vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance),
                                  "Failed to create the vulkan instance");

    }

    void Graphics::GetWindowExtensions(List<const char *> &windowExtensions) {
        std::uint32_t count{};
        const char **extensions = glfwGetRequiredInstanceExtensions(&count);
        for (size_t i = 0; i < count; i++) {
            windowExtensions.push_back(extensions[i]);
        }
    }

    void Graphics::GetAvailableInstanceLayers(List<VkLayerProperties> &layerProperties) {
        uint32_t count{};
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        layerProperties.resize(count);
        vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
    }

    VkDebugUtilsMessengerCreateInfoEXT Graphics::CreateDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfoExt{};
        messengerCreateInfoExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfoExt.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        messengerCreateInfoExt.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        messengerCreateInfoExt.pfnUserCallback = ValidationCallback;
        return messengerCreateInfoExt;
    }

#pragma endregion
#pragma region Device_and_Queues

    void Graphics::PickPhysicalDeviceAndCreateLogicalDevice() {
        List<VkPhysicalDevice> physicalDeviceList{};
        GetPhysicalDeviceList(physicalDeviceList);

        List<VkPhysicalDevice>::iterator iter = physicalDeviceList.begin();
        while (iter != physicalDeviceList.end()) {
            if (!CheckIfDeviceSuitable(*iter)) {
                iter = physicalDeviceList.erase(iter);
            } else {
                ++iter;
            }
        }
        if (physicalDeviceList.empty()) {
            LOG_ERROR("No Suitable Physical Device Found to run the Graphics");
            std::exit(EXIT_FAILURE);
        }
        mDevices.physicalDevice = physicalDeviceList[0];
        CreateLogicalDevice(mDevices.physicalDevice);
    }

    void Graphics::GetPhysicalDeviceList(List<VkPhysicalDevice> &physicalDeviceList) {
        std::uint32_t count{};
        vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
        physicalDeviceList.resize(count);
        vkEnumeratePhysicalDevices(mInstance, &count, physicalDeviceList.data());
    }

    bool Graphics::CheckIfDeviceSuitable(VkPhysicalDevice &physicalDevice) {
        FindQueueFamilyIndex(physicalDevice);
        return mQueueFamily.IsValid();
    }

    void Graphics::FindQueueFamilyIndex(VkPhysicalDevice &physicalDevice) {
        std::uint32_t count{};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        List<VkQueueFamilyProperties> queueFamilyProperties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());

        List<VkQueueFamilyProperties>::iterator iter = std::find_if(queueFamilyProperties.begin(),
                                                                    queueFamilyProperties.end(),
                                                                    [](VkQueueFamilyProperties property) -> bool {
                                                                        return (property.queueFlags &
                                                                                (VK_QUEUE_GRAPHICS_BIT |
                                                                                 VK_QUEUE_TRANSFER_BIT));
                                                                    });
        if (iter == queueFamilyProperties.end()) {
            LOG_ERROR("Failed to get the Graphics Queue Family for this device");
            mQueueFamily.graphicsQueueIndex = std::nullopt;
            mQueueFamily.presentationQueueIndex = std::nullopt;
            return;
        }
        mQueueFamily.graphicsQueueIndex = iter - queueFamilyProperties.begin();
        for (int i = 0; i < queueFamilyProperties.size(); i++) {
            VkBool32 hasPresentationMode = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mSurface, &hasPresentationMode);
            if (hasPresentationMode) {
                mQueueFamily.presentationQueueIndex = i;
            }
        }
    }

    void Graphics::CreateLogicalDevice(VkPhysicalDevice &physicalDevice) {
        std::set<std::uint32_t> queueIndex = {mQueueFamily.graphicsQueueIndex.value(),
                                              mQueueFamily.presentationQueueIndex.value()};
        List<VkDeviceQueueCreateInfo> queueCreateInfos{};
        std::float_t priority = 1.f;
        for (std::uint32_t index: queueIndex) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.queueFamilyIndex = index;
            queueCreateInfo.pQueuePriorities = &priority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        // Get physical Device Extensions for the swapchain
        List<const char *> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        List<VkExtensionProperties> availableExtensionProperties{};
        GetPhysicalDeviceExtensionProperties(physicalDevice, availableExtensionProperties);
        CheckAvailability<VkExtensionProperties>(requiredExtensions, availableExtensionProperties,
                                                 &Graphics::ComparePropertyNames);
        deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
        // Enabling required features for the physical device on to the logical device
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        Utility::CheckVulkanError(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &mDevices.logicalDevice),
                                  "Failed to create the logical device from the physical device");
        vkGetDeviceQueue(mDevices.logicalDevice, mQueueFamily.graphicsQueueIndex.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevices.logicalDevice, mQueueFamily.presentationQueueIndex.value(), 0, &mPresentationQueue);

    }

    void Graphics::GetPhysicalDeviceExtensionProperties(VkPhysicalDevice &physicalDevice,
                                                        List<VkExtensionProperties> &propertiesList) {
        std::uint32_t count{};
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
        propertiesList.resize(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, propertiesList.data());
    }

#pragma endregion
#pragma region Surface_and_Swapchain

    void Graphics::GetWindowSurface() {
        Utility::CheckVulkanError(glfwCreateWindowSurface(mInstance, mRenderWindow, nullptr, &mSurface),
                                  "Failed to create the surface for the Rendering window");
    }

    void Graphics::GetSurfaceCapabilities() {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevices.physicalDevice, mSurface,
                                                  &mSwapChainProperties.surfaceCapabilities);
        std::uint32_t surfaceFormats{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(mDevices.physicalDevice, mSurface, &surfaceFormats, nullptr);
        mSwapChainProperties.surfaceFormats.resize(surfaceFormats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(mDevices.physicalDevice, mSurface, &surfaceFormats,
                                             mSwapChainProperties.surfaceFormats.data());
        std::uint32_t presentModes{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevices.physicalDevice, mSurface, &presentModes, nullptr);
        mSwapChainProperties.presentModes.resize(presentModes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(mDevices.physicalDevice, mSurface, &presentModes,
                                                  mSwapChainProperties.presentModes.data());
    }

    VkSurfaceFormatKHR Graphics::ChooseSurfaceFormat() {
        if (mSwapChainProperties.surfaceFormats.size() == 1 &&
            mSwapChainProperties.surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
            return {VkFormat::VK_FORMAT_R8G8B8A8_SNORM, VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }
        return mSwapChainProperties.surfaceFormats[0];
    }

    VkPresentModeKHR Graphics::ChoosePresentMode() {
        List<VkPresentModeKHR>::iterator iter = std::find_if(mSwapChainProperties.presentModes.begin(),
                                                             mSwapChainProperties.presentModes.end(),
                                                             [](VkPresentModeKHR presentMode) -> bool {
                                                                 return presentMode == VK_PRESENT_MODE_MAILBOX_KHR;
                                                             });
        if (iter == mSwapChainProperties.presentModes.end()) {
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        return (*iter);
    }

    size_t Graphics::GetSwapChainImageCount() {
        uint32_t minImageCount = mSwapChainProperties.surfaceCapabilities.minImageCount + 1;
        if (minImageCount > mSwapChainProperties.surfaceCapabilities.maxImageCount) {
            return mSwapChainProperties.surfaceCapabilities.maxImageCount;
        }
        return minImageCount;
    }

    VkExtent2D Graphics::GetWindowExtents() {
        constexpr std::uint32_t invalidCount = std::numeric_limits<std::uint32_t>::max();
        if (mSwapChainProperties.surfaceCapabilities.currentExtent.width == invalidCount) {
            return mSwapChainProperties.surfaceCapabilities.currentExtent;
        }
        glm::ivec2 frameBufferSize{};
        VkExtent2D actualExtent = {
                static_cast<std::uint32_t>(frameBufferSize.x),
                static_cast<std::uint32_t>(frameBufferSize.y)
        };
        actualExtent.width = std::clamp(actualExtent.width,
                                        mSwapChainProperties.surfaceCapabilities.minImageExtent.width,
                                        mSwapChainProperties.surfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         mSwapChainProperties.surfaceCapabilities.minImageExtent.height,
                                         mSwapChainProperties.surfaceCapabilities.maxImageExtent.height);
        return actualExtent;
    }

    void Graphics::CreateSwapChain() {
        GetSurfaceCapabilities();
        mSurfaceFormat = ChooseSurfaceFormat();
        mPresentMode = ChoosePresentMode();
        mWindowExtent = GetWindowExtents();

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = mSurface;
        swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
        swapchainCreateInfo.minImageCount = GetSwapChainImageCount();
        swapchainCreateInfo.imageExtent = mWindowExtent;
        swapchainCreateInfo.presentMode = mPresentMode;
        swapchainCreateInfo.preTransform = mSwapChainProperties.surfaceCapabilities.currentTransform;
        swapchainCreateInfo.oldSwapchain = nullptr;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.clipped = VK_TRUE;

        if (mQueueFamily.presentationQueueIndex.value() != mQueueFamily.graphicsQueueIndex.value()) {
            std::array<std::uint32_t, 2> queueFamilyIndices{mQueueFamily.presentationQueueIndex.value(),
                                                            mQueueFamily.graphicsQueueIndex.value()};
            swapchainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        } else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        Utility::CheckVulkanError(
                vkCreateSwapchainKHR(mDevices.logicalDevice, &swapchainCreateInfo, nullptr, &mSwapChain),
                "Failed to create the swap chain");
        std::uint32_t swapchainImageCount{};
        vkGetSwapchainImagesKHR(mDevices.logicalDevice, mSwapChain, &swapchainImageCount, nullptr);
        mSwapChainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(mDevices.logicalDevice, mSwapChain, &swapchainImageCount, mSwapChainImages.data());

        // Creating the Image views for the images
        mSwapChainImageViews.resize(swapchainImageCount);
        List<VkImageView>::iterator iter = mSwapChainImageViews.begin();
        for (VkImage image: mSwapChainImages) {
            Utility::CreateImageView(mDevices.logicalDevice, image, mSurfaceFormat.format, (*iter),
                                     VK_IMAGE_ASPECT_COLOR_BIT);
            iter++;
        }
    }

#pragma endregion
#pragma region Pipeline

    VkShaderModule Graphics::CreateShaderModule(const char *filePath) {
        VkShaderModule module{};
        List<uint8_t> moduleCode{};
        VkShaderModuleCreateInfo moduleCreateInfo{};
        Utility::ReadFileBinary(filePath, moduleCode);

        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = moduleCode.size();
        moduleCreateInfo.pCode = reinterpret_cast<std::uint32_t *>(moduleCode.data());

        Utility::CheckVulkanError(vkCreateShaderModule(mDevices.logicalDevice, &moduleCreateInfo, nullptr, &module),
                                  "Failed to create the Shader Module");
        return module;
    }

    void Graphics::CreateRenderPass() {
        VkAttachmentDescription colorImageAttachmentDescription{};
        colorImageAttachmentDescription.format = mSurfaceFormat.format;
        colorImageAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorImageAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorImageAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorImageAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorImageAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorImageAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format = mDepthBufferFormat;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;


        // Create the reference for the image attachment for the subpass;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescriptionOne{};
        subpassDescriptionOne.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptionOne.colorAttachmentCount = 1;
        subpassDescriptionOne.pColorAttachments = &colorAttachmentReference;
        subpassDescriptionOne.pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkAttachmentDescription, 2> attachments{colorImageAttachmentDescription, depthAttachmentDescription};
        std::array<VkSubpassDescription, 1> subPass{subpassDescriptionOne};
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subPass.size();
        renderPassCreateInfo.pSubpasses = subPass.data();

        // Create the Render Pass
        Utility::CheckVulkanError(
                vkCreateRenderPass(mDevices.logicalDevice, &renderPassCreateInfo, nullptr, &mRenderPass),
                "Failed to create the Render Pass");

    }

    void Graphics::CreatePipeline() {
        std::string vertexShaderFile = R"(D:\cProjects\SmallVkEngine\Shaders\default.vert.spv)";
        std::string fragShaderFile = R"(D:\cProjects\SmallVkEngine\Shaders\default.frag.spv)";

        VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderFile.c_str());
        VkShaderModule fragmentShaderModule = CreateShaderModule(fragShaderFile.c_str());

        VkPipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStage.module = vertexShaderModule;
        vertexShaderStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStage{};
        fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStage.module = fragmentShaderModule;
        fragmentShaderStage.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{vertexShaderStage, fragmentShaderStage};

        std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        mViewport.x = 0;
        mViewport.y = 0;
        mViewport.width = static_cast<std::float_t>(mWindowExtent.width);
        mViewport.height = static_cast<std::float_t>(mWindowExtent.height);
        mViewport.minDepth = 0;
        mViewport.maxDepth = 1;

        mScissors.offset = {0, 0};
        mScissors.extent = mWindowExtent;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &mViewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &mScissors;

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription positionAttribute{};
        positionAttribute.binding = 0;
        positionAttribute.location = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        positionAttribute.offset = offsetof(Vertex, pos);

        VkVertexInputAttributeDescription colorAttribute{};
        colorAttribute.binding = 0;
        colorAttribute.location = 1;
        colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttribute.offset = offsetof(Vertex, color);

        VkVertexInputAttributeDescription textureCoordsInputAttribute{};
        textureCoordsInputAttribute.binding = 0;
        textureCoordsInputAttribute.location = 2;
        textureCoordsInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
        textureCoordsInputAttribute.offset = offsetof(Vertex, uv);

        VkVertexInputAttributeDescription normalInputAttribute{};
        normalInputAttribute.binding = 0;
        normalInputAttribute.location = 3;
        normalInputAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        normalInputAttribute.offset = offsetof(Vertex, normals);

        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{
                positionAttribute, colorAttribute, textureCoordsInputAttribute, normalInputAttribute
        };
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
        pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

        CreateDescriptorLayouts();
        CreateTextureDefaultSampler();

        List<VkDescriptorSetLayout> setLayouts{mViewProjectionDescriptorSetLayout, mSamplerDescriptorLayout,
                                               mLightsDescriptorSetLayout};
        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = setLayouts.size();
        layoutCreateInfo.pSetLayouts = setLayouts.data();

        Utility::CheckVulkanError(
                vkCreatePipelineLayout(mDevices.logicalDevice, &layoutCreateInfo, nullptr, &mPipelineLayout),
                "Failed to create the layout for the pipeline");


        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.renderPass = mRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.layout = mPipelineLayout;
        pipelineCreateInfo.stageCount = shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;

        Utility::CheckVulkanError(
                vkCreateGraphicsPipelines(mDevices.logicalDevice, nullptr, 1, &pipelineCreateInfo, nullptr, &mPipeline),
                "Failed to create the pipeline");
        vkDestroyShaderModule(mDevices.logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(mDevices.logicalDevice, fragmentShaderModule, nullptr);
    }

#pragma endregion
#pragma region Draw

    void Graphics::CreateSemaphoresAndFences() {
        VkSemaphoreCreateInfo getImageSemaphoreCreateInfo{};
        getImageSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphoreCreateInfo presentImageSemaphoreCreateInfo{};
        presentImageSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo presentImageFenceCreateInfo{};
        presentImageFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        presentImageFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        Utility::CheckVulkanError(
                vkCreateSemaphore(mDevices.logicalDevice, &getImageSemaphoreCreateInfo, nullptr, &mGetImageSemaphore),
                "Failed to create the wait get image semaphore");
        Utility::CheckVulkanError(vkCreateSemaphore(mDevices.logicalDevice, &presentImageSemaphoreCreateInfo, nullptr,
                                                    &mPresentImageSemaphore),
                                  "Failed to create the present Image semaphore");
        Utility::CheckVulkanError(
                vkCreateFence(mDevices.logicalDevice, &presentImageFenceCreateInfo, nullptr, &mPresentFinishFence),
                "Failed to create the fence for the present Image");

    }

    void Graphics::CreateFrameBuffers() {
        mFrameBuffers.resize(mSwapChainImageViews.size());
        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments{mSwapChainImageViews[i], mDepthBufferImageViews[i]};
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = mRenderPass;
            framebufferCreateInfo.width = mWindowExtent.width;
            framebufferCreateInfo.height = mWindowExtent.height;
            framebufferCreateInfo.attachmentCount = attachments.size();
            framebufferCreateInfo.pAttachments = attachments.data();
            framebufferCreateInfo.layers = 1;

            Utility::CheckVulkanError(
                    vkCreateFramebuffer(mDevices.logicalDevice, &framebufferCreateInfo, nullptr, &mFrameBuffers[i]),
                    "Failed to create the frame buffer for the image view");
        }
    }

    void Graphics::CreateCommandPool() {
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = mQueueFamily.graphicsQueueIndex.value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        Utility::CheckVulkanError(
                vkCreateCommandPool(mDevices.logicalDevice, &commandPoolCreateInfo, nullptr, &mCommandPool),
                "Failed to create the graphics queue command Pool");

    }

    void Graphics::AllocateCommandBuffer() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = mCommandPool;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        Utility::CheckVulkanError(vkAllocateCommandBuffers(mDevices.logicalDevice, &allocateInfo, &mCommandBuffer),
                                  "Failed to allocate the command buffer");
    }

    void Graphics::BeginCommand(std::uint32_t currentImageIndex) {
        vkResetCommandBuffer(mCommandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(mCommandBuffer, &beginInfo);

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = mRenderPass;
        renderPassBeginInfo.framebuffer = mFrameBuffers[currentImageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = mWindowExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{.2f, .2f, .2f}};
        clearValues[1].depthStencil.depth = 1;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
        vkCmdSetViewport(mCommandBuffer, 0, 1, &mViewport);
        vkCmdSetScissor(mCommandBuffer, 0, 1, &mScissors);
    }

    void Graphics::EndCommand() {
        vkCmdEndRenderPass(mCommandBuffer);
        Utility::CheckVulkanError(vkEndCommandBuffer(mCommandBuffer), "Failed to end the graphics command buffer");
    }

    void Graphics::BeginFrame() {
        vkWaitForFences(mDevices.logicalDevice, 1, &mPresentFinishFence, VK_TRUE, UINT64_MAX);
        vkResetFences(mDevices.logicalDevice, 1, &mPresentFinishFence);
        vkAcquireNextImageKHR(mDevices.logicalDevice, mSwapChain, UINT64_MAX, mGetImageSemaphore, nullptr,
                              &mCurrentImageIndex);
        BeginCommand(mCurrentImageIndex);
    }

    void Graphics::Draw() {
        //vkCmdDraw(mCommandBuffer, 3, 1, 0, 0);
        List<VkDescriptorSet> descriptorSets{};
        auto iter = meshObjectList.begin();
        while (iter != meshObjectList.end()) {
            VkBuffer vertexBuffer = iter->second->GetVertexBuffer();
            VkBuffer indexBuffer = iter->second->GetIndexBuffer();

            VkDeviceSize offset = {0};
            vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(mCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            // Binding the descriptor sets
            UpdateViewProjectionBuffersUniformBuffers(mCurrentImageIndex);
            light->UpdateLightDescriptorSet(mCurrentImageIndex);

            VkDescriptorSet textureDescriptor{};
            Map<std::string, Texture *, std::hash<std::string>>::iterator texIter = mTextureMap.find(
                    (*iter).second->GetTextureId());
            if (texIter == mTextureMap.end()) {
                LOG_WARN("No Texture Found for the Item {}.. Falling to Default Texture",
                         (*iter).second->GetTextureId().c_str());
                textureDescriptor = mTextureMap.find(
                        R"(D:\cProjects\SmallVkEngine\textures\brick.png)")->second->GetTextureDescriptorSet();
            } else {
                textureDescriptor = texIter->second->GetTextureDescriptorSet();
            }

            descriptorSets.push_back(mViewProjectionDescriptorSets[mCurrentImageIndex]);
            descriptorSets.push_back(textureDescriptor);
            descriptorSets.push_back(light->GetLightDescriptorSets(mCurrentImageIndex));
            vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0,
                                    descriptorSets.size(),
                                    descriptorSets.data(), 0,
                                    nullptr);
            vkCmdDrawIndexed(mCommandBuffer, iter->second->GetStaticMeshIndicesCount(), 1, 0, 0, 0);
            iter++;
        }
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCommandBuffer);
    }

    void Graphics::EndFrame() {
        EndCommand();
        VkSubmitInfo commandSubmitInfo{};
        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        commandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        commandSubmitInfo.commandBufferCount = 1;
        commandSubmitInfo.pCommandBuffers = &mCommandBuffer;
        commandSubmitInfo.waitSemaphoreCount = 1;
        commandSubmitInfo.pWaitSemaphores = &mGetImageSemaphore;
        commandSubmitInfo.signalSemaphoreCount = 1;
        commandSubmitInfo.pSignalSemaphores = &mPresentImageSemaphore;
        commandSubmitInfo.pWaitDstStageMask = &stageFlags;

        Utility::CheckVulkanError(vkQueueSubmit(mGraphicsQueue, 1, &commandSubmitInfo, mPresentFinishFence),
                                  "Failed to submit the command to the queue");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &mPresentImageSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mSwapChain;
        presentInfo.pImageIndices = &mCurrentImageIndex;

        vkQueuePresentKHR(mPresentationQueue, &presentInfo);
    }

    void Graphics::Imgui_vulkan_init() {
        VkDescriptorPoolSize pool_size[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_size);
        pool_info.pPoolSizes = pool_size;

        Utility::CheckVulkanError(
                vkCreateDescriptorPool(mDevices.logicalDevice, &pool_info, nullptr, &mImguiDescriptorPool),
                "Failed to create the descriptor pool for imgui");

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(mRenderWindow, true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = mInstance;
        init_info.PhysicalDevice = mDevices.physicalDevice;
        init_info.Device = mDevices.logicalDevice;
        init_info.Queue = mGraphicsQueue;
        init_info.DescriptorPool = mImguiDescriptorPool;
        init_info.MinImageCount = mSwapChainImageViews.size();
        init_info.ImageCount = mSwapChainImageViews.size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.RenderPass = mRenderPass;

        ImGui_ImplVulkan_Init(&init_info);

    }

#pragma endregion
#pragma region Descriptors

    void Graphics::CreateDescriptorLayouts() {
        VkDescriptorSetLayoutBinding viewProjectionBinding{};
        viewProjectionBinding.binding = 0;
        viewProjectionBinding.descriptorCount = 1;
        viewProjectionBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewProjectionBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        viewProjectionBinding.pImmutableSamplers = nullptr;

        List<VkDescriptorSetLayoutBinding> layoutBinding = {viewProjectionBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = layoutBinding.size();
        layoutCreateInfo.pBindings = layoutBinding.data();

        Utility::CheckVulkanError(vkCreateDescriptorSetLayout(mDevices.logicalDevice, &layoutCreateInfo, nullptr,
                                                              &mViewProjectionDescriptorSetLayout),
                                  "Failed to create the descriptor Set layout");

        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = 0;
        samplerBinding.descriptorCount = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBinding.pImmutableSamplers = nullptr;

        List<VkDescriptorSetLayoutBinding> samplerLayoutBindings = {samplerBinding};
        VkDescriptorSetLayoutCreateInfo samplerLayoutCreateInfo{};
        samplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        samplerLayoutCreateInfo.bindingCount = samplerLayoutBindings.size();
        samplerLayoutCreateInfo.pBindings = samplerLayoutBindings.data();
        Utility::CheckVulkanError(vkCreateDescriptorSetLayout(
                mDevices.logicalDevice, &samplerLayoutCreateInfo, nullptr, &mSamplerDescriptorLayout
        ), "Failed to create the sampler descriptor set layout");
        mRendererContext.samplerDescriptorSetLayout = mSamplerDescriptorLayout;

        VkDescriptorSetLayoutBinding lightsLayoutBinding{};
        lightsLayoutBinding.binding = 0;
        lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightsLayoutBinding.descriptorCount = 1;
        lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        lightsLayoutBinding.pImmutableSamplers = nullptr;

        List<VkDescriptorSetLayoutBinding> lightsLayoutBindings{lightsLayoutBinding};
        VkDescriptorSetLayoutCreateInfo lightsLayoutCreateInfo{};
        lightsLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        lightsLayoutCreateInfo.bindingCount = lightsLayoutBindings.size();
        lightsLayoutCreateInfo.pBindings = lightsLayoutBindings.data();

        Utility::CheckVulkanError(vkCreateDescriptorSetLayout(mDevices.logicalDevice, &lightsLayoutCreateInfo, nullptr,
                                                              &mLightsDescriptorSetLayout),
                                  "Failed to create the descriptor set layouts for lights");
        mRendererContext.lightsLayout = mLightsDescriptorSetLayout;
    }

    void Graphics::CreateDescriptorPool() {
        VkDescriptorPoolSize viewProjectionPoolSize{};
        viewProjectionPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewProjectionPoolSize.descriptorCount = static_cast<std::uint32_t>(mViewProjectionBuffers.size());

        List<VkDescriptorPoolSize> poolSize{viewProjectionPoolSize};
        VkDescriptorPoolCreateInfo viewProjectionDescriptorCreateInfo{};
        viewProjectionDescriptorCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        viewProjectionDescriptorCreateInfo.maxSets = static_cast<std::uint32_t>(mSwapChainImageViews.size());
        viewProjectionDescriptorCreateInfo.poolSizeCount = poolSize.size();
        viewProjectionDescriptorCreateInfo.pPoolSizes = poolSize.data();

        Utility::CheckVulkanError(vkCreateDescriptorPool(mDevices.logicalDevice, &viewProjectionDescriptorCreateInfo,
                                                         nullptr, &mViewProjectionDescriptorPool),
                                  "Failed to create the Descriptor Pool For view and projection");

        // Create The Sampler Descriptor Set Pool;
        VkDescriptorPoolSize samplerPoolSize{};
        samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerPoolSize.descriptorCount = Utility::MAX_OBJECTS;

        List<VkDescriptorPoolSize> samplerPools{samplerPoolSize};
        VkDescriptorPoolCreateInfo samplerPoolCreateInfo{};
        samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        samplerPoolCreateInfo.poolSizeCount = samplerPools.size();
        samplerPoolCreateInfo.pPoolSizes = samplerPools.data();
        samplerPoolCreateInfo.maxSets = Utility::MAX_OBJECTS;

        Utility::CheckVulkanError(vkCreateDescriptorPool(mDevices.logicalDevice, &samplerPoolCreateInfo, nullptr,
                                                         &mSamplerDescriptorPool),
                                  "Failed to create the descriptor pool for the sampler");
        mRendererContext.samplerDescriptorPool = mSamplerDescriptorPool;
        // Creating the descriptor pool for the lights;
        VkDescriptorPoolSize lightsPoolSize{};
        lightsPoolSize.descriptorCount = static_cast<std::uint32_t>(mSwapChainImageViews.size());
        lightsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        List<VkDescriptorPoolSize> lightsDescriptorPoolSizes{lightsPoolSize};
        VkDescriptorPoolCreateInfo lightsPoolCreateInfo{};
        lightsPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        lightsPoolCreateInfo.maxSets = static_cast<std::uint32_t>(mSwapChainImageViews.size());
        lightsPoolCreateInfo.poolSizeCount = lightsDescriptorPoolSizes.size();
        lightsPoolCreateInfo.pPoolSizes = lightsDescriptorPoolSizes.data();

        Utility::CheckVulkanError(
                vkCreateDescriptorPool(mDevices.logicalDevice, &lightsPoolCreateInfo, nullptr, &mLightDescriptorPool),
                "Failed to create the descriptor pool for lights");

        mRendererContext.lightsDescriptorPool = mLightDescriptorPool;
    }

    void Graphics::AllocateDescriptorSets() {
        mViewProjectionDescriptorSets.resize(mSwapChainImageViews.size());
        List<VkDescriptorSetLayout> viewProjectionDescriptorLayouts(mSwapChainImageViews.size(),
                                                                    mViewProjectionDescriptorSetLayout);

        VkDescriptorSetAllocateInfo projectionViewAllocateInfo{};
        projectionViewAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        projectionViewAllocateInfo.descriptorSetCount = static_cast<std::uint32_t>(mSwapChainImageViews.size());
        projectionViewAllocateInfo.pSetLayouts = viewProjectionDescriptorLayouts.data();
        projectionViewAllocateInfo.descriptorPool = mViewProjectionDescriptorPool;

        Utility::CheckVulkanError(vkAllocateDescriptorSets(mDevices.logicalDevice, &projectionViewAllocateInfo,
                                                           mViewProjectionDescriptorSets.data()),
                                  "Failed to allocate the descriptor set for the view and projection matrix");
        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            VkDescriptorBufferInfo viewProjectionBufferInfo{};
            viewProjectionBufferInfo.buffer = mViewProjectionBuffers[i];
            viewProjectionBufferInfo.offset = 0;
            viewProjectionBufferInfo.range = sizeof(ViewProjection);

            VkWriteDescriptorSet viewProjectionWriteInfo{};
            viewProjectionWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            viewProjectionWriteInfo.descriptorCount = 1;
            viewProjectionWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            viewProjectionWriteInfo.dstBinding = 0;
            viewProjectionWriteInfo.dstArrayElement = 0;
            viewProjectionWriteInfo.pBufferInfo = &viewProjectionBufferInfo;
            viewProjectionWriteInfo.dstSet = mViewProjectionDescriptorSets[i];

            List<VkWriteDescriptorSet> writeInfo{viewProjectionWriteInfo};
            vkUpdateDescriptorSets(mDevices.logicalDevice, writeInfo.size(), writeInfo.data(),
                                   0, nullptr);
        }

    }

    void Graphics::CreateViewProjectionUniformBuffers() {
        mViewProjectionBuffers.resize(mSwapChainImageViews.size());
        mViewProjectionMemory.resize(mSwapChainImageViews.size());
        VkDeviceSize viewProjectionSize = sizeof(ViewProjection);

        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            std::string viewProjectionBufferName = "View Projection Buffer";
            Utility::CreateBuffer(mRendererContext, mViewProjectionBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  mViewProjectionMemory[i],
                                  (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                                  viewProjectionSize, viewProjectionBufferName);
        }
    }

    void Graphics::UpdateViewProjectionBuffersUniformBuffers(size_t currentImageIndex) {
        void *data;
        // Updating the view and model matrix;
        vkMapMemory(mDevices.logicalDevice, mViewProjectionMemory[currentImageIndex], 0, sizeof(ViewProjection), 0,
                    &data);
        memcpy(data, &mViewProjection, sizeof(ViewProjection));
        vkUnmapMemory(mDevices.logicalDevice, mViewProjectionMemory[currentImageIndex]);
    }

    ViewProjection Graphics::mViewProjection = {};

    void Graphics::SetViewProjection(rn::ViewProjection &&viewProjection) {
        mViewProjection = viewProjection;
    }

#pragma endregion
#pragma region Depth_Buffer

    void Graphics::CreateDepthBufferImages() {
        mDepthBufferImages.resize(mSwapChainImageViews.size());
        mDepthBufferImageViews.resize(mSwapChainImageViews.size());
        mDepthBufferImageMemory.resize(mSwapChainImageViews.size());
        for (int i = 0; i < mDepthBufferImages.size(); i++) {
            List<VkFormat> requiredFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                              VK_FORMAT_D24_UNORM_S8_UINT};
            mDepthBufferFormat = ChooseSupportedFormats(requiredFormats, VK_IMAGE_TILING_OPTIMAL,
                                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            mDepthBufferImages[i] = Utility::CreateImage("Depth BufferImage", mDevices.physicalDevice,
                                                         mDevices.logicalDevice, mWindowExtent.width,
                                                         mWindowExtent.height, mDepthBufferFormat,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT),
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                         mDepthBufferImageMemory[i]);
            Utility::CreateImageView(mDevices.logicalDevice, mDepthBufferImages[i], mDepthBufferFormat,
                                     mDepthBufferImageViews[i],
                                     VK_IMAGE_ASPECT_DEPTH_BIT);
        }
    }


    VkFormat Graphics::ChooseSupportedFormats(List<VkFormat> &formats, VkImageTiling tiling,
                                              VkFormatFeatureFlags formatFeatureFlags) {
        for (VkFormat format: formats) {
            VkFormatProperties properties{};
            vkGetPhysicalDeviceFormatProperties(mDevices.physicalDevice, format, &properties);
            if ((tiling == VK_IMAGE_TILING_LINEAR &&
                 (properties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags) ||
                tiling == VK_IMAGE_TILING_OPTIMAL &
                (properties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags) {
                return format;
            }
        }
        LOG_ERROR("Failed to find any supported Format for Depth Buffer Images");
        std::exit(EXIT_FAILURE);
    }

#pragma endregion
#pragma region Texture

    void Graphics::CreateTextureDefaultSampler() {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.maxAnisotropy = 16;

        Utility::CheckVulkanError(
                vkCreateSampler(mDevices.logicalDevice, &samplerCreateInfo, nullptr, &mTextureSampler),
                "Failed to create the sampler for the textures");
        mRendererContext.textureSampler = mTextureSampler;
    }

    Texture *Graphics::RegisterTexture(std::string &textureId) {
        Map<std::string, Texture *, std::hash<std::string>>::iterator iter = mTextureMap.find(textureId);
        Texture *texture = nullptr;
        if (iter == mTextureMap.end()) {
            texture = new Texture(textureId.c_str(), &mRendererContext);
            mTextureMap.insert({textureId, texture});

        } else {
            LOG_WARN("Texture {} already Existing", textureId.c_str());
            texture = iter->second;
        }
        return texture;
    }

    void Graphics::CreateDefaultTexture(const std::string &defaultTexturePath) {
        Texture *defaultTexture = new Texture(defaultTexturePath.c_str(), &mRendererContext);
        mTextureMap.insert({defaultTexturePath, defaultTexture});

        // Testing the default lights;
        OmniDirectionalInfo testInfo = {{0, -0.5, -1}, {1, 1, 1, 0}, .5f, .5};
        light = new OmniDirectionalLight{"test", &mRendererContext, testInfo};
    }

#pragma endregion
}