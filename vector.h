#ifndef _VECTOR_H
#define _VECTOR_H

#include <string.h>

#include "common.h"
#include "memory.h"

namespace rtl {

template <class T, class Alloc = allocator<T>>
class vector {
   public:
    using iterator = T*;
    using const_iterator = const T*;
    using allocator_type = Alloc;

   public:
    vector() = default;

    ~vector() {
        clear();
        if (start_) {
            allocator_type().deallocate(start_, 0);
            start_ = nullptr;
        }
    }

    vector(const vector&) = delete;             // TODO
    vector& operator=(const vector&) = delete;  // TODO

    /// @brief destructor all elems
    void clear() {
        while (start_ < end_) {
            (--end_)->~T();
        }
    }

    _NODISCARD bool empty() const {
        return size() == 0;
    }

    T& operator[](size_t pos) {
        return start_[pos];
    }

    const T& operator[](size_t pos) const {
        return start_[pos];
    }

    _NODISCARD iterator begin() {
        return start_;
    }

    _NODISCARD iterator end() {
        return end_;
    }

    _NODISCARD const_iterator begin() const {
        return start_;
    }

    _NODISCARD const_iterator end() const {
        return end_;
    }

    _NODISCARD size_t size() const {
        return end_ - start_;
    }

    _NODISCARD size_t capacity() const {
        return last_ - start_;
    }

    void reserve(size_t n) {
        if (n > capacity()) {
            size_t pos = size();
            allocator_type al;
            T* tmp = al.allocate(n);
            assert(tmp);
            memmove(tmp, start_, pos * sizeof(T));
            al.deallocate(start_, pos);
            start_ = tmp;
            end_ = tmp + pos;
            last_ = tmp + n;
        }
    }

    void resize(size_t n, const T& val = T()) {
        while (start_ + n < end_) {
            (--end_)->~T();
        }

        if (n > capacity()) {
            reserve(n);
        }

        while (end_ < start_ + n) {
            new (end_) T(val);
            end_++;
        }
    }

    void assign(size_t n, const T& val) {
        end_ = start_;
        resize(n, val);
    }

    template <typename... _Valty>
    void emplace_back(_Valty&&... val) {
        if (end_ == last_) {
            reserve(capacity() == 0 ? 4 : capacity() * 2);
        }
        new (end_) T(val...);
        end_++;
    }

   private:
    iterator start_ = nullptr;
    iterator end_ = nullptr;
    iterator last_ = nullptr;
};

}  // namespace rtl

#endif
