//
// Created by ghima on 25-09-2025.
//

#ifndef SMALLVKENGINE_LOGGER_H
#define SMALLVKENGINE_LOGGER_H

#include "Utility.h"
#include "BlockingQueue.h"
#include "Constants.h"

namespace vk {
    enum class LogType {
        INFO,
        ERROR,
        WARN
    };
    struct Log {
        LogType type{};
        const char *message{};
        std::chrono::system_clock::time_point timeStamp = std::chrono::system_clock::now();
    };

    class Logger {
    private:
        static Logger *instance;

        Logger() = default;

        static rn::BlockingQueue<Log> mLogQueue;
        List<Log> mLogs{};
        std::mutex mMutex;

    public:
        static Logger *GetInstance();

        void StartLoggerThread();

        void WriteLog(const Log &log);

        void SetUpLogConsole();
    };
}
#endif //SMALLVKENGINE_LOGGER_H
