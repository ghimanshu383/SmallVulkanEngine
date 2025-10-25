//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_MEMORY_H
#define SMALLVKENGINE_CORE_MEMORY_H

#include "core_platform.h"
#include "core_assert.h"
#include <stdlib.h>

namespace rn {
    TH_INLINE void memory_copy(void *dst, const void *src, size_t size) {
        memcpy(dst, src, size);
    }

    TH_INLINE void memory_zero(void *dst, size_t size) {
        memset(dst, 0, size);
    }

    TH_INLINE size_t memory_align(size_t size, size_t align) {
        size_t mask = align - 1;
        return (size + mask) & ~mask;
    }

    TH_INLINE void *memory_allocate(size_t size, size_t alignment = 16) {
        void *ptr = nullptr;
#if PLATFORM_WINDOWS
        ptr = _aligned_malloc(size, alignment);
#else
        posix_memalign(&ptr, alignment, size);
#endif
        TH_ENSURE(ptr != nullptr, "Memory Allocation Failed");
        return ptr;
    }

    TH_INLINE void free_memory(void *ptr) {
#if PLATFORM_WINDOWS
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
}
#endif //SMALLVKENGINE_CORE_MEMORY_H


#define ALIGN_16    16
#define ALIGN_32    32
#define ALIGN_64    64

#define rKilo(size)     (size* 1024)
#define rMega(size)     (size * 1024 * 1024)
#define rGiga(size)     (size * 1024 * 1024)
