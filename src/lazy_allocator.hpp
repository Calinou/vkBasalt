#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>
#include <vector>

namespace vkBasalt
{
    class LazyAllocator
    {
    public:
        LazyAllocator() = default;

        ~LazyAllocator() noexcept
        {
            for (void* mem : m_allocations)
                std::free(mem);
        }

        LazyAllocator(const LazyAllocator& other) = delete;
        LazyAllocator& operator=(const LazyAllocator& other) = delete;

        LazyAllocator(LazyAllocator&& other) noexcept : m_allocations(std::exchange(other.m_allocations, {}))
        {
        }

        LazyAllocator& operator=(LazyAllocator&& other) noexcept
        {
            std::swap(m_allocations, other.m_allocations);
            return *this;
        }

        template<typename T, typename = std::enable_if_t<std::is_trivially_default_constructible<T>::value>>
        [[nodiscard]] T* alloc(size_t count = 1) noexcept
        {
            void* mem = std::calloc(count, sizeof(T));

            m_allocations.push_back(mem);

            return reinterpret_cast<T*>(mem);
        }

    private:
        std::vector<void*> m_allocations;
    };
} // namespace vkBasalt
