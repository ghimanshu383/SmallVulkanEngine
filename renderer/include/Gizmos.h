//
// Created by ghima on 27-09-2025.
//

#ifndef SMALLVKENGINE_GIZMOS_H
#define SMALLVKENGINE_GIZMOS_H

#include "Utility.h"

namespace rn {
    class Gizmos {
    private:
        class StaticMesh *mTranslateMesh;

        const float LINE_WIDTH = 6.0f;
        const float AXIS_LENGTH = .5f;
        const float ARROW_SIZE = 0.1f;

        void CreatePipeline();

        void SetUpMesh();

        glm::mat4 mModelMatrix{};

        RendererContext *mCtx;
        VkPipeline mGizmoPipeline{};
        VkViewport mViewport{};
        VkRect2D mScissors{};
        VkPipelineLayout mLayout{};
    public:
        explicit Gizmos(RendererContext *ctx);
        ~Gizmos();

        void DrawGizmos(size_t currentImageIndex);

        void SetModelMatrix(const glm::mat4 &modelMatrix) {
            mModelMatrix = modelMatrix;
        }
    };
}
#endif //SMALLVKENGINE_GIZMOS_H
