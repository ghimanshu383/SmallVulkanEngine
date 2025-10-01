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
        const float ARROW_BASE_SIZE = 0.08f;
        const float ARROW_TIP_LENGTH = 0.15f;
        const float CUBE_SIZE = .04f;
        uint32_t rotationStartIndex = 0.0;

        enum class TOPOLOGY_TYPE {
            LINES,
            TRIANGLES,
            LINE_STRIP
        };


        void CreatePipeline(TOPOLOGY_TYPE type);

        void BuildRotationGizmo(std::vector<Vertex> &verts,
                                std::vector<uint32_t> &indices,
                                float AXIS_LENGTH,
                                int segments = 64,
                                float radiusMultiplier = .90f);

        void SetUpMesh();

        void DrawRotationGizmo(std::uint32_t currentImageIndex);
        void DrawTranslateScaleGizmo(std::uint32_t currentImageIndex);

        glm::mat4 mModelMatrix{};

        RendererContext *mCtx;
        VkPipeline mGizmoPipelineLines{};
        VkPipeline mGizmoPipelineTriangles{};
        VkPipeline mGizmoPipelineLineStrip{};
        VkViewport mViewport{};
        VkRect2D mScissors{};
        VkPipelineLayout mLayoutLines{};
        VkPipelineLayout mLayoutLineStrip{};
        VkPipelineLayout mLayoutTriangles{};
        GIZMO_TYPE mGizmoType = GIZMO_TYPE::TRANSLATE;
    public:
        explicit Gizmos(RendererContext *ctx);

        ~Gizmos();

        void DrawGizmos(size_t currentImageIndex);


        void SetModelMatrix(const glm::mat4 &modelMatrix) {
            mModelMatrix = modelMatrix;
        }

        void SetGizmoType(GIZMO_TYPE type) {
            mGizmoType = type;
        }

        const GIZMO_TYPE &GetGizmoType() { return mGizmoType; }
    };
}
#endif //SMALLVKENGINE_GIZMOS_H
