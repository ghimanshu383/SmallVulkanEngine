//
// Created by ghima on 13-09-2025.
//

#ifndef SMALLVKENGINE_TRANSFORMCOMPONENT_H
#define SMALLVKENGINE_TRANSFORMCOMPONENT_H

#include "Component.h"

namespace vk {
    class TransformComponent : public Component {
    private:
        glm::mat4 mModelMatrix{1};
    public:
        explicit TransformComponent(GameObject *gameObject, const std::string &id);

        void setTranslate(glm::vec3 value);

        void setRotationX(float value);

        void setRotationY(float value);

        void setRotationZ(float value);

        void setScale(glm::vec3 value);

        const glm::mat4 &GetModelMatrix() const { return mModelMatrix; }
    };
}
#endif //SMALLVKENGINE_TRANSFORMCOMPONENT_H
