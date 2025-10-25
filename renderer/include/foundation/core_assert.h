//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_ASSERT_H
#define SMALLVKENGINE_CORE_ASSERT_H

#include "Utility.h"

namespace rn {
    void DefaultHandler(const char *condition, const char *message, const char *file, int line);

    using AssertHandler = std::function<void(const char *, const char *message, const char *file, int line)>;
    static AssertHandler _handlerRenderer = DefaultHandler;
    static AssertHandler _handlerEngine = DefaultHandler;

    void SetAssertHandlerEnsure(AssertHandler &handler);

    void SetAssertHandlerCheck(AssertHandler &handler);

    void TriggerAssertEnsure(const char *condition, const char *message, const char *file, int line);

    void TriggerAssertCheck(const char *condition, const char *message, const char *file, int line);


#define TH_ENSURE(cond, msg)\
    do { if (!(cond)) rn::TriggerAssertEnsure(#cond, msg, __FILE__, __LINE__); } while(0)
#define TH_CHECK(cond, msg)\
    do { if (!(cond)) rn::TriggerAssertCheck(#cond, msg, __FILE__, __LINE__); } while(0)
}
#endif //SMALLVKENGINE_CORE_ASSERT_H
