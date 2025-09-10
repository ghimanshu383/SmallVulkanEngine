//
// Created by ghima on 05-09-2025.
//

#ifndef SMALLVKENGINE_INPUTSYSTEM_H
#define SMALLVKENGINE_INPUTSYSTEM_H
namespace vk {
    class InputSystem {
    private:
        static InputSystem *instance;
        bool Keys[1024]{false};
        bool mMouseKeys[2]{false};

        double *xChange{new double{0}};
        double *yChange{new double{0}};
        double *lastX{new double{0}};
        double *lastY{new double{0}};
        bool firstMove{false};

        InputSystem();

    public:
        static InputSystem *GetInstance();

        void KeyInputHandler(int key, int code, int action, int mode);

        bool *GetKeys() { return Keys; }

    };
}
#endif //SMALLVKENGINE_INPUTSYSTEM_H
