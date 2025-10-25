//
// Created by ghima on 26-10-2025.
//

#ifndef SMALLVKENGINE_CORE_ARRAY_H
#define SMALLVKENGINE_CORE_ARRAY_H

#include "core_memory.h"
#include "core_assert.h"

namespace rn {
    template<typename T>
    struct Array {
        T *m_data = nullptr;
        u32 m_size = 0;
        u32 m_capacity = 0;
        MemoryAllocator *m_allocator = nullptr;

        Array() = default;

        ~Array() = default;


        void init(MemoryAllocator *alloc, u32 initialCapacity = 0, u32 initialSize = 0);

        void shutdown();

        void grow(u32 newCapacity);

        void push(const T &element);

        void pop();

        void delete_swap(u32 index);

        void clear();

        void setSize(u32 size);

        void setCapacity(u32 capacity);

        T &back();

        const T &back() const;

        T &front();

        const T &front() const;

        u32 getSizeInBytes();

        u32 getCapacityInBytes();

        const T &operator[](u32 index) const {
            TH_ENSURE(index < m_size, "Index out of range");
            return m_data[index];
        }

        T &operator[](u32 index) {
            TH_ENSURE(index < m_size, "Index out of range");
            return m_data[index];
        }
    };

    template<typename T>
    TH_INLINE u32 Array<T>::getCapacityInBytes() {
        return m_capacity * sizeof(T);
    }

    template<typename T>
    TH_INLINE u32 Array<T>::getSizeInBytes() {
        return m_size * sizeof(T);
    }

    template<typename T>
    TH_INLINE const T &Array<T>::front() const {
        TH_ENSURE(m_size > 0, "Array is Empty");
        return m_data[0];
    }

    template<typename T>
    TH_INLINE T &Array<T>::front() {
        TH_ENSURE(m_size > 0, "Array is Empty");
        return m_data[0];
    }

    template<typename T>
    TH_INLINE const T &Array<T>::back() const {
        TH_ENSURE(m_size > 0, "Array is Empty");
        return m_data[m_size - 1];
    }

    template<typename T>
    TH_INLINE T &Array<T>::back() {
        TH_ENSURE(m_size > 0, "Array is Empty");
        return m_data[m_size - 1];
    }

    template<typename T>
    TH_INLINE void Array<T>::setCapacity(u32 capacity) {
        if (capacity > m_capacity) {
            grow(capacity);
        }
    }

    template<typename T>
    TH_INLINE void Array<T>::setSize(u32 size) {
        if (size > m_capacity) {
            grow(size);
        }
        m_size = size;
    }

    template<typename T>
    TH_INLINE void Array<T>::clear() {
        m_size = 0;
    }

    template<typename T>
    TH_INLINE void Array<T>::delete_swap(u32 index) {
        TH_ENSURE(index < m_size, "Invalid Index for Delete Swap");
        m_data[index] = m_data[--m_size];
    }

    template<typename T>
    TH_INLINE void Array<T>::pop() {
        TH_ENSURE(m_size > 0, "Array underflow on pop");
        --m_size;
    }

    template<typename T>
    TH_INLINE void Array<T>::push(const T &element) {
        if (m_size >= m_capacity) {
            grow(m_capacity + 1);
        }
        if constexpr (std::is_trivially_copyable<T>::value) {
            m_data[m_size++] = element;
        } else {
            // invoking the copy constructor here
            new(&m_data[m_size++]) T(element);
        }

    }

    template<typename T>
    TH_INLINE void Array<T>::init(rn::MemoryAllocator *alloc, u32 initialCapacity, u32 initialSize) {
        m_allocator = alloc;
        m_capacity = 0;
        m_size = initialSize;
        if (initialCapacity > 0) {
            grow(initialCapacity);
        }
    }

    template<typename T>
    TH_INLINE void Array<T>::grow(u32 newCapacity) {
        if (newCapacity < m_capacity * 2) {
            newCapacity = m_capacity * 2;
        } else if (newCapacity < 4) {
            newCapacity = 4;
        }
        T *new_data = (T *) (m_allocator->allocate(newCapacity * sizeof(T), alignof(T)));

        if (m_capacity) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                memory_copy(new_data, m_data, sizeof(T) * m_size);
            } else {
                for (u32 i = 0; i < m_size; i++) {
                    new(&new_data[i]) T(std::move(m_data[i]));
                    m_data[i].~T();
                }
            }

            m_allocator->deallocate(m_data);
        }
        m_data = new_data;
        m_capacity = newCapacity;
    }

    template<typename T>
    TH_INLINE void Array<T>::shutdown() {
        if (m_data && m_capacity > 0) {
            if constexpr (!std::is_trivially_copyable_v<T>) {
                for (u32 i = 0; i < m_size; i++) {
                    m_data[i].~T();
                }
            }
            m_allocator->deallocate(m_data);
        }
    }
}
#endif //SMALLVKENGINE_CORE_ARRAY_H
