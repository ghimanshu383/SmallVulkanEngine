//
// Created by ghima on 02-10-2025.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>
#include "lights/PointLightShadowMap.h"
#include "StaticMesh.h"

namespace rn {
    PointLightShadowMap::PointLightShadowMap(RendererContext *ctx, rn::PointLightInfo lightInfo) : mCtx{ctx},
                                                                                                   mLightInfo{
                                                                                                           lightInfo},
                                                                                                   mModelPushConstant{},
                                                                                                   mSampler{} {
        CreateRenderPass();
        CreateFrameBuffersImagesAndImageViews();
        CreateDescriptors();
        CreateCommandBufferAndFences();
        CreatePipeline();
        CreateSampler();
    }

    PointLightShadowMap::~PointLightShadowMap() {
        // Clearing the frame buffers;
        for (int i = 0; i < 6; i++) {
            vkDestroyFramebuffer(mCtx->logicalDevice, mFrameBuffers[i], nullptr);
            vkDestroyImageView(mCtx->logicalDevice, mShadowRendingImageViews[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mShadowRenderingImageViewsMemory[i], nullptr);
        }
        vkDestroyImageView(mCtx->logicalDevice, mSamplerImageView, nullptr);
        vkDestroyImage(mCtx->logicalDevice, mShadowImage, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mShadowImageMemory, nullptr);
        vkDestroyImageView(mCtx->logicalDevice, mShadowImageDepthView, nullptr);
        vkDestroyImage(mCtx->logicalDevice, mShadowImageDepth, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mShadowImageDepthMemory, nullptr);
        vkDestroyDescriptorPool(mCtx->logicalDevice, mDescriptorPool, nullptr);

        vkDestroySampler(mCtx->logicalDevice, mSampler, nullptr);
        vkDestroyBuffer(mCtx->logicalDevice, viewProjectionBuffer, nullptr);
        vkFreeMemory(mCtx->logicalDevice, viewProjectionMemory, nullptr);
        vkDestroyBuffer(mCtx->logicalDevice, mLightDataBuffer, nullptr);
        vkFreeMemory(mCtx->logicalDevice, mLightDataMemory, nullptr);
        vkDestroyPipeline(mCtx->logicalDevice, mPipeline, nullptr);
        vkDestroyDescriptorSetLayout(mCtx->logicalDevice, mDescriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mPipelineLayout, nullptr);
        vkDestroyRenderPass(mCtx->logicalDevice, mRenderPass, nullptr);

    }

    void PointLightShadowMap::CreateFrameBuffersImagesAndImageViews() {
        mFrameBuffers.resize(6);
        mShadowRendingImageViews.resize(6);
        mShadowRenderingImageViewsMemory.resize(6);
        mShadowImage = Utility::CreateImage("Point Light Shadow Image", mCtx->physicalDevice, mCtx->logicalDevice,
                                            SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
                                            VK_FORMAT_R32_SFLOAT,
                                            VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            mShadowImageMemory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
        mShadowImageDepth = Utility::CreateImage("Point Light Shadow Image View Depth", mCtx->physicalDevice,
                                                 mCtx->logicalDevice,
                                                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
                                                 VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                 mShadowImageDepthMemory);
        Utility::CreateImageView(mCtx->logicalDevice, mShadowImageDepth, VK_FORMAT_D32_SFLOAT, mShadowImageDepthView,
                                 VK_IMAGE_ASPECT_DEPTH_BIT);

        for (int i = 0; i < 6; i++) {
            Utility::CreateImageView(mCtx->logicalDevice, mShadowImage, VK_FORMAT_R32_SFLOAT,
                                     mShadowRendingImageViews[i], VK_IMAGE_ASPECT_COLOR_BIT, i);
            List<VkImageView> attachments{mShadowRendingImageViews[i], mShadowImageDepthView};
            // Creating the frame Buffer;
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.height = SHADOW_MAP_SIZE;
            framebufferCreateInfo.width = SHADOW_MAP_SIZE;
            framebufferCreateInfo.flags = 0;
            framebufferCreateInfo.attachmentCount = attachments.size();
            framebufferCreateInfo.pAttachments = attachments.data();
            framebufferCreateInfo.renderPass = mRenderPass;
            framebufferCreateInfo.layers = 1;

            Utility::CheckVulkanError(
                    vkCreateFramebuffer(mCtx->logicalDevice, &framebufferCreateInfo, nullptr, &mFrameBuffers[i]),
                    "Failed to create the frame buffer for the point lights");
        }
        // Creating the sampler image view;
        Utility::CreateImageView(mCtx->logicalDevice, mShadowImage, VK_FORMAT_R32_SFLOAT, mSamplerImageView,
                                 VK_IMAGE_ASPECT_COLOR_BIT, 0, 6, VK_IMAGE_VIEW_TYPE_CUBE);
    }

    void PointLightShadowMap::CreateRenderPass() {
        VkAttachmentDescription colorAttachmentDescription{};
        colorAttachmentDescription.format = VK_FORMAT_R32_SFLOAT;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format = VK_FORMAT_D32_SFLOAT;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription{};
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

        List<VkAttachmentDescription> attachments{colorAttachmentDescription, depthAttachmentDescription};
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.flags = 0;

        Utility::CheckVulkanError(vkCreateRenderPass(mCtx->logicalDevice, &renderPassCreateInfo, nullptr, &mRenderPass),
                                  "Failed to create the render pass for the point lights");
    }

    void PointLightShadowMap::CreatePipeline() {
        VkShaderModule vertexShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                        R"(D:\cProjects\SmallVkEngine\Shaders\cubeShadow.ver.spv)");
        VkShaderModule fragShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                      R"(D:\cProjects\SmallVkEngine\Shaders\cubeShadow.frag.spv)");

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
        mViewPort = {0.0f, 0.0f, static_cast<float>(SHADOW_MAP_SIZE),
                     static_cast<float>(SHADOW_MAP_SIZE), 0.0f, 1.0f};
        mScissors = {{0, 0},
                     mCtx->viewportExtends};

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
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

        VkPipelineColorBlendAttachmentState cb{};
        cb.blendEnable = VK_FALSE;
        cb.colorBlendOp = VK_BLEND_OP_MIN;
        cb.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &cb;

        // push constants: model matrix
        mModelPushConstant.size = sizeof(glm::mat4);
        mModelPushConstant.offset = 0;
        mModelPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 1;
        layoutCreateInfo.pSetLayouts = &mDescriptorSetLayout;
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &mModelPushConstant;

        Utility::CheckVulkanError(
                vkCreatePipelineLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mPipelineLayout),
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
        pipelineCreateInfo.layout = mPipelineLayout;
        pipelineCreateInfo.renderPass = mRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;

        Utility::CheckVulkanError(vkCreateGraphicsPipelines(mCtx->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
                                                            nullptr, &mPipeline),
                                  "Failed to create the pipeline for the shadow map");

        vkDestroyShaderModule(mCtx->logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(mCtx->logicalDevice, fragShaderModule, nullptr);
    }

    void PointLightShadowMap::CreateDescriptors() {
        Utility::CreateBuffer(*mCtx, viewProjectionBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, viewProjectionMemory,
                              (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
                              sizeof(ViewProjection), "Point Light view Projection Buffer");
        Utility::CreateBuffer(*mCtx, mLightDataBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mLightDataMemory,
                              (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
                              sizeof(LightData), "Point Light Data Buffer");
        // Creating the DescriptorSet Layout;
        VkDescriptorSetLayoutBinding viewProjectionBinding{};
        viewProjectionBinding.binding = 0;
        viewProjectionBinding.descriptorCount = 1;
        viewProjectionBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        viewProjectionBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorSetLayoutBinding lightDataBinding{};
        lightDataBinding.binding = 1;
        lightDataBinding.descriptorCount = 1;
        lightDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        lightDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        List<VkDescriptorSetLayoutBinding> bindings{viewProjectionBinding, lightDataBinding};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = bindings.size();
        layoutCreateInfo.pBindings = bindings.data();
        layoutCreateInfo.flags = 0;

        Utility::CheckVulkanError(vkCreateDescriptorSetLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr,
                                                              &mDescriptorSetLayout),
                                  "Failed to create the layout for the view projection in the point lights");
        // Creating the descriptor set pool
        VkDescriptorPoolSize viewProjectionPoolSize{};
        viewProjectionPoolSize.descriptorCount = 1;
        viewProjectionPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorPoolSize lightDataPoolSize{};
        lightDataPoolSize.descriptorCount = 1;
        lightDataPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        List<VkDescriptorPoolSize> poolSizes{viewProjectionPoolSize, lightDataPoolSize};
        VkDescriptorPoolCreateInfo viewProjectionPoolCreateInfo{};
        viewProjectionPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        viewProjectionPoolCreateInfo.maxSets = 1;
        viewProjectionPoolCreateInfo.poolSizeCount = poolSizes.size();
        viewProjectionPoolCreateInfo.pPoolSizes = poolSizes.data();
        viewProjectionPoolCreateInfo.flags = 0;

        Utility::CheckVulkanError(vkCreateDescriptorPool(mCtx->logicalDevice, &viewProjectionPoolCreateInfo, nullptr,
                                                         &mDescriptorPool),
                                  "Failed to create the descriptor pool for the view projection point lights");
        VkDescriptorSetAllocateInfo allocateInfo{};

        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pSetLayouts = &mDescriptorSetLayout;
        allocateInfo.descriptorPool = mDescriptorPool;
        allocateInfo.descriptorSetCount = 1;

        vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, &viewProjectionDescriptorSet);

        // Writing the descriptor set;
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ViewProjection);
        bufferInfo.buffer = viewProjectionBuffer;

        VkWriteDescriptorSet writeInfo{};
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeInfo.dstArrayElement = 0;
        writeInfo.dstBinding = 0;
        writeInfo.dstSet = viewProjectionDescriptorSet;
        writeInfo.pBufferInfo = &bufferInfo;

        VkDescriptorBufferInfo lightDataBufferInfo{};
        lightDataBufferInfo.offset = 0;
        lightDataBufferInfo.range = sizeof(LightData);
        lightDataBufferInfo.buffer = mLightDataBuffer;

        VkWriteDescriptorSet lightDataWriteInfo{};
        lightDataWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        lightDataWriteInfo.descriptorCount = 1;
        lightDataWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightDataWriteInfo.dstArrayElement = 0;
        lightDataWriteInfo.dstBinding = 1;
        lightDataWriteInfo.dstSet = viewProjectionDescriptorSet;
        lightDataWriteInfo.pBufferInfo = &lightDataBufferInfo;

        List<VkWriteDescriptorSet> writes{writeInfo, lightDataWriteInfo};

        vkUpdateDescriptorSets(mCtx->logicalDevice, writes.size(), writes.data(), 0, nullptr);
    }

