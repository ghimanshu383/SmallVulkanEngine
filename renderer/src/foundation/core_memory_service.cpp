//
// Created by ghima on 25-10-2025.
//
#include <foundation/core_memory_service.h>

namespace rn {
    MemoryService g_memoryService{};

    void MemoryService::init(size_t systemHeapSize, size_t scratchSize) {
        m_systemAllocator = new HeapAllocator();
        m_scratchAllocator = new LinearAllocator();
        m_scratchPtr = memory_allocate(scratchSize, ALIGN_64);
        m_scratchAllocator->init(m_scratchPtr, scratchSize);
        LOG_INFO("Memory Service Initialized Scratch Size {} MB and Heap On Demand {}", scratchSize / rMega(1));
    }

    void MemoryService::shutdown() {
        if (m_scratchPtr) {
            free_memory(m_scratchPtr);
        }
        if (m_scratchAllocator) {
            m_scratchAllocator->shutdown();
            delete m_scratchAllocator;
            m_scratchAllocator = nullptr;
        }
        if (m_systemAllocator) {
            delete m_systemAllocator;
            m_systemAllocator = nullptr;
        }
        LOG_INFO("Memory Service ShutDown complete");
    }
}