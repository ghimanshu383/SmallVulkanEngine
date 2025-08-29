//
// Created by ghima on 27-08-2025.
//
#include "Graphics.h"

namespace rn {
#pragma region Common

    Graphics::Graphics(GLFWwindow *window) : mRenderWindow{window} {
        InitVulkan();
    }

    void Graphics::InitVulkan() {
        CreateInstance();
        GetWindowSurface();
        PickPhysicalDeviceAndCreateLogicalDevice();
        CreateSwapChain();
        CreateRenderPass();
        CreatePipeline();
    }

    Graphics::~Graphics() {
        for (VkImageView imageView: mSwapChainImageViews) {
            vkDestroyImageView(mDevices.logicalDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(mDevices.logicalDevice, mSwapChain, nullptr);
        vkDestroyDevice(mDevices.logicalDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        vkDestroyInstance(mInstance, nullptr);
    }

    void Graphics::CheckVulkanError(VkResult result, const char *message) {
        if (result != VK_SUCCESS) {
            LOG_ERROR("VULKAN GRAPHICS ERROR : %s", message);
            std::exit(EXIT_FAILURE);
        }
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

        CheckVulkanError(vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance),
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

        CheckVulkanError(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &mDevices.logicalDevice),
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
        CheckVulkanError(glfwCreateWindowSurface(mInstance, mRenderWindow, nullptr, &mSurface),
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

    void Graphics::CreateImageView(VkImage &image, VkImageView &imageView, VkImageAspectFlags imageAspect) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = mSurfaceFormat.format;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = imageAspect;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;

        CheckVulkanError(vkCreateImageView(mDevices.logicalDevice, &imageViewCreateInfo, nullptr, &imageView),
                         "Failed to create the image View");
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
        CheckVulkanError(vkCreateSwapchainKHR(mDevices.logicalDevice, &swapchainCreateInfo, nullptr, &mSwapChain),
                         "Failed to create the swap chain");
        std::uint32_t swapchainImageCount{};
        vkGetSwapchainImagesKHR(mDevices.logicalDevice, mSwapChain, &swapchainImageCount, nullptr);
        mSwapChainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(mDevices.logicalDevice, mSwapChain, &swapchainImageCount, mSwapChainImages.data());

        // Creating the Image views for the images
        mSwapChainImageViews.resize(swapchainImageCount);
        List<VkImageView>::iterator iter = mSwapChainImageViews.begin();
        for (VkImage image: mSwapChainImages) {
            CreateImageView(image, (*iter), VK_IMAGE_ASPECT_COLOR_BIT);
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

        CheckVulkanError(vkCreateShaderModule(mDevices.logicalDevice, &moduleCreateInfo, nullptr, &module),
                         "Failed to create the Shader Module");
        return module;
    }

    void Graphics::CreateRenderPass() {
        VkAttachmentDescription colorImageAttachmentDescription{};
        colorImageAttachmentDescription.format = VK_FORMAT_R8G8B8A8_SNORM;
        colorImageAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorImageAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorImageAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorImageAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorImageAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorImageAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        // Create the reference for the image attachment for the subpass;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescriptionOne{};
        subpassDescriptionOne.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptionOne.colorAttachmentCount = 1;
        subpassDescriptionOne.pColorAttachments = &colorAttachmentReference;

        std::array<VkAttachmentDescription, 1> attachments{colorImageAttachmentDescription};
        std::array<VkSubpassDescription, 1> subPass{subpassDescriptionOne};
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subPass.size();
        renderPassCreateInfo.pSubpasses = subPass.data();

        // Create the Render Pass
        CheckVulkanError(vkCreateRenderPass(mDevices.logicalDevice, &renderPassCreateInfo, nullptr, &mRenderPass),
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

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


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

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        CheckVulkanError(vkCreatePipelineLayout(mDevices.logicalDevice, &layoutCreateInfo, nullptr, &mPipelineLayout),
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
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;

        CheckVulkanError(
                vkCreateGraphicsPipelines(mDevices.logicalDevice, nullptr, 1, &pipelineCreateInfo, nullptr, &mPipeline),
                "Failed to create the pipeline");
    }

#pragma endregion
}