#include "new.h"

#if defined(_WIN32) && defined(_KRTL)
#include <ntifs.h>

static constexpr ULONG alloc_tag = 'nltr';

void* __cdecl operator new(size_t, void* p) noexcept { return p; }

void* __cdecl operator new[](size_t, void* p) noexcept { return p; }

#if defined(NTDDI_WIN10_MN) && WDK_NTDDI_VERSION >= NTDDI_WIN10_MN
static POOL_FLAGS Tag2PoolFlags(PoolTag tag) {
    switch (tag) {
        case PoolTag::Paged:
            return POOL_FLAG_PAGED;
        case PoolTag::NonPaged:
        case PoolTag::NonPagedNx:
            return POOL_FLAG_NON_PAGED;
        case PoolTag::NonPagedExecute:
            return POOL_FLAG_NON_PAGED_EXECUTE;
        default:
            return POOL_FLAG_PAGED;
    }
}

void* __cdecl operator new(size_t n, PoolTag tag) {
    PVOID p = nullptr;
    if (n) {
        p = ExAllocatePool2(Tag2PoolFlags(tag), n, alloc_tag);
    }
    return p;
}

void* __cdecl operator new[](size_t n, PoolTag tag) {
    PVOID p = nullptr;
    if (n) {
        p = ExAllocatePool2(Tag2PoolFlags(tag), n, alloc_tag);
    }
    return p;
}
#else
static POOL_TYPE Tag2PoolType(PoolTag tag) {
    switch (tag) {
        case PoolTag::Paged:
            return PagedPool;
        case PoolTag::NonPaged:
            return NonPagedPool;
        case PoolTag::NonPagedExecute:
            return NonPagedPoolExecute;
        case PoolTag::NonPagedNx:
            return NonPagedPoolNx;
        default:
            return PagedPool;
    }
}

void* __cdecl operator new(size_t n, PoolTag tag) {
    PVOID p = nullptr;
    if (n) {
        p = ExAllocatePoolWithTag(Tag2PoolType(tag), n, alloc_tag);
    }
    return p;
}

void* __cdecl operator new[](size_t n, PoolTag tag) {
    PVOID p = nullptr;
    if (n) {
        p = ExAllocatePoolWithTag(Tag2PoolType(tag), n, alloc_tag);
    }
    return p;
}
#endif
// allocation new

//
// allocation delete
//
void __cdecl operator delete(void* p) noexcept {
    if (p) {
        ExFreePoolWithTag(p, alloc_tag);
    }
}

void __cdecl operator delete[](void* p) noexcept {
    if (p) {
        ExFreePoolWithTag(p, alloc_tag);
    };
}

void assert(void* p) {
    if (p == nullptr) ExRaiseStatus(STATUS_NO_MEMORY);
}

#else
#include <malloc.h>

void* __cdecl operator new(size_t n, PoolTag tag) {
    return malloc(n);
}

void* __cdecl operator new[](size_t n, PoolTag tag) {
    return malloc(n);
}

void __cdecl operator delete(void* p) noexcept {
    if (p) {
        free(p);
    }
}

void __cdecl operator delete[](void* p) noexcept {
    if (p) {
        free(p);
    };
}

void assert(void* p) {
    ;
}

#endif

void __cdecl operator delete(void* p, size_t) noexcept {
    ::operator delete(p);
}

void __cdecl operator delete[](void* p, size_t) noexcept {
    ::operator delete(p);
}

void __cdecl operator delete(void* p, PoolTag) noexcept {
    ::operator delete(p);
}

void __cdecl operator delete[](void* p, PoolTag) noexcept {
    ::operator delete(p);
}