    void PointLightShadowMap::UpdateDescriptorSet(const ViewProjection &viewProjection) {
        void *data;
        vkMapMemory(mCtx->logicalDevice, viewProjectionMemory, 0, sizeof(ViewProjection), 0, &data);
        memcpy(data, &viewProjection, sizeof(ViewProjection));
        vkUnmapMemory(mCtx->logicalDevice, viewProjectionMemory);
        vkMapMemory(mCtx->logicalDevice, mLightDataMemory, 0, sizeof(LightData), 0, &data);
        memcpy(data, &mLightData, sizeof(LightData));
        vkUnmapMemory(mCtx->logicalDevice, mLightDataMemory);
    }

    void PointLightShadowMap::CreateCommandBufferAndFences() {
//        VkCommandBufferAllocateInfo allocateInfo{};
//        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//        allocateInfo.commandBufferCount = 1;
//        allocateInfo.commandPool = mCtx->commandPool;
//        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//
//        Utility::CheckVulkanError(vkAllocateCommandBuffers(mCtx->logicalDevice, &allocateInfo, &mCommandBuffer),
//                                  "Failed to allocate the command buffer for the point lights shadow");
//        VkFenceCreateInfo fenceCreateInfo{};
//        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//        Utility::CheckVulkanError(
//                vkCreateFence(mCtx->logicalDevice, &fenceCreateInfo, nullptr, &renderShadowSceneFence),
//                "Failed  to create the render shadow scene fence for the point lights");
    }

