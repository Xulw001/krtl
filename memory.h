#ifndef _MEMORY_HPP
#define _MEMORY_HPP

#include <cstddef>

#include "new.h"

namespace rtl {
template <typename T, PoolTag Tag = PoolTag::NonPaged>
class allocator {
   public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = allocator<U, Tag>;
    };

    allocator() = default;
    ~allocator() = default;

    template <typename U>
    allocator(const allocator<U, Tag>&) {}

    T* allocate(size_t n) const {
        return static_cast<T*>(::operator new(n * sizeof(T), Tag));
    }

    void deallocate(T* p, size_t) const {
        ::operator delete(p, Tag);
    }
};

template <typename Alloc>
struct pool_tag;

template <typename T, PoolTag tag>
struct pool_tag<allocator<T, tag>> {
    static constexpr PoolTag value = tag;
};

}  // namespace rtl

#endif