//
// Created by ghima on 25-10-2025.
//
#include "foundation/core_assert.h"
namespace rn {
    void DefaultHandler(const char *condition, const char *message, const char *file, int line) {
        LOG_ERROR("{} ( {} : {}) Assertion Failed for {}", message, file, line, condition);
        TH_DEBUG_BREAK;
    }
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
}