//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_PLATFORM_H
#define SMALLVKENGINE_CORE_PLATFORM_H

#include <cstdint>
#include <cstddef>

#if !defined(_MSC_VER)
#include <signal.h>
#endif

// Checking the compiler version
#if defined (_MSC_VER)
#define COMPILER_MSVC 1
#elif defined(__clang__)
#define COMPILER_CLANG 1
#elif defined(__GNUC__)
#define COMPILER_GCC 1
#else
#error "unknown Platorm"
#endif

// Platform Detection
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define PLATFORM_MAC 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#else
#error "Unknown Platform"
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X64 1
#elif defined(__i386__) || defined (_M_IX86)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error "unknow Architecture"
#endif

// Build Configurations
#if defined(_DEBUG) || defined(DEBUG)
#define BUILD_DEBUG 1
#else
#define BUILD_RELEASE 1
#endif

// Break Point Macros
#if COMPILER_MSVC
#define TH_INLINE inline
#define TH_FINLINE __forceinline
#define TH_DEBUG_BREAK __debugbreak()
#elif COMPILER_GCC || COMPILER_CLANG
#define TH_INLINE inline
#define TH_FINLINE inline __attribute__((always_inline))
#define TH_DEBUG_BREAK raise(SIGTRAP)
#endif

// Utility Macros
#define TH_STRINGIZE(L) #L
#define TH_MAKE_STRING(L) TH_STRINGIZE(L)
#define TH_CONCAT(x, y) x##y
#define TH_LINE_STRING TH_MAKE_STRING(__LINE__)
#define TH_FILELINE(MESSAGE) __FILE__ "(" TH_LINE_STRING ") : " MESSAGE
#define TH_UNIQUE(PARAM) TH_CONCAT(PARAM, __LINE__)
#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array[0]))

// Basic Type Alias
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t sizet;
typedef const char *cstring;

#endif //SMALLVKENGINE_CORE_PLATFORM_H
