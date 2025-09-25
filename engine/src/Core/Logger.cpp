//
// Created by ghima on 25-09-2025.
//
#include "Core/Logger.h"
#include "imgui/imgui.h"

namespace vk {
    Logger *Logger::instance = nullptr;
    rn::BlockingQueue<Log> Logger::mLogQueue{};


    Logger *Logger::GetInstance() {
        if (instance == nullptr) {
            instance = new Logger();
            instance->StartLoggerThread();
        }
        return instance;
    }

    void Logger::StartLoggerThread() {
        std::thread logThread{[this]() -> void {
            while (true) {
                Log entry = mLogQueue.Pop();
                {
                    std::lock_guard<std::mutex> lock{mMutex};
                    mLogs.push_back(entry);
                    if (mLogs.size() > Constants::MAX_LOGS) {
                        mLogs.erase(mLogs.begin());
                    }
                }

            }
        }};
        logThread.detach();
    }

    void Logger::WriteLog(const Log &log) {
        mLogQueue.Push(log);
    }

    void Logger::SetUpLogConsole() {
        ImGui::Begin("Console");
        std::lock_guard<std::mutex> guard{mMutex};
        for (const Log &log: mLogs) {
            auto inTime = std::chrono::system_clock::to_time_t(log.timeStamp);

            struct tm buff{};
            localtime_s(&buff, &inTime);
            char timeBuff[16]{'\0'};
            strftime(timeBuff, sizeof(timeBuff), "%H:%M:%S", &buff);

            ImVec4 color;
            switch (log.type) {
                case LogType::INFO:
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    break;
                case LogType::WARN:
                    color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);
                    break;
                case LogType::ERROR:
                    color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                    break;
            }

            ImGui::TextColored(color, "[%s] : %s", timeBuff, log.message);
        }
        ImGui::End();
    }
}
