//
// Created by ghima on 11-10-2025.
//

#ifndef SMALLVKENGINE_PBRIMPL_H
#define SMALLVKENGINE_PBRIMPL_H


namespace rn {
    class PbrTexture;

    class RendererContext;

    class PbrMaterial;

    class PbrImpl {
        RendererContext *mCtx;
        VkPipeline mPipeline{};
        VkPipelineLayout mLayout{};
        VkDescriptorSetLayout mSetLayout{};
        VkCommandBuffer mSecondaryCommandBuffer{};
        List<PbrMaterial *> mPbrMaterialList{};
        VkPushConstantRange mModelRange{};

        glm::vec3 fallbackColor{1.0, 1.0, 1.0};
        float fallbackMetallic = 0.0;
        float fallbackRoughness = .5;

        void AllocateSecondaryCommandBuffer();

        void CreateDescriptorLayout();

        void CreatePipeline();

        void BeginFrame(int currentImageIndex);

        void BindPipelineAndDrawPbrScene();

        void EndFrame();

        void CreateTestObject();

    public:
        explicit PbrImpl(RendererContext *ctx);

        void Render(int currentImageIndex);

        const VkCommandBuffer &GetSecondaryCommandBuffer() const { return mSecondaryCommandBuffer; };
    };
}
#endif //SMALLVKENGINE_PBRIMPL_H
