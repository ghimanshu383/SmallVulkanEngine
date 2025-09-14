//
// Created by ghima on 14-09-2025.
//

#ifndef SMALLVKENGINE_MODELCOMPONENT_H
#define SMALLVKENGINE_MODELCOMPONENT_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Component.h"
#include "Core/Constants.h"


namespace vk {

    class ModelComponent : public Component {
    private:
        List<std::shared_ptr<class MeshComponent>> mMeshCompList;
        static List<std::string> mTextures ;

    public:
        ModelComponent(class GameObject *gameObject, const std::string &id, const std::string &objectFile);

        bool LoadModel(const std::string &fileName);

        static void LoadTextureMaterials(const aiScene *scene, List<std::string> &textureList);

        const std::shared_ptr<MeshComponent> GetMeshComponent(std::uint32_t index) const;

        const std::uint32_t GetMeshCount() const {
            return mMeshCompList.size();
        }
    };
}
#endif //SMALLVKENGINE_MODELCOMPONENT_H
