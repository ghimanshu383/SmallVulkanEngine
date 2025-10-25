//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_TH_UTIL_H
#define SMALLVKENGINE_TH_UTIL_H

#include "Core/Logger.h"

#define TH_LOG_INFO(MSG) \
    vk::Logger::GetInstance()->WriteLog({ vk::LogType::INFO, MSG })
#define TH_LOG_WARN(MSG) \
vk::Logger::GetInstance()->WriteLog({ vk::LogType::WARN, TH_FILELINE(MSG) })
#define TH_LOG_ERROR(MSG)\
vk::Logger::GetInstance()->WriteLog({ vk::LogType::ERROR, TH_FILELINE(MSG) })
#endif //SMALLVKENGINE_TH_UTIL_H
