//
// Created by ghima on 06-10-2025.
//
#include "SkyBox.h"
#include "StaticMesh.h"

#define STB_IMAGE_RESIZE2_IMPLEMENTATION

#include "stb_image_resize2.h"

namespace rn {
    Skybox::Skybox(rn::RendererContext *ctx) : mCtx{ctx}, mCubeMesh{nullptr} {
        SimpleCubeMeshBox();
        CreateDescriptorPoolAndAllocateSets();
        CreatePipeline();
         CreateImageAndImageViews();
        CreateSamplerAndWriteDescriptorSet();
    }

    void Skybox::CreatePipeline() {
        VkShaderModule vertexShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                        R"(D:\cProjects\SmallVkEngine\Shaders\Skybox.ver.spv)");
        VkShaderModule fragShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                      R"(D:\cProjects\SmallVkEngine\Shaders\Skybox.frag.spv)");

        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
        vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageCreateInfo.module = vertexShaderModule;
        vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
        fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCreateInfo.module = fragShaderModule;
        fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCreateInfo.pName = "main";

        List<VkPipelineShaderStageCreateInfo> shaderStages{vertexShaderStageCreateInfo, fragShaderStageCreateInfo};

        VkVertexInputBindingDescription vertexInputBindingDescription{};
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(Vertex);
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription positionAttribute{};
        positionAttribute.binding = 0;
        positionAttribute.location = 0;
        positionAttribute.offset = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

        List<VkVertexInputAttributeDescription> attributeDescriptions{positionAttribute};
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
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
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
        pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendStates[2]{};

        blendStates[0].blendEnable = VK_FALSE;
        blendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        blendStates[1].blendEnable = VK_FALSE;
        blendStates[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.attachmentCount = 2;
        colorBlendStateCreateInfo.pAttachments = blendStates;

        List<VkDynamicState> states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = states.size();
        dynamicStateCreateInfo.pDynamicStates = states.data();

        mViewport.x = 0;
        mViewport.y = 0;
        mViewport.width = static_cast<std::float_t>(mCtx->windowExtents.width);
        mViewport.height = static_cast<std::float_t>(mCtx->windowExtents.height);
        mViewport.minDepth = 0;
        mViewport.maxDepth = 1;

        mScissors.offset = {0, 0};
        mScissors.extent = mCtx->windowExtents;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &mViewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &mScissors;

        VkPushConstantRange viewProjectionPush{};
        viewProjectionPush.size = sizeof(glm::mat4);
        viewProjectionPush.offset = 0;
        viewProjectionPush.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &viewProjectionPush;
           layoutCreateInfo.setLayoutCount = 1;
         layoutCreateInfo.pSetLayouts = &mSetLayout;

        Utility::CheckVulkanError(vkCreatePipelineLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mLayout),
                                  "Failed to create the layout for the sky box");
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = mLayout;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.renderPass = mCtx->offScreenRenderPass;
        pipelineCreateInfo.stageCount = shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;

        Utility::CheckVulkanError(vkCreateGraphicsPipelines(mCtx->logicalDevice, nullptr, 1, &pipelineCreateInfo,
                                                            nullptr, &mPipeline),
                                  "Failed to create the pipeline for the sky box");
        vkDestroyShaderModule(mCtx->logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(mCtx->logicalDevice, fragShaderModule, nullptr);
    }

    void Skybox::RenderSkyBox() {
        vkCmdBindPipeline(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) mCtx->viewportExtends.width;
        viewport.height = (float) mCtx->viewportExtends.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mCtx->viewportExtends;
        VkBuffer vertexBuffer = mCubeMesh->GetVertexBuffer();
        vkCmdSetViewport(mCtx->mainCommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(mCtx->mainCommandBuffer, 0, 1, &scissor);

        VkDeviceSize offset = {};
        vkCmdBindVertexBuffers(mCtx->mainCommandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(mCtx->mainCommandBuffer, mCubeMesh->GetIndexBuffer(), offset, VK_INDEX_TYPE_UINT32);

        ViewProjection viewProjection = *mCtx->GetViewProjectionMatrix();
        glm::mat4 VP = viewProjection.projection * glm::mat4(glm::mat3(viewProjection.view)); // drop translation
        vkCmdPushConstants(mCtx->mainCommandBuffer, mLayout, VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(glm::mat4), &VP);
        vkCmdBindDescriptorSets(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mLayout, 0, 1,
                                &mDescriptorSets, 0,
                                nullptr);
        vkCmdDrawIndexed(mCtx->mainCommandBuffer, mCubeMesh->GetStaticMeshIndicesCount(), 1, 0, 0, 0);

    }

    void Skybox::SimpleCubeMeshBox() {
        List<Vertex> skyboxVertices = {
// Front face (-Z)
                {{-1.0f, -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, 1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},

// Back face (+Z)
                {{-1.0f, -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, 1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},


                // Left face (-X)
                {{-1.0f, -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, 1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, 1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},

                // Right face (+X)
                {{1.0f,  -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},

                // Top face (+Y)
                {{-1.0f, 1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, 1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  1.0f,  -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},

                // Bottom face (-Y)
                {{-1.0f, -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{-1.0f, -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  -1.0f, 1.0f},  {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
                {{1.0f,  -1.0f, -1.0f}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0}},
        };

        List<uint32_t> skyboxIndices = {
                0, 1, 2, 2, 3, 0,        // front
                4, 5, 6, 6, 7, 4,        // back
                8, 9, 10, 10, 11, 8,     // left
                12, 13, 14, 14, 15, 12,  // right
                16, 17, 18, 18, 19, 16,  // top
                20, 21, 22, 22, 23, 20   // bottom
        };
        std::string empty;
        mCubeMesh = new StaticMesh(*mCtx, skyboxVertices, skyboxIndices, -1, empty, false);

    }

    void Skybox::CreateDescriptorPoolAndAllocateSets() {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBinding.pImmutableSamplers = nullptr;

        List<VkDescriptorSetLayoutBinding> binding{samplerBinding};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = binding.size();
        layoutCreateInfo.pBindings = binding.data();
        layoutCreateInfo.flags = 0;
        vkCreateDescriptorSetLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mSetLayout);

        VkDescriptorPoolSize poolSizeSampler{};
        poolSizeSampler.descriptorCount = 1;
        poolSizeSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        List<VkDescriptorPoolSize> poolSizes{poolSizeSampler};
        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = poolSizes.size();
        poolCreateInfo.pPoolSizes = poolSizes.data();
        poolCreateInfo.maxSets = 1;

        vkCreateDescriptorPool(mCtx->logicalDevice, &poolCreateInfo, nullptr, &mDescriptorPool);

        // Allocating the descriptor set for the sampler
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = mDescriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &mSetLayout;

        vkAllocateDescriptorSets(mCtx->logicalDevice, &allocateInfo, &mDescriptorSets);
    }

    void Skybox::CreateImageAndImageViews() {
        mSkyBoxImage = Utility::CreateImage("Sky box Image", mCtx->physicalDevice, mCtx->logicalDevice,
                                            SKY_BOX_RESOLUTION, SKY_BOX_RESOLUTION, VK_FORMAT_R8G8B8A8_SRGB,
                                            VK_IMAGE_TILING_OPTIMAL,
                                            (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            mSkyBoxImageMemory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
        Utility::TransitionImageLayout(*mCtx, mSkyBoxImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6, 0);
        // Creating the image view with 6 layers.
        Utility::CreateImageView(mCtx->logicalDevice, mSkyBoxImage, VK_FORMAT_R8G8B8A8_SRGB, mSkyBoxImageView,
                                 VK_IMAGE_ASPECT_COLOR_BIT, 0, 6, VK_IMAGE_VIEW_TYPE_CUBE);
        mStagingBuffers.resize(6);
        mStagingBufferMemory.resize(6);

        for (int i = 0; i < 6; i++) {
            int height, width;
            VkDeviceSize imageSize;
            uint8_t *imageData = Utility::LoadTextureImage(R"(D:\cProjects\SmallVkEngine\textures\Textile.jpg)", width,
                                                           height, imageSize);
            VkDeviceSize SkyBoxImageSize = 4 * SKY_BOX_RESOLUTION * SKY_BOX_RESOLUTION;
            unsigned char *resizeData = new unsigned char[4 * SKY_BOX_RESOLUTION * SKY_BOX_RESOLUTION]{};
            stbir_resize_uint8_linear(reinterpret_cast<unsigned char *>(imageData), width, height, 0, resizeData,
                                      SKY_BOX_RESOLUTION, SKY_BOX_RESOLUTION,
                                      0, STBIR_RGBA);

            Utility::CreateBuffer(*mCtx, mStagingBuffers[i], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, mStagingBufferMemory[i],
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  4 * SKY_BOX_RESOLUTION * SKY_BOX_RESOLUTION, "Sky Box Buffer");
            void *data;
            vkMapMemory(mCtx->logicalDevice, mStagingBufferMemory[i], 0, SkyBoxImageSize, 0, &data);
            memcpy(data, resizeData, SkyBoxImageSize);
            vkUnmapMemory(mCtx->logicalDevice, mStagingBufferMemory[i]);
            stbi_image_free(imageData);
            delete[] resizeData;

            Utility::CopyBufferToImage(*mCtx, mStagingBuffers[i], mSkyBoxImage, SKY_BOX_RESOLUTION, SKY_BOX_RESOLUTION,
                                       VK_IMAGE_ASPECT_COLOR_BIT, i);
            vkDestroyBuffer(mCtx->logicalDevice, mStagingBuffers[i], nullptr);
            vkFreeMemory(mCtx->logicalDevice, mStagingBufferMemory[i], nullptr);
        }
        Utility::TransitionImageLayout(*mCtx, mSkyBoxImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6, 0);
    }

    void Skybox::CreateSamplerAndWriteDescriptorSet() {
        VkSamplerCreateInfo sampInfo{};
        sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampInfo.magFilter = VK_FILTER_LINEAR;
        sampInfo.minFilter = VK_FILTER_LINEAR;
        sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampInfo.maxAnisotropy = 1.0f;
        sampInfo.compareEnable = VK_FALSE;
        sampInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        sampInfo.minLod = 0.0f;
        sampInfo.maxLod = 1.0f;

        Utility::CheckVulkanError(vkCreateSampler(mCtx->logicalDevice, &sampInfo, nullptr, &mCubeSampler),
                                  "Failed to create the sampler for the point light shadows");

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mSkyBoxImageView;
        imageInfo.sampler = mCubeSampler;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.dstSet = mDescriptorSets;
        writeDescriptorSet.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(mCtx->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
    }
}