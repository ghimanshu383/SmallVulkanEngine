//
// Created by ghima on 14-09-2025.
//
#include "lights/ShadowMap.h"
#include "lights/OmniDirectionalLight.h"
#include "StaticMesh.h"

namespace rn {
    ShadowMap::ShadowMap(rn::RendererContext *ctx, rn::OmniDirectionalLight *light, int width, int height,
                         Map<std::string, StaticMesh *, std::hash<std::string>> *objectMap) : mCtx{ctx},
                                                                                              mDirectionalLight{
                                                                                                      light},
                                                                                              mWidth{width},
                                                                                              mHeight{height},
                                                                                              mObjectMap(
                                                                                                      objectMap) {

    }

    void ShadowMap::Init() {
        CreateShadowMapSemaphore();
        CreateRenderPass();
        CreatePipeline();
        CreateFrameBuffers();
        //CreateDebugDisplayFrameBuffers();
        CreateDescriptorSet();
        WriteViewProjectionDescriptor();
        CreateCommandBuffer();
        CreateSampler();
    }

    void ShadowMap::CreatePipeline() {
        VkShaderModule vertexShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                        R"(D:\cProjects\SmallVkEngine\Shaders\shadow.ver.spv)");
        VkShaderModule fragShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                      R"(D:\cProjects\SmallVkEngine\Shaders\shadow.frag.spv)");

        VkPipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStage.module = vertexShaderModule;
        vertexShaderStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStage{};
        fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStage.module = fragShaderModule;
        fragShaderStage.pName = "main";

        // NOTE: Do NOT include a fragment stage for a depth-only pipeline.
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertexShaderStage, fragShaderStage};

        // viewport / scissor (we'll still use them as dynamic)
        mViewPort = {0.0f, 0.0f, static_cast<float>(mCtx->windowExtents.width),
                     static_cast<float>(mCtx->windowExtents.height), 0.0f, 1.0f};
        mScissors = {{0, 0},
                     mCtx->windowExtents};

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &mViewPort;   // dynamic but set default
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &mScissors;   // dynamic but set default

        // vertex input: position only (match your Vertex struct)
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription positionAttribute{};
        positionAttribute.location = 0;
        positionAttribute.binding = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        positionAttribute.offset = offsetof(Vertex, pos);

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = &positionAttribute;

        // input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // MUST be false for depth writes
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // disable culling while debugging
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // match your mesh winding or set NONE above
        rasterizationStateCreateInfo.lineWidth = 1.0f;
        rasterizationStateCreateInfo.depthBiasEnable = VK_TRUE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 1.25f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 1.75f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;

        // multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // depth/stencil - enable test and write
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        // dynamic states: viewport & scissor
        std::array<VkDynamicState, 3> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                       VK_DYNAMIC_STATE_DEPTH_BIAS};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

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

        // descriptor set layout must be created before layout
        CreateDescriptorSetLayout();

        // push constants: model matrix
        mModelPushConstant.size = sizeof(glm::mat4);
        mModelPushConstant.offset = 0;
        mModelPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 1;
        layoutCreateInfo.pSetLayouts = &mShadowDescriptorLayout;
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &mModelPushConstant;

        Utility::CheckVulkanError(
                vkCreatePipelineLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mShadowPipelineLayout),
                "Failed to create the layout for the shadow pipeline");

        // Assemble pipeline create info
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampling;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.layout = mShadowPipelineLayout;
        pipelineCreateInfo.renderPass = mShadowRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;

        Utility::CheckVulkanError(vkCreateGraphicsPipelines(mCtx->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
                                                            nullptr, &mShadowPipeline),
                                  "Failed to create the pipeline for the shadow map");

        vkDestroyShaderModule(mCtx->logicalDevice, vertexShaderModule, nullptr);
    }

    void ShadowMap::CreateRenderPass() {
        // Creating the image attachment for the display debug Images;
        VkAttachmentDescription colorImageDescription{};
        colorImageDescription.format = mCtx->swapChainFormat;
        colorImageDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorImageDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorImageDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorImageDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorImageDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorImageDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;


        VkAttachmentDescription shadowImageDescription{};
        shadowImageDescription.format = VK_FORMAT_D32_SFLOAT;
        shadowImageDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowImageDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowImageDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        shadowImageDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowImageDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        shadowImageDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        shadowImageDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentReference colorImageRef{};
        colorImageRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorImageRef.attachment = 1;

        VkAttachmentReference shadowImageRef{};
        shadowImageRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowImageRef.attachment = 0;

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//        subpassDescription.pColorAttachments = &colorImageRef;
//        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pDepthStencilAttachment = &shadowImageRef;

        List<VkAttachmentDescription> attachments{shadowImageDescription};
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;

        Utility::CheckVulkanError(
                vkCreateRenderPass(mCtx->logicalDevice, &renderPassCreateInfo, nullptr, &mShadowRenderPass),
                "Failed to create the render pass for the shadows");
    }

    void ShadowMap::CreateFrameBuffers() {
        mSceneImage = Utility::CreateImage("Shadow Map Image", mCtx->physicalDevice, mCtx->logicalDevice,
                                           mCtx->windowExtents.width,
                                           mCtx->windowExtents.height,
                                           VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                           (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
                                           mSceneImageMemory);
        Utility::CreateImageView(mCtx->logicalDevice, mSceneImage, (VK_FORMAT_D32_SFLOAT), mSceneImageview,
                                 VK_IMAGE_ASPECT_DEPTH_BIT);


        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.width = mCtx->windowExtents.width;
        framebufferCreateInfo.height = mCtx->windowExtents.height;
        framebufferCreateInfo.renderPass = mShadowRenderPass;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &mSceneImageview;

        Utility::CheckVulkanError(
                vkCreateFramebuffer(mCtx->logicalDevice, &framebufferCreateInfo, nullptr, &mShadowFrameBuffer),
                "Failed to create the frame buffer for the shadows");
    }

    void ShadowMap::CreateDebugDisplayFrameBuffers() {
        mShadowDebugFrameBuffers.resize(mCtx->swapChainImageCount);

        mSceneImage = Utility::CreateImage("Shadow Map Image", mCtx->physicalDevice, mCtx->logicalDevice,
                                           mCtx->windowExtents.width,
                                           mCtx->windowExtents.height,
                                           VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                           (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
                                           mSceneImageMemory);
        Utility::CreateImageView(mCtx->logicalDevice, mSceneImage, (VK_FORMAT_D32_SFLOAT), mSceneImageview,
                                 VK_IMAGE_ASPECT_DEPTH_BIT);

        //Copying the image to the debug Buffers;
        std::string debugBufferName = "Debug Shadow ImageBuffer";
        Utility::CreateBuffer(*mCtx, mDebugBuffer, (VK_BUFFER_USAGE_TRANSFER_DST_BIT), mDebugBufferMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                              mWidth * mHeight * sizeof(float), debugBufferName);

        for (int i = 0; i < mShadowDebugFrameBuffers.size(); i++) {
            List<VkImageView> attachments{mSceneImageview, mCtx->swapChainImageViews->at(i)};
            VkFramebufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.width = mCtx->windowExtents.width;
            createInfo.height = mCtx->windowExtents.height;
            createInfo.renderPass = mShadowRenderPass;
            createInfo.attachmentCount = attachments.size();
            createInfo.pAttachments = attachments.data();
            createInfo.layers = 1;
            vkCreateFramebuffer(mCtx->logicalDevice, &createInfo, nullptr, &mShadowDebugFrameBuffers[i]);

        }
    }

    void ShadowMap::CreateDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding viewProjectionBinding{};
        viewProjectionBinding.binding = 0;
        viewProjectionBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewProjectionBinding.descriptorCount = 1;
        viewProjectionBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        viewProjectionBinding.pImmutableSamplers = nullptr;

//        VkDescriptorSetLayoutBinding samplerBinding{};
//        samplerBinding.binding = 1;
//        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//        samplerBinding.descriptorCount = 1;
//        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//        samplerBinding.pImmutableSamplers = nullptr;

        List<VkDescriptorSetLayoutBinding> bindings{viewProjectionBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = bindings.size();
        layoutCreateInfo.pBindings = bindings.data();
        layoutCreateInfo.flags = 0;

        Utility::CheckVulkanError(
                vkCreateDescriptorSetLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mShadowDescriptorLayout),
                "Failed to create the view projection layout for the shadow");
    }

    void ShadowMap::CreateDescriptorSet() {
        // Creating the descriptor set pool;
        VkDescriptorPoolSize viewProjectionPoolSize{};
        viewProjectionPoolSize.descriptorCount = 1;
        viewProjectionPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

//        VkDescriptorPoolSize samplerPoolSize{};
//        samplerPoolSize.descriptorCount = 1;
//        samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        List<VkDescriptorPoolSize> poolSizes{viewProjectionPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.maxSets = 1;
        poolCreateInfo.poolSizeCount = poolSizes.size();
        poolCreateInfo.pPoolSizes = poolSizes.data();
        poolCreateInfo.flags = 0;
        Utility::CheckVulkanError(
                vkCreateDescriptorPool(mCtx->logicalDevice, &poolCreateInfo, nullptr, &mShadowDescriptorPool),
                "Failed to create the descriptor Pool for the Shadow Mapping");

        // Allocating the descriptor sets;
        VkDescriptorSetAllocateInfo setAllocateInfo{};
        setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorPool = mShadowDescriptorPool;
        setAllocateInfo.pSetLayouts = &mShadowDescriptorLayout;
        setAllocateInfo.descriptorSetCount = 1;

        vkAllocateDescriptorSets(mCtx->logicalDevice, &setAllocateInfo, &mShadowDescriptorSet);
    }

    void ShadowMap::CreateCommandBuffer() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.commandPool = mCtx->commandPool;
        Utility::CheckVulkanError(vkAllocateCommandBuffers(mCtx->logicalDevice, &allocateInfo, &mShadowCommandBuffer),
                                  "Failed to allocate the command Buffer for the shadow Maps");

    }

    void ShadowMap::CreateShadowMapSemaphore() {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        Utility::CheckVulkanError(
                vkCreateSemaphore(mCtx->logicalDevice, &semaphoreCreateInfo, nullptr, &mShadowMapSemaphore),
                "Failed to create the semaphore for the shadow map");
        vkCreateSemaphore(mCtx->logicalDevice, &semaphoreCreateInfo, nullptr, &mGetNextImageSemaphore);
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(mCtx->logicalDevice, &fenceCreateInfo, nullptr, &mPresentationFinishFence);
    }

    void ShadowMap::BeginShadowFrame() {
//        vkWaitForFences(mCtx->logicalDevice, 1, &mPresentationFinishFence, true, UINT64_MAX);
//        vkResetFences(mCtx->logicalDevice, 1, &mPresentationFinishFence);
//        vkAcquireNextImageKHR(mCtx->logicalDevice, mCtx->swapchain, UINT64_MAX, mGetNextImageSemaphore, nullptr,
//                              &mCurrentImageIndex);

        vkResetCommandBuffer(mShadowCommandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(mShadowCommandBuffer, &beginInfo);

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.framebuffer = mShadowFrameBuffer;
        renderPassBeginInfo.renderArea = {{0, 0},
                                          mCtx->windowExtents};
        List<VkClearValue> clearValue{};
        clearValue.resize(1);
        clearValue[0].depthStencil.depth = 1;
        //   clearValue[1].color = {.4, .4, .4, 0};
        renderPassBeginInfo.pClearValues = clearValue.data();
        renderPassBeginInfo.clearValueCount = clearValue.size();
        renderPassBeginInfo.renderPass = mShadowRenderPass;
        vkCmdBeginRenderPass(mShadowCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(mShadowCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowPipeline);
        vkCmdSetViewport(mShadowCommandBuffer, 0, 1, &mViewPort);
        vkCmdSetScissor(mShadowCommandBuffer, 0, 1, &mScissors);
        vkCmdSetDepthBias(mShadowCommandBuffer, 1.25f, 0.0f, 1.75f);

        // Create The Draw Call
        Map<std::string, StaticMesh *, std::hash<std::string>>::iterator iter = mObjectMap->begin();
        while (iter != mObjectMap->end()) {
//            for (Vertex &vert: (*iter).second->GetVertexList()) {
//                glm::vec4 gl_pos = mDirectionalLight->GetLightViewProjection().projection *
//                                   mDirectionalLight->GetLightViewProjection().view * iter->second->GetModelMatrix() *
//                                   glm::vec4{vert.pos, 1};
//                glm::vec3 ndc = glm::vec3(gl_pos) / gl_pos.w;
//                LOG_INFO("NDC X {}", ndc.x);
//
//            }
            VkBuffer vertBuffer = iter->second->GetVertexBuffer();
            VkBuffer indexBuffer = iter->second->GetIndexBuffer();

            VkDeviceSize offset = {0};
            vkCmdBindVertexBuffers(mShadowCommandBuffer, 0, 1, &vertBuffer, &offset);
            vkCmdBindIndexBuffer(mShadowCommandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            UpdateViewProjectionMatrix(mDirectionalLight->GetLightViewProjection());
            vkCmdPushConstants(mShadowCommandBuffer, mShadowPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(glm::mat4), &(iter->second->GetModelMatrix()));
            vkCmdBindDescriptorSets(mShadowCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowPipelineLayout, 0, 1,
                                    &mShadowDescriptorSet, 0,
                                    nullptr);
            vkCmdDrawIndexed(mShadowCommandBuffer, iter->second->GetStaticMeshIndicesCount(), 1, 0, 0, 0);
            iter++;
        }


    }

    void ShadowMap::EndShadowFrame() {
        vkCmdEndRenderPass(mShadowCommandBuffer);
        //  Updating the image layout
//        VkImageMemoryBarrier barrier{};
//        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//        barrier.image = mSceneImage;
//        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//        barrier.subresourceRange.levelCount = 1;
//        barrier.subresourceRange.layerCount = 1;
//
//        vkCmdPipelineBarrier(
//                mShadowCommandBuffer,
//                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
//                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                0,
//                0, nullptr,
//                0, nullptr,
//                1, &barrier
//        );

        vkEndCommandBuffer(mShadowCommandBuffer);

        // Setting the command buffer to the graphics queue;
        //VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mShadowCommandBuffer;
//        submitInfo.waitSemaphoreCount = 1;
//        submitInfo.pWaitSemaphores = &mGetNextImageSemaphore;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &mShadowMapSemaphore;
        submitInfo.pWaitDstStageMask = &stageFlags;

        vkQueueSubmit(mCtx->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);


//        VkPresentInfoKHR presentInfoKhr{};
//        presentInfoKhr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//        presentInfoKhr.waitSemaphoreCount = 1;
//        presentInfoKhr.pWaitSemaphores = &mShadowMapSemaphore;
//        presentInfoKhr.waitSemaphoreCount = 1;
//        presentInfoKhr.swapchainCount = 1;
//        presentInfoKhr.pSwapchains = &mCtx->swapchain;
//        presentInfoKhr.pImageIndices = &mCurrentImageIndex;
//
//        vkQueuePresentKHR(mCtx->presentationQueue, &presentInfoKhr);

        //   CreateDebugTransitions();
    }

    void ShadowMap::WriteViewProjectionDescriptor() {
        std::string bufferName = "Shadow Buffer name";
        Utility::CreateBuffer(*mCtx, mViewProjectionBuffer, (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), mViewProjectionMemory,
                              (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                              sizeof(ViewProjection), bufferName);
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ViewProjection);
        bufferInfo.buffer = mViewProjectionBuffer;

        VkWriteDescriptorSet writeSet{};
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.descriptorCount = 1;
        writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeSet.dstSet = mShadowDescriptorSet;
        writeSet.dstBinding = 0;
        writeSet.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &writeSet, 0, nullptr);
    }

    void ShadowMap::UpdateViewProjectionMatrix(const ViewProjection &viewProjection) {
        void *data;
        vkMapMemory(mCtx->logicalDevice, mViewProjectionMemory, 0, sizeof(ViewProjection), 0, &data);
        memcpy(data, &viewProjection, sizeof(ViewProjection));
        vkUnmapMemory(mCtx->logicalDevice, mViewProjectionMemory);

    }

    void ShadowMap::CreateSampler() {
        VkSamplerCreateInfo sampInfo{};
        sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampInfo.magFilter = VK_FILTER_LINEAR;
        sampInfo.minFilter = VK_FILTER_LINEAR;
        sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampInfo.maxAnisotropy = 1.0f;
        sampInfo.compareEnable = VK_FALSE;
        sampInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        sampInfo.minLod = 0.0f;
        sampInfo.maxLod = 1.0f;

        vkCreateSampler(mCtx->logicalDevice, &sampInfo, nullptr, &mShadowSampler);

        //Write combined-image-sampler descriptor
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mSceneImageview;
        imageInfo.sampler = mShadowSampler;

        VkWriteDescriptorSet imgWrite{};
        imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imgWrite.dstSet = mCtx->shadowDescriptorSet;
        imgWrite.dstBinding = 0;
        imgWrite.dstArrayElement = 0;
        imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imgWrite.descriptorCount = 1;
        imgWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &imgWrite, 0, nullptr);
    }

    void ShadowMap::CreateDebugTransitions() {
        VkCommandBufferBeginInfo beginInfo{};
        vkBeginCommandBuffer(mShadowCommandBuffer, &beginInfo);

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.image = mSceneImage;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;

        vkCmdPipelineBarrier(mShadowCommandBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr, 0, nullptr,
                             1, &imageMemoryBarrier);
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {mCtx->windowExtents.width, mCtx->windowExtents.height, 1};

        vkCmdCopyImageToBuffer(mShadowCommandBuffer,
                               mSceneImage,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               mDebugBuffer,
                               1, &region);
        vkEndCommandBuffer(mShadowCommandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mShadowCommandBuffer;

        vkQueueSubmit(mCtx->graphicsQueue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(mCtx->graphicsQueue);
        WriteDebugBufferToImage();
    }

    void ShadowMap::WriteDebugBufferToImage() {
        void *data;
        vkMapMemory(mCtx->logicalDevice, mDebugBufferMemory, 0, mWidth * mHeight * sizeof(float), 0, &data);
        float *depthValues = reinterpret_cast<float *>(data);

        for (uint32_t y = 0; y < mHeight; y++) {
            for (uint32_t x = 0; x < mWidth; x++) {
                float d = depthValues[y * mWidth + x];
                if (d != 1.f && d != 0.f) {
                    printf("%f ", d);
                }

            }
            printf("\n");
        }

        vkUnmapMemory(mCtx->logicalDevice, mDebugBufferMemory);
    }
}