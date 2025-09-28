//
// Created by ghima on 30-08-2025.
//

#ifndef SMALLVKENGINE_MESHCOMPONENT_H
#define SMALLVKENGINE_MESHCOMPONENT_H

#include "Component.h"
#include "Core/Constants.h"
#include "Utility.h"
#include "StaticMesh.h"
#include "Core/GizmoController.h"

namespace vk {
    class GameObject;

    class MeshComponent : public Component {
    protected:
        List<rn::Vertex> mVertexList;
        List<std::uint32_t> mIndexList;
        rn::StaticMesh *mStaticMesh;
        std::string mTextureId;
        bool mCalculateNormals;
        GizmoDragController gizmoDragController{};
    public:
        MeshComponent(GameObject *gameObject, const std::string &id, List<rn::Vertex> &vertices,
                      List<std::uint32_t> &indices, std::string textureId = "", bool calculateNormals = false);

        ~MeshComponent();

        rn::StaticMesh *GetStaticMesh() const { return mStaticMesh; }

        virtual void BeginPlay() override;

        virtual void Tick(float DeltaTime) override;

    };
}
#endif //SMALLVKENGINE_MESHCOMPONENT_H
