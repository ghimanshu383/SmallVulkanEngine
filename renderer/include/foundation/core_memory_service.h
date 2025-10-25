//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_MEMORY_SERVICE_H
#define SMALLVKENGINE_CORE_MEMORY_SERVICE_H

#include "foundation/core_heap_allocator.h"
#include "foundation/core_linear_allocator.h"
#include "foundation/core_memory.h"

namespace rn {
    struct MemoryService {
        void init(size_t systemHeapSize = 0, size_t scratchSize = rMega(4));

        void shutdown();

        HeapAllocator *GetSystemAllocator() { return m_systemAllocator; };

        LinearAllocator *GetScratchAllocator() { return m_scratchAllocator; }

        static MemoryService *GetInstance();

    private:
        MemoryService();

        ~MemoryService();

        static MemoryService *m_instance;
        HeapAllocator *m_systemAllocator = nullptr;
        LinearAllocator *m_scratchAllocator = nullptr;
        void *m_scratchPtr = nullptr;
    };
}
#endif //SMALLVKENGINE_CORE_MEMORY_SERVICE_H
