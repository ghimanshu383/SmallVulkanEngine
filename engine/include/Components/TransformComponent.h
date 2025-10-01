//
// Created by ghima on 13-09-2025.
//

#ifndef SMALLVKENGINE_TRANSFORMCOMPONENT_H
#define SMALLVKENGINE_TRANSFORMCOMPONENT_H

#include "Component.h"
#include "imgui/imgui.h"

namespace vk {
    class TransformComponent : public Component {
    private:
        glm::mat4 mModelMatrix{1};
        glm::vec3 mPosition{0};
        glm::mat4 mTest{1};
        glm::vec3 mRotations{0};
        glm::vec3 mScale{1};

    public:
        explicit TransformComponent(GameObject *gameObject, const std::string &id);

        virtual void BeginPlay() override;

        void setTranslate(glm::vec3 value);

        void setRotation(glm::vec3);

        void setScale(glm::vec3 value);

        const glm::mat4 &GetModelMatrix() const { return mModelMatrix; }

        void SetModelMatrixTranslate(const glm::vec3 pos) {
            mModelMatrix[3] = glm::vec4{pos, 1.0};
        }

        bool SetUpGuiInspector();

        void UpdateModelMatrix();

        void SetPosition(glm::vec3 position) {
            mPosition = position;
        }

    };
}
#endif //SMALLVKENGINE_TRANSFORMCOMPONENT_H
