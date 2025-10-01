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
        CreatePipeline(TOPOLOGY_TYPE::LINE_STRIP);
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
                type == TOPOLOGY_TYPE::LINES ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : type == TOPOLOGY_TYPE::TRIANGLES
                                                                                 ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                                                                 : VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
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
                                                                                      : type == TOPOLOGY_TYPE::TRIANGLES
                                                                                        ? &mLayoutTriangles
                                                                                        : &mLayoutLineStrip),
                                  "Failed to create the layout for the gizmos");

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.renderPass = mCtx->offScreenRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.layout =
                type == TOPOLOGY_TYPE::LINES ? mLayoutLines : type == TOPOLOGY_TYPE::TRIANGLES ? mLayoutTriangles
                                                                                               : mLayoutLineStrip;
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
                                                                                                  : type ==
                                                                                                    TOPOLOGY_TYPE::TRIANGLES
                                                                                                    ? &mGizmoPipelineTriangles
                                                                                                    : &mGizmoPipelineLineStrip),
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

                // X axis scale cube (Red)
                {{AXIS_LENGTH -
                  CUBE_SIZE,        -CUBE_SIZE,       -CUBE_SIZE},       {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH +
                  CUBE_SIZE,        -CUBE_SIZE,       -CUBE_SIZE},       {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH +
                  CUBE_SIZE,        CUBE_SIZE,        -CUBE_SIZE},       {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH -
                  CUBE_SIZE,        CUBE_SIZE,        -CUBE_SIZE},       {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH -
                  CUBE_SIZE,        -CUBE_SIZE,       CUBE_SIZE},        {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH +
                  CUBE_SIZE,        -CUBE_SIZE,       CUBE_SIZE},        {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH +
                  CUBE_SIZE,        CUBE_SIZE,        CUBE_SIZE},        {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},
                {{AXIS_LENGTH -
                  CUBE_SIZE,        CUBE_SIZE,        CUBE_SIZE},        {1.0f, 0.2f, 0.2f, 1.0f}, {1001.0f, 0},    {0,    0,    1}},

                // Y axis scale cube (Green)
                {{-CUBE_SIZE,       AXIS_LENGTH -
                                    CUBE_SIZE,        -CUBE_SIZE},       {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        AXIS_LENGTH -
                                    CUBE_SIZE,        -CUBE_SIZE},       {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        AXIS_LENGTH +
                                    CUBE_SIZE,        -CUBE_SIZE},       {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       AXIS_LENGTH +
                                    CUBE_SIZE,        -CUBE_SIZE},       {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       AXIS_LENGTH -
                                    CUBE_SIZE,        CUBE_SIZE},        {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        AXIS_LENGTH -
                                    CUBE_SIZE,        CUBE_SIZE},        {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        AXIS_LENGTH +
                                    CUBE_SIZE,        CUBE_SIZE},        {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       AXIS_LENGTH +
                                    CUBE_SIZE,        CUBE_SIZE},        {0.2f, 1.0f, 0.2f, 1.0f}, {2002.0f, 0},    {0,    0,    1}},

                // Z axis scale cube (Blue)
                {{-CUBE_SIZE,       -CUBE_SIZE,       AXIS_LENGTH -
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        -CUBE_SIZE,       AXIS_LENGTH -
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        CUBE_SIZE,        AXIS_LENGTH -
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       CUBE_SIZE,        AXIS_LENGTH -
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       -CUBE_SIZE,       AXIS_LENGTH +
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        -CUBE_SIZE,       AXIS_LENGTH +
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{CUBE_SIZE,        CUBE_SIZE,        AXIS_LENGTH +
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},
                {{-CUBE_SIZE,       CUBE_SIZE,        AXIS_LENGTH +
                                                      CUBE_SIZE},        {0.2f, 0.2f, 1.0f, 1.0f}, {3003.0f, 0},    {0,    0,    1}},


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
                12, 13, 14,
                //Cubes
                15, 16, 17, 17, 18, 15, // back
                19, 20, 21, 21, 22, 19, // front
                15, 19, 22, 22, 18, 15, // left
                16, 20, 21, 21, 17, 16, // right
                18, 17, 21, 21, 22, 18, // top
                15, 16, 20, 20, 19, 15,  // bottom
                // YAxis
                23, 24, 25, 25, 26, 23, // back
                27, 28, 29, 29, 30, 27, // front
                23, 27, 30, 30, 26, 23, // left
                24, 28, 29, 29, 25, 24, // right
                26, 25, 29, 29, 30, 26, // top
                23, 24, 28, 28, 27, 23,  // bottom
                // ZAXis
                31, 32, 33, 33, 34, 31, // back
                35, 36, 37, 37, 38, 35, // front
                31, 35, 38, 38, 34, 31, // left
                32, 36, 37, 37, 33, 32, // right
                34, 33, 37, 37, 38, 34, // top
                31, 32, 36, 36, 35, 31  // bottom
        };
        rotationStartIndex = indices.size();
        BuildRotationGizmo(gizmoVertices, indices, AXIS_LENGTH);
        std::string noTex;
        mTranslateMesh = new StaticMesh(*mCtx, gizmoVertices, indices, 0, noTex, false);
    }

    void Gizmos::BuildRotationGizmo(std::vector<Vertex> &verts,
                                    std::vector<uint32_t> &indices,
                                    float AXIS_LENGTH,
                                    int segments,
                                    float radiusMultiplier) {
        const float PI = 3.14159265358979323846f;
        const float TWO_PI = 2.0f * PI;
        const float radius = AXIS_LENGTH * radiusMultiplier;

        // IDs (like you used 1001/2002/3003 before) - use distinct ids for rotation gizmo
        const float idX = 1001.0f; // X-rotation ring (around X axis)
        const float idY = 2002.0f; // Y-rotation ring (around Y axis)
        const float idZ = 3003.0f; // Z-rotation ring (around Z axis)

        // Helper lambda to append a circle
        auto append_circle = [&](int axis, const glm::vec4 &color, float id) {
            uint32_t baseVertex = static_cast<uint32_t>(verts.size());
            // generate vertices
            for (int i = 0; i < segments; ++i) {
                float a = (float) i / (float) segments * TWO_PI;
                glm::vec3 pos(0.0f);
                glm::vec3 normal(0.0f);

                if (axis == 1) {
                    // circle in YZ plane (rotation around X)
                    pos.y = cosf(a) * radius;
                    pos.z = sinf(a) * radius;
                    normal = glm::vec3(1.0f, 0.0f, 0.0f);
                } else if (axis == 2) {
                    // circle in ZX plane (rotation around Y)
                    pos.z = cosf(a) * radius;
                    pos.x = sinf(a) * radius;
                    normal = glm::vec3(0.0f, 1.0f, 0.0f);
                } else {
                    // axis == 3: circle in XY plane (rotation around Z)
                    pos.x = cosf(a) * radius;
                    pos.y = sinf(a) * radius;
                    normal = glm::vec3(0.0f, 0.0f, 1.0f);
                }

                // match your Vertex initializer: {{pos}, {r,g,b,a}, {id,0}, {normal}}
                verts.push_back(Vertex{
                        {pos.x,    pos.y,    pos.z},
                        {color.r,  color.g,  color.b, color.a},
                        {id,       0.0f},
                        {normal.x, normal.y, normal.z}
                });
            }

            // indices: make a closed line strip (append first vertex at end)
            for (int i = 0; i < segments; ++i) indices.push_back(baseVertex + i);
            indices.push_back(baseVertex + 0); // close loop (segments+1 indices)
        };

        append_circle(1, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), idX);
        append_circle(2, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f), idY);
        append_circle(3, glm::vec4(0.2f, 0.2f, 1.0f, 1.0f), idZ);
    }

    void Gizmos::DrawGizmos(size_t currentImageIndex) {
        if (mGizmoType == GIZMO_TYPE::ROTATE) {
            DrawRotationGizmo(currentImageIndex);
        } else if ((mGizmoType == GIZMO_TYPE::SCALE) || (mGizmoType == GIZMO_TYPE::TRANSLATE)) {
            DrawTranslateScaleGizmo(currentImageIndex);
        }
    }

    Gizmos::~Gizmos() {
        delete mTranslateMesh;
        vkDestroyPipeline(mCtx->logicalDevice, mGizmoPipelineLines, nullptr);
        vkDestroyPipeline(mCtx->logicalDevice, mGizmoPipelineTriangles, nullptr);
        vkDestroyPipeline(mCtx->logicalDevice, mGizmoPipelineLineStrip, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayoutLines, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayoutTriangles, nullptr);
        vkDestroyPipelineLayout(mCtx->logicalDevice, mLayoutLineStrip, nullptr);

    }

    void Gizmos::DrawRotationGizmo(std::uint32_t currentImageIndex) {
        std::uint32_t activeId = static_cast<std::uint32_t>(mCtx->GetActiveGizmoAxis());
        mTranslateMesh->SetModelMatrix(mModelMatrix);
        vkCmdBindPipeline(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mGizmoPipelineLineStrip);
        vkCmdSetLineWidth(mCtx->mainCommandBuffer, LINE_WIDTH);
        VkDeviceSize offset = {};
        VkBuffer vertexBuffer = mTranslateMesh->GetVertexBuffer();
        vkCmdBindVertexBuffers(mCtx->mainCommandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(mCtx->mainCommandBuffer, mTranslateMesh->GetIndexBuffer(), offset,
                             VK_INDEX_TYPE_UINT32);

        std::uint32_t dyOffset = 0;
        vkCmdBindDescriptorSets(mCtx->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mLayoutLineStrip, 0, 1,
                                &(mCtx->viewProjectionDescriptorSet[currentImageIndex]), 1,
                                &dyOffset);

        ModelUBO modelUbo = {mTranslateMesh->GetModelMatrix(), activeId};
        vkCmdPushConstants(mCtx->mainCommandBuffer, mLayoutLineStrip,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelUBO),
                           &modelUbo);
        vkCmdDrawIndexed(mCtx->mainCommandBuffer, 65, 1, rotationStartIndex, 0, 0); // X
        vkCmdDrawIndexed(mCtx->mainCommandBuffer, 65, 1, rotationStartIndex + 65, 0, 0); // Y
        vkCmdDrawIndexed(mCtx->mainCommandBuffer, 65, 1, rotationStartIndex + 130, 0, 0); // Z
    }

    void Gizmos::DrawTranslateScaleGizmo(std::uint32_t currentImageIndex) {
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
                int indexCount = mGizmoType == GIZMO_TYPE::TRANSLATE ? 9 : 3 * 36;
                int firstIndex = mGizmoType == GIZMO_TYPE::TRANSLATE ? 6 : 15;
                vkCmdDrawIndexed(mCtx->mainCommandBuffer, indexCount, 1, firstIndex, 0, 0);
            }
        }
    }
}