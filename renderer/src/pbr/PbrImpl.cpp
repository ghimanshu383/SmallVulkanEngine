//
// Created by ghima on 11-10-2025.
//

#ifndef SMALLVKENGINE_PBRIMPL_CPP
#define SMALLVKENGINE_PBRIMPL_CPP

#include "Utility.h"
#include "pbr/PbrImpl.h"
#include "pbr/PbrTexture.h"
#include "pbr/PbrMaterials.h"
#include "lights/PointLights.h"
#include "StaticMesh.h"

namespace rn {
    PbrImpl *PbrImpl::instance = nullptr;

    PbrImpl::PbrImpl(RendererContext *ctx) : mCtx{ctx} {
        CreateDescriptorLayout();
        CreatePipeline();
        AllocateSecondaryCommandBuffer();
        CreateDefaultTexture();
    }

    PbrImpl *PbrImpl::GetInstance(RendererContext *ctx) {
        if (instance == nullptr) {
            instance = new PbrImpl(ctx);
        }

        return instance;
    }

    void PbrImpl::CreatePipeline() {
        VkShaderModule vertexModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                  R"(D:\cProjects\SmallVkEngine\Shaders\pbr.ver.spv)");
        VkShaderModule fragModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                R"(D:\cProjects\SmallVkEngine\Shaders\pbr.frag.spv)");

        VkPipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStage.module = vertexModule;
        vertexShaderStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStage{};
        fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStage.module = fragModule;
        fragShaderStage.pName = "main";

        List<VkPipelineShaderStageCreateInfo> shaderStages{vertexShaderStage, fragShaderStage};

        VkVertexInputBindingDescription inputBindingDescription{};
        inputBindingDescription.binding = 0;
        inputBindingDescription.stride = sizeof(Vertex);
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription posAttribute{};
        posAttribute.binding = 0;
        posAttribute.location = 0;
        posAttribute.offset = offsetof(Vertex, pos);
        posAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

        VkVertexInputAttributeDescription colorAttribute{};
        colorAttribute.binding = 0;
        colorAttribute.location = 1;
        colorAttribute.offset = offsetof(Vertex, color);
        colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

        VkVertexInputAttributeDescription uvAttribute{};
        uvAttribute.binding = 0;
        uvAttribute.location = 2;
        uvAttribute.offset = offsetof(Vertex, uv);
        uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;

        VkVertexInputAttributeDescription normalAttribute{};
        normalAttribute.binding = 0;
        normalAttribute.location = 3;
        normalAttribute.offset = offsetof(Vertex, normals);
        normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;

        List<VkVertexInputAttributeDescription> attributeDescriptions{posAttribute, colorAttribute, uvAttribute,
                                                                      normalAttribute};

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &inputBindingDescription;
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
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;;
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

        VkViewport mViewport{};
        VkRect2D mScissors{};
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

        List<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();


        mModelRange.size = sizeof(ModelUBO);
        mModelRange.offset = 0;
        mModelRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        List<VkDescriptorSetLayout> setLayouts{mSetLayout};

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pSetLayouts = setLayouts.data();
        layoutCreateInfo.setLayoutCount = setLayouts.size();
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &mModelRange;

        Utility::CheckVulkanError(vkCreatePipelineLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mLayout),
                                  "Failed to create the pipeline layout for the pbr");
        // Creating pipeline for the pbr.
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.renderPass = *mCtx->offScreenRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.layout = mLayout;
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

        Utility::CheckVulkanError(vkCreateGraphicsPipelines(mCtx->logicalDevice, nullptr, 1, &pipelineCreateInfo,
                                                            nullptr, &mPipeline),
                                  "failed to create the pipeline for the pbr ");
        vkDestroyShaderModule(mCtx->logicalDevice, vertexModule, nullptr);
        vkDestroyShaderModule(mCtx->logicalDevice, fragModule, nullptr);
        VkRenderPass mPbrPipelineRenderPassHandle = pipelineCreateInfo.renderPass;
        uint32_t mPbrPipelineSubpass = pipelineCreateInfo.subpass;
        printf("PBR pipeline created for renderPass %p subpass %u\n", (void *) mPbrPipelineRenderPassHandle,
               mPbrPipelineSubpass);
    }

    void PbrImpl::AllocateSecondaryCommandBuffer() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = mCtx->commandPool;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        Utility::CheckVulkanError(
                vkAllocateCommandBuffers(mCtx->logicalDevice, &allocateInfo, &mSecondaryCommandBuffer),
                "Failed to allocate the secondary command buffer for pbr");

    }

    void PbrImpl::CreateDescriptorLayout() {
        VkDescriptorSetLayoutBinding viewProjectionLayoutBinding{};
        viewProjectionLayoutBinding.binding = 0;
        viewProjectionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewProjectionLayoutBinding.descriptorCount = 1;
        viewProjectionLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding diffuseLightBinding{};
        diffuseLightBinding.binding = 1;
        diffuseLightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        diffuseLightBinding.descriptorCount = 1;
        diffuseLightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorSetLayoutBinding pointLightBinding{};
        pointLightBinding.binding = 2;
        pointLightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pointLightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pointLightBinding.descriptorCount = 1;

        //Creating the sampler Binding
        VkDescriptorSetLayoutBinding pbrTextureBinding{};
        pbrTextureBinding.binding = 3;
        pbrTextureBinding.descriptorCount = 5;
        pbrTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pbrTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        List<VkDescriptorSetLayoutBinding> bindings{viewProjectionLayoutBinding, diffuseLightBinding, pointLightBinding,
                                                    pbrTextureBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = bindings.size();
        layoutCreateInfo.pBindings = bindings.data();
        layoutCreateInfo.flags = 0;

        Utility::CheckVulkanError(
                vkCreateDescriptorSetLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr, &mSetLayout),
                "Failed to create the descriptor set layout for the pbr");
    }

    void PbrImpl::BeginFrame(std::uint32_t currentImageIndex) {
        VkCommandBufferInheritanceInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        info.renderPass = *mCtx->offScreenRenderPass;
        info.subpass = 0;
        info.framebuffer = mCtx->offScreenFrameBuffers->at(currentImageIndex);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags =
                VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = &info;

        vkBeginCommandBuffer(mSecondaryCommandBuffer, &beginInfo);
    }


    void PbrImpl::BindPipelineAndDrawPbrMesh(StaticMesh *mesh) {
        PbrMaterial *material;
        Map<std::string, PbrMaterial *, std::hash<std::string>>::iterator iter = std::find_if(
                mMaterialMap.begin(),
                mMaterialMap.end(),
                [&](const std::pair<std::string, PbrMaterial *> &entry) -> bool {
                    return mesh->GetTextureId() ==
                           entry.first;
                });
        if (iter == mMaterialMap.end()) {
            material = mMaterialMap.at(BASE_PBR_MATERIAL_ID);
        } else {
            material = iter->second;
        }

        vkCmdBindPipeline(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
        VkViewport viewport = {0, 0, static_cast<float>(mCtx->viewportExtends.width),
                               static_cast<float>(mCtx->viewportExtends.height), 0, 1};
        VkRect2D scissors = {
                0, 0,
                mCtx->viewportExtends.width, mCtx->viewportExtends.height
        };
        vkCmdSetViewport(mCtx->mainCommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(mCtx->mainCommandBuffer, 0, 1, &scissors);

        material->UpdateUniformBuffersFromGraphicsContext();
        VkDeviceSize offset{};
        VkBuffer vertexBuffer = mesh->GetVertexBuffer();
        vkCmdBindVertexBuffers(mCtx->mainCommandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(mCtx->mainCommandBuffer, mesh->GetIndexBuffer(), offset,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mLayout, 0, 1,
                                &material->GetDescriptorSet(), 0,
                                nullptr);
        ModelUBO modelUbo{mesh->GetModelMatrix(), mesh->GetPickId()};
        vkCmdPushConstants(mCtx->mainCommandBuffer, mLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO),
                           &modelUbo);
        vkCmdDrawIndexed(mCtx->mainCommandBuffer,
                         mesh->GetStaticMeshIndicesCount(),
                         1, 0, 0, 0);
    }

    void PbrImpl::EndFrame() {
        vkEndCommandBuffer(mSecondaryCommandBuffer);
    }

    void PbrImpl::Render(std::uint32_t currentImageIndex, StaticMesh *mesh) {
        BeginFrame(currentImageIndex);
        BindPipelineAndDrawPbrMesh(mesh);
        EndFrame();
    }

    void PbrImpl::CreateDefaultTexture() {

        PbrMaterial *defaultMaterial = new PbrMaterial{mCtx, mSetLayout, BASE_PBR_MATERIAL_ID};
        mMaterialMap.insert({BASE_PBR_MATERIAL_ID, defaultMaterial});
    }

    void PbrImpl::CleanUp() {
        auto iter = mMaterialMap.begin();
        while (iter != mMaterialMap.end()) {
            auto material = iter->second;
            delete material;
            iter++;
        }
        vkDestroyPipeline(mCtx->logicalDevice, mPipeline, nullptr);
        vkDestroyDescriptorSetLayout(mCtx->logicalDevice, mSetLayout, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayout, nullptr);
        delete instance;
    }

    PbrMaterial * PbrImpl::LoadTexture(const std::string &texturePath) {
        Map<std::string, PbrMaterial *, std::hash<std::string>>::iterator iter = mMaterialMap.find(texturePath);
        if (iter == mMaterialMap.end()) {
            auto material = new PbrMaterial(mCtx, mSetLayout, texturePath);
            mMaterialMap.insert({texturePath, material});
            return material;
        } else {
            LOG_WARN("The Material already Exists");
            return iter->second;
        }

    }
}

#endif //SMALLVKENGINE_PBRIMPL_CPP
