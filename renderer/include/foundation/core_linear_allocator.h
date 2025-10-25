//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_LINEAR_ALLOCATOR_H
#define SMALLVKENGINE_CORE_LINEAR_ALLOCATOR_H

#include "core_platform.h"
#include "core_assert.h"
#include "Utility.h"

namespace rn {
    struct LinearAllocator : public MemoryAllocator {
        u8 *m_start = nullptr;
        size_t m_total_size = 0;
        size_t m_offset = 0;

        void init(void *memoryAdd, size_t size) {
            m_start = reinterpret_cast<u8 *>(memoryAdd);
            m_total_size = size;
            m_offset = 0;
        }

        virtual void *allocate(size_t size, size_t align) override;

        virtual void deallocate(void *ptr) override;

        void shutdown();

        void reset();
    };
}
#endif //SMALLVKENGINE_CORE_LINEAR_ALLOCATOR_H
