#ifndef _UNIQUE_PTR_H
#define _UNIQUE_PTR_H

#include "new.h"

namespace rtl {

template <typename T>
class unique_ptr {
   public:
    // Constructor: initializes ptr with the given raw pointer
    explicit unique_ptr(T* ptr = nullptr) : ptr(ptr) {}

    // Destructor: deletes the object pointed to by ptr
    ~unique_ptr() {
        delete ptr;
    }

    // Move constructor: transfers ownership of the resource
    unique_ptr(unique_ptr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;  // Set the other pointer to nullptr to avoid double deletion
    }

    // Move assignment operator: transfers ownership of the resource
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this != &other) {
            delete ptr;           // Delete current resource if any
            ptr = other.ptr;      // Transfer ownership
            other.ptr = nullptr;  // Set the other pointer to nullptr to avoid double deletion
        }
        return *this;
    }

    // Deleted copy constructor: unique_ptr cannot be copied
    unique_ptr(const unique_ptr&) = delete;

    // Deleted copy assignment operator: unique_ptr cannot be copied
    unique_ptr& operator=(const unique_ptr&) = delete;

    // Dereference operator
    T& operator*() const {
        return *ptr;
    }

    // Arrow operator
    T* operator->() const {
        return ptr;
    }

    // Get the raw pointer
    T* get() const {
        return ptr;
    }

    // Check if the pointer is null
    explicit operator bool() const {
        return ptr != nullptr;
    }

    // Release the ownership of the resource (return the raw pointer)
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Reset the unique_ptr with a new raw pointer (delete old resource)
    void reset(T* new_ptr = nullptr) {
        delete ptr;
        ptr = new_ptr;
    }

   private:
    T* ptr;  // Pointer to the object
};

// Helper function to make a unique_ptr from a raw pointer
template <typename T>
unique_ptr<T> make_unique(T* ptr) {
    return unique_ptr<T>(ptr);
}
};  // namespace rtl

#endif