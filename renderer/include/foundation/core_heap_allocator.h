//
// Created by ghima on 25-10-2025.
//

#ifndef SMALLVKENGINE_CORE_HEAP_ALLOCATOR_H
#define SMALLVKENGINE_CORE_HEAP_ALLOCATOR_H


#include "Utility.h"

namespace rn {
    struct HeapAllocator : public MemoryAllocator {
        HeapAllocator() = default;

        virtual void *allocate(size_t size, size_t align) override;

        virtual void deallocate(void *ptr) override;

    };
}
#endif //SMALLVKENGINE_CORE_HEAP_ALLOCATOR_H
