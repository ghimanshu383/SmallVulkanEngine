//
// Created by ghima on 25-10-2025.
//
#include "foundation/core_linear_allocator.h"

namespace rn {
    void *LinearAllocator::allocate(size_t size, size_t align) {
        if (size == 0) return nullptr;
        size_t alignOffset = (m_offset + align - 1) & ~(align - 1);
        TH_ENSURE(size <= m_total_size, "Size Requested is greater than total size");
        TH_ENSURE(alignOffset + size <= m_total_size, "Linear Allocator out of memory");
        void *ptr = m_start + alignOffset;
        m_offset = alignOffset + size;
        return ptr;
    }

    void LinearAllocator::deallocate(void *ptr) {

    }

    void LinearAllocator::reset() {
        m_offset = 0;
    }

    void LinearAllocator::shutdown() {
        m_start = nullptr;
        m_total_size = 0;
        m_offset = 0;
    }

}