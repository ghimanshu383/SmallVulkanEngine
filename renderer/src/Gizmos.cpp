//
// Created by ghima on 27-09-2025.
//
#include "Gizmos.h"
#include "StaticMesh.h"

namespace rn {
    Gizmos::Gizmos(RendererContext *ctx) : mTranslateMesh{nullptr}, mCtx{ctx} {
        SetUpMesh();
        CreatePipeline(TOPOLOGY_TYPE::LINES);
        CreatePipeline(TOPOLOGY_TYPE::TRIANGLES);
    }

    void Gizmos::CreatePipeline(TOPOLOGY_TYPE type) {
        VkShaderModule vertexShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                        R"(D:\cProjects\SmallVkEngine\Shaders\gizmo.ver.spv)");
        VkShaderModule fragShaderModule = Utility::CreateShaderModule(mCtx->logicalDevice,
                                                                      R"(D:\cProjects\SmallVkEngine\Shaders\gizmo.frag.spv)");

        VkPipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStage.module = vertexShaderModule;
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStage{};
        fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStage.module = fragShaderModule;
        fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStage.pName = "main";

        List<VkPipelineShaderStageCreateInfo> shaderStages{vertexShaderStage, fragShaderStage};

        VkVertexInputBindingDescription vertexInputBindingDescription{};
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputBindingDescription.stride = sizeof(Vertex);

        VkVertexInputAttributeDescription positionAttribute{};
        positionAttribute.binding = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        positionAttribute.offset = offsetof(Vertex, pos);
        positionAttribute.location = 0;

        VkVertexInputAttributeDescription colorAttribute{};
        colorAttribute.binding = 0;
        colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttribute.offset = offsetof(Vertex, color);
        colorAttribute.location = 1;

        VkVertexInputAttributeDescription uvAttribute{};
        uvAttribute.binding = 0;
        uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
        uvAttribute.offset = offsetof(Vertex, uv);
        uvAttribute.location = 2;

        VkVertexInputAttributeDescription normalAttribute{};
        normalAttribute.binding = 0;
        normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        normalAttribute.offset = offsetof(Vertex, normals);
        normalAttribute.location = 3;

        List<VkVertexInputAttributeDescription> inputAttributes{positionAttribute, colorAttribute, uvAttribute,
                                                                normalAttribute};

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

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributes.size();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology =
                type == TOPOLOGY_TYPE::LINES ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
        depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
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

        VkPushConstantRange modelRange{};
        modelRange.offset = 0;
        modelRange.size = sizeof(ModelUBO);
        modelRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        List<VkPushConstantRange> pushConstants{modelRange};

        List<VkDescriptorSetLayout> layouts{mCtx->viewProjectionLayout};
        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = layouts.size();
        layoutCreateInfo.pSetLayouts = layouts.data();
        layoutCreateInfo.pushConstantRangeCount = pushConstants.size();
        layoutCreateInfo.pPushConstantRanges = pushConstants.data();
        layoutCreateInfo.flags = 0;

