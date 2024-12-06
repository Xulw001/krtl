#ifndef _NEW_HPP
#define _NEW_HPP

#include <cstddef>

enum class PoolTag {
    Paged,
    NonPaged,
    NonPagedExecute,
    NonPagedNx,
};

// allocation new
void* __cdecl operator new(size_t n, PoolTag tag);
void* __cdecl operator new[](size_t n, PoolTag tag);

// placement new
void* __cdecl operator new(size_t, void* p) noexcept;
void* __cdecl operator new[](size_t, void* p) noexcept;

void assert(void* p);

//
// allocation delete
//
void __cdecl operator delete(void* p, PoolTag) noexcept;
void __cdecl operator delete[](void* p, PoolTag) noexcept;
void __cdecl operator delete(void* p) noexcept;
void __cdecl operator delete[](void* p) noexcept;
void __cdecl operator delete(void* p, size_t) noexcept;
void __cdecl operator delete[](void* p, size_t) noexcept;

#endif