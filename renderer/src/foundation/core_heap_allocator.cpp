//
// Created by ghima on 25-10-2025.
//
#include "foundation/core_heap_allocator.h"
#include "foundation/core_platform.h"
#include "foundation/core_memory.h"

namespace rn {
    void *HeapAllocator::allocate(size_t size, size_t align) {
        return memory_allocate(size, align);
    }

    void HeapAllocator::deallocate(void *ptr) {
        free_memory(ptr);
    }
}