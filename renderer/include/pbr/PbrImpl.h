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
        Map<std::string, class PbrMaterial *, std::hash<std::string>> mMaterialMap{};
        VkPushConstantRange mModelRange{};

        glm::vec3 fallbackColor{1.0, 1.0, 1.0};
        float fallbackMetallic = 0.0;
        float fallbackRoughness = .5;

        void AllocateSecondaryCommandBuffer();

        void CreateDescriptorLayout();

        void CreatePipeline();

        void BeginFrame(std::uint32_t currentImageIndex);

        void BindPipelineAndDrawPbrMesh(StaticMesh *mesh);

        void EndFrame();

        void CreateDefaultTexture();

        explicit PbrImpl(RendererContext *ctx);

        static PbrImpl *instance;
    public:

        static PbrImpl *GetInstance(RendererContext *ctx);

        void Render(std::uint32_t currentImageIndex, StaticMesh *mesh);

        const VkCommandBuffer &GetSecondaryCommandBuffer() const { return mSecondaryCommandBuffer; };

        void CleanUp();

        PbrMaterial * LoadTexture(const std::string &texturePath);
    };
}
#endif //SMALLVKENGINE_PBRIMPL_H
