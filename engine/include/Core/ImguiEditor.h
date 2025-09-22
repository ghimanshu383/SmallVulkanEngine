//
// Created by ghima on 23-09-2025.
//

#ifndef SMALLVKENGINE_IMGUIEDITOR_H
#define SMALLVKENGINE_IMGUIEDITOR_H
namespace vk {
    class ImguiEditor {
    private:
        ImguiEditor() = default;

        static ImguiEditor *instance;
    public:
        static ImguiEditor *GetInstance();

        void Init();

        void Tick(float DeltaTime);
    };
}
#endif //SMALLVKENGINE_IMGUIEDITOR_H