    void PointLightShadowMap::BeginPointShadowFrame(VkCommandBuffer commandBuffer) {
        ComputePointLightViewProjection();
        for (int i = 0; i < 6; i++) {
            VkRenderPassBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = mRenderPass;
            beginInfo.framebuffer = mFrameBuffers[i];
            std::array<VkClearValue, 2> clearValue{};
            clearValue[0].color = {1.0, 1.0, 1.0, 1.0};
            clearValue[1].depthStencil.depth = 1.0;
            beginInfo.clearValueCount = clearValue.size();
            beginInfo.pClearValues = clearValue.data();
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = {SHADOW_MAP_SIZE,
                                           SHADOW_MAP_SIZE};
            vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) SHADOW_MAP_SIZE;
            viewport.height = (float) SHADOW_MAP_SIZE;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);
            UpdateDescriptorSet({mViewProjection.projection, mViewProjection.view[i]});
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                                    &viewProjectionDescriptorSet, 0,
                                    nullptr);

            Map<std::string, StaticMesh *, std::hash<std::string>>::iterator iter = mCtx->GetSceneObjectMap()->begin();
            while (iter != mCtx->GetSceneObjectMap()->end()) {

                vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                                   &iter->second->GetModelMatrix());

                VkBuffer vertexBuffer = iter->second->GetVertexBuffer();
                VkDeviceSize offset = {};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
                vkCmdBindIndexBuffer(commandBuffer, iter->second->GetIndexBuffer(), offset, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer, iter->second->GetStaticMeshIndicesCount(), 1, 0, 0, 0);
                iter++;
            }
            vkCmdEndRenderPass(commandBuffer);
        }
    }

    void PointLightShadowMap::EndFrame(VkCommandBuffer commandBuffer) {
        ImageTransition(commandBuffer);
    }

    void PointLightShadowMap::ImageTransition(VkCommandBuffer commandBuffer) {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.image = mShadowImage;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.subresourceRange.layerCount = 6;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr,
                             0, nullptr,
                             1, &imageMemoryBarrier);
    }

    void PointLightShadowMap::ComputePointLightViewProjection() {
        mLightData.position = mLightInfo.position;
        mLightData.farPlane = 100;

        mViewProjection.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, mLightData.farPlane);
        mViewProjection.projection[1][1] *= -1; // Vulkan clip correction

        glm::vec3 lightPos = glm::vec3(mLightInfo.position);

        // Cubemap face orientations
        mViewProjection.view[0] = glm::lookAt(lightPos, lightPos + glm::vec3( 1, 0, 0), glm::vec3(0, -1, 0)); // +X
        mViewProjection.view[1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)); // -X
        mViewProjection.view[2] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  1, 0), glm::vec3(0,  0, 1)); // +Y
        mViewProjection.view[3] = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0,  0,-1)); // -Y
        mViewProjection.view[4] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  0, 1), glm::vec3(0, -1, 0)); // +Z
        mViewProjection.view[5] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  0,-1), glm::vec3(0, -1, 0)); // -Z

    }
 
    void PointLightShadowMap::CreateSampler() {
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

        Utility::CheckVulkanError(vkCreateSampler(mCtx->logicalDevice, &sampInfo, nullptr, &mSampler),
                                  "Failed to create the sampler for the point light shadows");

    }
}