//
// Created by ghima on 24-09-2025.
//

#ifndef SMALLVKENGINE_BLOCKINGQUEUE_H
#define SMALLVKENGINE_BLOCKINGQUEUE_H

#include <mutex>
#include <queue>

namespace rn {
    template<typename T>
    class BlockingQueue {
    public:

        void Push(const T &item) {
            {
                std::lock_guard<std::mutex> lockGuard{mutex_};
                queue.push(item);
            }
            cv_.notify_one();
        }

        T Pop() {
            std::unique_lock<std::mutex> lock{mutex_};
            cv_.wait(lock, [this]() -> bool { return !(queue.empty()); });
            T item = queue.front();
            queue.pop();
            return item;
        }

    private:
        std::mutex mutex_{};
        std::condition_variable cv_{};
        std::queue<T> queue{};
    };
}
#endif //SMALLVKENGINE_BLOCKINGQUEUE_H
