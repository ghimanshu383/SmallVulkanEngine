//
// Created by ghima on 05-09-2025.
//

#ifndef SMALLVKENGINE_DELEGATE_H
#define SMALLVKENGINE_DELEGATE_H

#include <functional>
#include "Core/Constants.h"

namespace vk {
    template<typename... Args>
    class Delegate {
        List<std::function<bool(Args...)>> mLamFunctions{};
    public:
        template<typename T>
        void Register(T *t, bool(T::*callback)(Args...)) {
            std::function<bool(Args...)> lamFunction = [t, callback](Args... args) -> bool {
                if (t != nullptr) {
                    return (t->*callback)(args...);
                }
                return false;
            };

            mLamFunctions.push_back(lamFunction);
        }

        void Invoke(Args... args) {
            typename List<std::function<bool(Args...)>>::iterator iter = mLamFunctions.begin();
            while (iter != mLamFunctions.end()) {
                if ((*iter)(args...)) {
                    iter++;
                } else {
                    iter = mLamFunctions.erase(iter);
                }
            }
        }
    };
}
#endif //SMALLVKENGINE_DELEGATE_H
