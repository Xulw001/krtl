/// @file Double linked list (with ListEntry)
#ifndef _LIST_H
#define _LIST_H

#include "memory.h"
#include "struct.h"

namespace rtl {

//
// Internal element
//
template <typename T>
struct __list_val : public ListEntry {
    T val_;

    __list_val() = delete;
    template <typename... _Valty>
    __list_val(_Valty&&... val) : val_(val...), ListEntry() { ; }
};

//
// iterator
//
template <class T, class Ref, class Ptr>
struct __list_iterator {
   public:
    typedef __list_val<T>* pointer;

    __list_iterator() = default;
    __list_iterator(pointer ptr) : ptr_(ptr) { ; }

    pointer ptr_;

    //
    // Access to value
    //
    Ref operator*() {
        return ptr_->val_;
    }

    Ptr operator->() {
        return &ptr_->val_;
    }

    //
    // Check the end
    //
    bool operator!=(const __list_iterator& it) const {
        return ptr_ != it.ptr_;
    }

    bool operator==(const __list_iterator& it) const {
        return ptr_ == it.ptr_;
    }

    //
    // Get next item
    //
    __list_iterator& operator++() {
        ptr_ = static_cast<pointer>(ptr_->next);
        return *this;
    }

    __list_iterator operator++(int) {
        __list_iterator tmp(*this);
        ptr_ = static_cast<pointer>(ptr_->next);
        return tmp;
    }

    //
    // Get prev item
    //
    __list_iterator& operator--() {
        ptr_ = static_cast<pointer>(ptr_->prev);
        return *this;
    }

    __list_iterator& operator--(int) {
        __list_iterator tmp(*this);
        ptr_ = static_cast<pointer>(ptr_->prev);
        return tmp;
    }
};

template <typename T, class Alloc = allocator<T>>
class list {
   public:
    using iterator = __list_iterator<T, T&, T*>;
    using const_iterator = __list_iterator<T, const T&, const T*>;
    using pointer = typename iterator::pointer;
    using const_pointer = typename const_iterator::pointer;
    using allocator_type = typename Alloc::template rebind<__list_val<T>>::other;
    using size_type = size_t;
    using value_type = T;

   public:
    list() = default;
    ~list() { clear(); }
    list(const list& other) = delete;
    list& operator=(const list& other) = delete;

    void clear() {
        iterator it = begin();
        while (it != end()) {
            it = erase(it);
        }
    }

    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    [[nodiscard]] size_type size() const {
        return size_;
    }

    void pop_front() {
        erase(begin());
    }

    void pop_back() {
        erase(--end());
    }

    T& front() {
        return static_cast<pointer>(head_.next)->val_;
    }

    const T& front() const {
        return static_cast<pointer>(head_.next)->val_;
    }

    T& back() {
        return static_cast<pointer>(head_.prev)->val_;
    }

    const T& back() const {
        return static_cast<pointer>(head_.prev)->val_;
    }

    iterator begin() {
        return iterator(static_cast<pointer>(head_.next));
    }

    iterator end() {
        return iterator(static_cast<pointer>(&head_));
    }

    const_iterator begin() const {
        return const_iterator(static_cast<const_pointer>(head_.next));
    }

    const_iterator end() const {
        return const_iterator(static_cast<const_pointer>(&head_));
    }

    iterator erase(iterator pos) {
        iterator tmp = pos++;
        RemoveEntryList(tmp.ptr_);
        tmp.ptr_->~__list_val();
        allocator_type().deallocate(tmp.ptr_, 0);
        size_--;
        return pos;
    }

    void splice(iterator where, list& right, iterator first, iterator last) {
        if (first != last && (this != &right || where != last || where != first)) {
            size_type count = 0;
            if (this != &right) {
                if (first == right.begin() && last == right.end()) {
                    count = right.size_;
                } else {
                    for (auto it = first; it != last; it++, count++) {
                        ;
                    }
                }
            }

            splice(where, right, first, last, count);
        }
    }

    template <typename... _Valty>
    void emplace_front(_Valty&&... val) {
        emplace(begin(), val...);
    }

    template <typename... _Valty>
    void emplace_back(_Valty&&... val) {
        emplace(end(), val...);
    }

   private:
    template <typename... _Valty>
    void emplace(iterator pos, _Valty&&... val) {
        __list_val<T>* node = allocator_type().allocate(1);
        assert(node);
        new (node) __list_val<T>(val...);
        InsertTailList(pos.ptr_, node);
        size_++;
    }

    void splice(iterator where, list& right, iterator first, iterator last, size_type count) {
        if (this != &right) {
            size_ += count;
            right.size_ -= count;
        }

        // fixup the _Next values
        const auto first_prev = first.ptr_->prev;
        first_prev->next = last.ptr_;
        const auto last_prev = last.ptr_->prev;
        last_prev->next = where.ptr_;
        const auto where_prev = where.ptr_->prev;
        where_prev->next = first.ptr_;

        // fixup the _Prev values
        where.ptr_->prev = last_prev;
        last.ptr_->prev = first_prev;
        first.ptr_->prev = where_prev;
    }

   private:
    ListEntry head_ = {};  //< Head
    size_type size_ = 0;
};

}  // namespace rtl

#endif
/// @}