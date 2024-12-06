/// @file string wrapper
#ifndef _STRING_H
#define _STRING_H

#include <string.h>

#include "common.h"
#include "hash.h"
#include "memory.h"

namespace rtl {

template <typename T, class Alloc = allocator<T>>
class basic_string {
   public:
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;

    basic_string() : size_(0), capacity_(kLocalSize - 1) { ; }

    basic_string(const_pointer ptr) : basic_string(ptr, length(ptr)) { ; }

    basic_string(const_pointer ptr, size_t size) : size_(size), capacity_(capacity(size)) {
        if (capacity_ > kLocalSize - 1) {
            data_ = Alloc().allocate(capacity_ + 1);
            assert(data_);
            new (data_) T[capacity_ + 1]();
            memcpy(data_, ptr, size_ * sizeof(T));
        } else {
            memcpy(local_, ptr, size_ * sizeof(T));
        }
    }

    basic_string(const basic_string& other) {
        basic_string tmp(other.data(), other.size_);
        swap(tmp);
    }

    basic_string& operator=(const basic_string& other) {
        basic_string tmp(other.data(), other.size_);
        swap(tmp);
    }

    ~basic_string() {
        if (capacity_ > kLocalSize - 1) {
            if (data_) {
                Alloc().deallocate(data_, capacity_ + 1);
                data_ = nullptr;
            }
        }
    }

    T& operator[](size_t pos) {
        return data()[pos];
    }

    const T& operator[](size_t pos) const {
        return data()[pos];
    }

    bool operator==(const basic_string& str) const {
        if (str.size_ != size_ || capacity_ != str.capacity_) {
            return false;
        }

        return memcmp(data(), str.data(), size_ * sizeof(T)) == 0;
    }

    bool operator!=(const basic_string& str) const {
        return !(*this == str);
    }

    basic_string& append(const T* ptr) {
        return append(ptr, length(ptr));
    }

    basic_string& append(const T* ptr, size_t n) {
        if (n + size_ > capacity_) {
            reserve(n + size_);
        }

        if (capacity_ > kLocalSize - 1) {
            memcpy(data_ + size_, ptr, n * sizeof(T));
            data_[size_ + n] = {};
        } else {
            memcpy(local_ + size_, ptr, n * sizeof(T));
            local_[size_ + n] = {};
        }
        size_ += n;
        return *this;
    }

    void clear() {
        size_ = 0;
    }

    void swap(basic_string& right) {
        size_t size = 0;
        size_t capacity = 0;
        T local[kLocalSize] = {0};
        size = right.size_;
        capacity = right.capacity_;
        memmove(local, right.local_, sizeof(local));

        right.size_ = size_;
        right.capacity_ = capacity_;
        memmove(right.local_, local_, sizeof(right.local_));

        size_ = size;
        capacity_ = capacity;
        memmove(local_, local, sizeof(local_));
    }

    void upper() {
        for (auto i = 0; i < size_; i++) {
            pointer p = data();
            if (p[i] >= T('a') && p[i] <= T('z')) {
                p[i] = p[i] - T('a') + T('A');
            }
        }
    }

    void lower() {
        for (auto i = 0; i < size_; i++) {
            pointer p = data();
            if (p[i] >= T('A') && p[i] <= T('Z')) {
                p[i] = p[i] - T('A') + T('a');
            }
        }
    }

    const_pointer c_str() const {
        return data();
    }

    pointer data() {
        if (capacity_ > kLocalSize - 1) {
            return data_;
        }
        return local_;
    }

    const_pointer data() const {
        if (capacity_ > kLocalSize - 1) {
            return data_;
        }
        return local_;
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return capacity_;
    }

   private:
    void reserve(size_t n) {
        n = capacity(n);
        if (n > capacity_) {
            Alloc al;
            T* tmp = al.allocate(n + 1);
            assert(tmp);
            for (auto i = 0; i < size_; i++) {
                new (tmp + i) T(data()[i]);
            }

            if (capacity_ > kLocalSize - 1) {
                al.deallocate(data_, size_);
            }
            data_ = tmp;
            capacity_ = n;
        }
    }

    size_t capacity(size_t n) const {
        if (n > kLocalSize - 1) {
            return (n * sizeof(T) / kAllocSize + 1) * kLocalSize - 1;
        }
        return kLocalSize - 1;
    }

    size_t length(const T* ptr) const {
        size_t count = 0;
        while (*ptr != T()) {
            ptr++;
            count++;
        }
        return count;
    }

   private:
    static const size_t kAllocSize = 16;
    static const size_t kLocalSize = kAllocSize / sizeof(T);

    size_t size_ = 0;
    size_t capacity_ = 0;
    union {
        pointer data_;
        T local_[kLocalSize] = {};
    };
};

template <class _Kty>
struct hash<rtl::basic_string<_Kty>> : _Conditionally_enabled_hash<rtl::basic_string<_Kty>, true> {
    static size_t _Do_hash(const rtl::basic_string<_Kty>& _Keyval) noexcept {
        // hash _Keyval to size_t value by pseudorandomizing transform
        return _Fnv1a_append_bytes(_FNV_offset_basis, reinterpret_cast<const unsigned char*>(_Keyval.c_str()), _Keyval.size() * sizeof(_Kty));
    }
};

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
}  // namespace rtl

#endif
