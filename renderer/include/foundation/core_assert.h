//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_ASSERT_H
#define SMALLVKENGINE_CORE_ASSERT_H

#include "precomp.h"

namespace rn {
    void DefaultHandler(const char *condition, const char *message, const char *file, int line) {
        LOG_ERROR("{} ( {} : {}) Assertion Failed for {}", message, file, line, condition);
        TH_DEBUG_BREAK;
    }

    using AssertHandler = std::function<void(const char *, const char *message, const char *file, int line)>;
    static AssertHandler _handlerRenderer = DefaultHandler;
    static AssertHandler _handlerEngine = DefaultHandler;

    void SetAssertHandlerEnsure(AssertHandler & handler) {
        _handlerRenderer = handler;
    }

    void SetAssertHandlerCheck(AssertHandler & handler) {
        _handlerEngine = handler;
    }

    void TriggerAssertEnsure(const char *condition, const char *message, const char *file, int line) {
        _handlerRenderer(condition, message, file, line);
    }
    void TriggerAssertCheck(const char *condition, const char *message, const char *file, int line) {
        _handlerEngine(condition, message, file, line);
    }


#define TH_ENSURE(cond, msg)\
    do { if (!(cond)) rn::TriggerAssertEnsure(#cond, msg, __FILE__, __LINE__); } while(0)
#define TH_CHECK(cond, msg)\
    do { if (!(cond)) rn::TriggerAssertCheck(#cond, msg, __FILE__, __LINE__); } while(0)
}
#endif //SMALLVKENGINE_CORE_ASSERT_H
