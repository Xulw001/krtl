#ifndef _HASH_H
#define _HASH_H

#include <limits.h>
#include <stddef.h>

#include "common.h"

namespace rtl {

//////////////////////////////////////////////////////////////////////////
//
// is_const
//
template <class>
constexpr bool is_const_v = false;  // determine whether type argument is const qualified

template <class _Ty>
constexpr bool is_const_v<const _Ty> = true;

template <class _Ty>
struct is_const : bool_constant<is_const_v<_Ty>> {};

//////////////////////////////////////////////////////////////////////////
//
// is_pointer
//
template <class>
constexpr bool is_pointer_v = false;  // determine whether _Ty is a pointer

template <class _Ty>
constexpr bool is_pointer_v<_Ty*> = true;

template <class _Ty>
constexpr bool is_pointer_v<_Ty* const> = true;

template <class _Ty>
constexpr bool is_pointer_v<_Ty* volatile> = true;

template <class _Ty>
constexpr bool is_pointer_v<_Ty* const volatile> = true;

template <class _Ty>
struct is_pointer : bool_constant<is_pointer_v<_Ty>> {};

#if defined(_WIN64)
constexpr size_t _FNV_offset_basis = 0xcbf29ce484222325ull;
constexpr size_t _FNV_prime = 0x100000001b3ull;
#else   // ^^^ defined(_WIN64) / !defined(_WIN64) vvv
constexpr size_t _FNV_offset_basis = 0x811c9dc5u;
constexpr size_t _FNV_prime = 0x01000193u;
#endif  // ^^^ !defined(_WIN64) ^^^

_NODISCARD inline size_t _Fnv1a_append_bytes(size_t _Val, const unsigned char* const _First,
                                                const size_t _Count) noexcept {  // accumulate range [_First, _First + _Count) into partial FNV-1a hash _Val
    for (size_t _Idx = 0; _Idx < _Count; ++_Idx) {
        _Val ^= static_cast<size_t>(_First[_Idx]);
        _Val *= _FNV_prime;
    }

    return _Val;
}

template <class _Kty>
_NODISCARD size_t _Fnv1a_append_value(
    const size_t _Val, const _Kty& _Keyval) noexcept {  // accumulate _Keyval into partial FNV-1a hash _Val
    return _Fnv1a_append_bytes(_Val, &reinterpret_cast<const unsigned char&>(_Keyval), sizeof(_Kty));
}

template <class _Kty>
_NODISCARD size_t _Hash_representation(const _Kty& _Keyval) noexcept {  // bitwise hashes the representation of a key
    return _Fnv1a_append_value(_FNV_offset_basis, _Keyval);
}

template <class _Kty>
struct hash;

//
//
//
template <class _Kty, bool _Enabled>
struct _Conditionally_enabled_hash {
    // conditionally enabled hash base
    typedef _Kty argument_type;
    typedef size_t result_type;

    _NODISCARD size_t operator()(const _Kty& _Keyval) const {
        return (hash<_Kty>::_Do_hash(_Keyval));
    }
};

template <class _Kty>
struct _Conditionally_enabled_hash<_Kty, false> {
    // conditionally disabled hash base
    _Conditionally_enabled_hash() = delete;
    _Conditionally_enabled_hash(const _Conditionally_enabled_hash&) = delete;
    _Conditionally_enabled_hash(_Conditionally_enabled_hash&&) = delete;
    _Conditionally_enabled_hash& operator=(const _Conditionally_enabled_hash&) = delete;
    _Conditionally_enabled_hash& operator=(_Conditionally_enabled_hash&&) = delete;
};

// STRUCT TEMPLATE hash
template <class _Kty>
struct hash : _Conditionally_enabled_hash<_Kty, !is_const_v<_Kty> && (is_integral_v<_Kty> || is_pointer_v<_Kty>)> {
    // hash functor primary template (handles enums, integrals, and pointers)
    static size_t _Do_hash(const _Kty& _Keyval) noexcept {
        // // hash _Keyval to size_t value by pseudorandomizing transform
        // return Fnv32::calcHash(_Keyval);
        return _Hash_representation(_Keyval);
    }
};

template <>
struct hash<float> {
    size_t operator()(const float _Keyval) noexcept {
        return _Hash_representation(_Keyval == 0.0F ? 0.0F : _Keyval);  // map -0 to 0
    }
};

template <>
struct hash<double> {
    size_t operator()(const double _Keyval) noexcept {
        return _Hash_representation(_Keyval == 0.0 ? 0.0 : _Keyval);  // map -0 to 0
    }
};

template <>
struct hash<long double> {
    size_t operator()(const long double _Keyval) noexcept {
        return _Hash_representation(_Keyval == 0.0L ? 0.0L : _Keyval);  // map -0 to 0
    }
};

template <>
struct hash<nullptr_t> {
    size_t operator()(nullptr_t) noexcept {
        void* _Null{};
        return _Hash_representation(_Null);
    }
};

}  // namespace rtl

#endif