        Utility::CheckVulkanError(vkCreatePipelineLayout(mCtx->logicalDevice, &layoutCreateInfo, nullptr,
                                                         type == TOPOLOGY_TYPE::LINES ? &mLayoutLines
                                                                                      : &mLayoutTriangles),
                                  "Failed to create the layout for the gizmos");

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.renderPass = mCtx->offScreenRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.layout = type == TOPOLOGY_TYPE::LINES ? mLayoutLines : mLayoutTriangles;
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
                                                            nullptr, type == TOPOLOGY_TYPE::LINES ? &mGizmoPipelineLines
                                                                                                  : &mGizmoPipelineTriangles),
                                  "Failed to create the gizmos pipeline");
        vkDestroyShaderModule(mCtx->logicalDevice, vertexShaderModule, nullptr);
        vkDestroyShaderModule(mCtx->logicalDevice, fragShaderModule, nullptr);

    }

    void Gizmos::SetUpMesh() {
        List<Vertex> gizmoVertices = {
                // X Axis (Red)
                {{0.0f,             0.0f,             0.0f},             {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // start
                {{AXIS_LENGTH,      0.0f,             0.0f},             {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // end

                // Y Axis (Green)
                {{0.0f,             0.0f,             0.0f},             {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // start
                {{0.0f,             AXIS_LENGTH,      0.0f},             {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // end

                // Z Axis (Blue)
                {{0.0f,             0.0f,             0.0f},             {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // start
                {{0.0f,             0.0f,             AXIS_LENGTH},      {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // end

                // Triangles on Top of the lines;
                // tip
                {{AXIS_LENGTH +
                  ARROW_TIP_LENGTH, 0.0f,             0.0f},             {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                // base upper
                {{AXIS_LENGTH,      ARROW_BASE_SIZE,  0.0f},             {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                // base lower
                {{AXIS_LENGTH,      -ARROW_BASE_SIZE, 0.0f},             {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

                // tip
                {{0.0f,             AXIS_LENGTH +
                                    ARROW_TIP_LENGTH, 0.0f},             {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
// base right
                {{ARROW_BASE_SIZE,  AXIS_LENGTH,      0.0f},             {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
// base left
                {{-ARROW_BASE_SIZE, AXIS_LENGTH,      0.0f},             {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

                // tip
                {{0.0f,             0.0f,             AXIS_LENGTH +
                                                      ARROW_TIP_LENGTH}, {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
// base top
                {{0.0f,             ARROW_BASE_SIZE,  AXIS_LENGTH},      {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
// base bottom
                {{0.0f,             -ARROW_BASE_SIZE, AXIS_LENGTH},      {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = {
                // X axis line
                0, 1,
                // Y axis line
                2, 3,
                // Z axis line
                4, 5,
                //Triangles.
                6, 7, 8,
                9, 10, 11,
                12, 13, 14
        };
        std::string noTex;
        mTranslateMesh = new StaticMesh(*mCtx, gizmoVertices, indices, 0, noTex, false);
    }

    void Gizmos::DrawGizmos(size_t currentImageIndex) {
        for (int i = 0; i < 2; i++) {
            std::uint32_t activeId = static_cast<std::uint32_t>(mCtx->GetActiveGizmoAxis());
            mTranslateMesh->SetModelMatrix(mModelMatrix);
            vkCmdBindPipeline(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              i == 0 ? mGizmoPipelineLines : mGizmoPipelineTriangles);
            vkCmdSetLineWidth(mCtx->mainCommandBuffer, LINE_WIDTH);
            VkDeviceSize offset = {};
            VkBuffer vertexBuffer = mTranslateMesh->GetVertexBuffer();
            vkCmdBindVertexBuffers(mCtx->mainCommandBuffer, 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(mCtx->mainCommandBuffer, mTranslateMesh->GetIndexBuffer(), offset,
                                 VK_INDEX_TYPE_UINT32);

            std::uint32_t dyOffset = 0;
            vkCmdBindDescriptorSets(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    i == 0 ? mLayoutLines : mLayoutTriangles, 0, 1,
                                    &(mCtx->viewProjectionDescriptorSet[currentImageIndex]), 1,
                                    &dyOffset);

            ModelUBO modelUbo = {mTranslateMesh->GetModelMatrix(), activeId};
            vkCmdPushConstants(mCtx->mainCommandBuffer, i == 0 ? mLayoutLines : mLayoutTriangles,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO),
                               &modelUbo);
            if (i == 0) {
                vkCmdDrawIndexed(mCtx->mainCommandBuffer, 6, 1, 0, 0, 0);
            } else {
                vkCmdDrawIndexed(mCtx->mainCommandBuffer, 9, 1, 6, 0, 0);
            }
        }

    }

    Gizmos::~Gizmos() {
        delete mTranslateMesh;
        vkDestroyPipeline(mCtx->logicalDevice, mGizmoPipelineLines, nullptr);
        vkDestroyPipeline(mCtx->logicalDevice, mGizmoPipelineTriangles, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayoutLines, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayoutTriangles, nullptr);
    }
}