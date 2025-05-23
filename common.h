#ifndef _COMMON_H
#define _COMMON_H

//////////////////////////////////////////////////////////////////////////
//
// namespace rtl (adapted from MSVC C++17)
//
namespace rtl {

//////////////////////////////////////////////////////////////////////////
//
// integral_constant
//
template <class _Ty, _Ty _Val>
struct integral_constant {
    static constexpr _Ty value = _Val;

    using value_type = _Ty;
    using type = integral_constant;

    constexpr operator value_type() const noexcept {
        return value;
    }

    constexpr value_type operator()() const noexcept {
        return value;
    }
};

template <bool _Val>
using bool_constant = integral_constant<bool, _Val>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

//////////////////////////////////////////////////////////////////////////
//
// enable_if
//
template <bool _Test, class _Ty = void>
struct enable_if {};  // no member "type" when !_Test

template <class _Ty>
struct enable_if<true, _Ty> {  // type is _Ty for _Test
    using type = _Ty;
};

template <bool _Test, class _Ty = void>
using enable_if_t = typename enable_if<_Test, _Ty>::type;

//////////////////////////////////////////////////////////////////////////
//
// conditional
//
template <bool _Test, class _Ty1, class _Ty2>
struct conditional {  // Choose _Ty1 if _Test is true, and _Ty2 otherwise
    using type = _Ty1;
};

template <class _Ty1, class _Ty2>
struct conditional<false, _Ty1, _Ty2> {
    using type = _Ty2;
};

template <bool _Test, class _Ty1, class _Ty2>
using conditional_t = typename conditional<_Test, _Ty1, _Ty2>::type;

//////////////////////////////////////////////////////////////////////////
//
// is_same_v
//
template <class, class>
constexpr bool is_same_v = false;  // determine whether arguments are the same type

template <class _Ty>
constexpr bool is_same_v<_Ty, _Ty> = true;

template <class _Ty1, class _Ty2>
struct is_same : bool_constant<is_same_v<_Ty1, _Ty2>> {};

//////////////////////////////////////////////////////////////////////////
//
// remove_cv
//
template <class _Ty>
struct remove_cv {  // remove top-level const and volatile qualifiers
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = _Fn<_Ty>;  // apply cv-qualifiers from the class template argument to _Fn<_Ty>
};

template <class _Ty>
struct remove_cv<const _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = const _Fn<_Ty>;
};

template <class _Ty>
struct remove_cv<volatile _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = volatile _Fn<_Ty>;
};

template <class _Ty>
struct remove_cv<const volatile _Ty> {
    using type = _Ty;

    template <template <class> class _Fn>
    using _Apply = const volatile _Fn<_Ty>;
};

template <class _Ty>
using remove_cv_t = typename remove_cv<_Ty>::type;

//////////////////////////////////////////////////////////////////////////
//
// disjunction_v
//
template <bool _First_value, class _First, class... _Rest>
struct _Disjunction {  // handle true trait or last trait
    using type = _First;
};

template <class _False, class _Next, class... _Rest>
struct _Disjunction<false, _False, _Next, _Rest...> {  // first trait is false, try the next trait
    using type = typename _Disjunction<_Next::value, _Next, _Rest...>::type;
};

template <class... _Traits>
struct disjunction : false_type {};  // If _Traits is empty, false_type

template <class _First, class... _Rest>
struct disjunction<_First, _Rest...> : _Disjunction<_First::value, _First, _Rest...>::type {
    // the first true trait in _Traits, or the last trait if none are true
};

template <class... _Traits>
constexpr bool disjunction_v = disjunction<_Traits...>::value;

//////////////////////////////////////////////////////////////////////////
//
// remove_reference
//
template <class _Ty, class... _Types>
constexpr bool _Is_any_of_v =  // true if and only if _Ty is in _Types
    disjunction_v<is_same<_Ty, _Types>...>;

//////////////////////////////////////////////////////////////////////////
//
// is_integral_v
//
template <class _Ty>
constexpr bool is_integral_v = _Is_any_of_v<remove_cv_t<_Ty>, bool, char, signed char, unsigned char, wchar_t,
                                            char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

//////////////////////////////////////////////////////////////////////////
//
// remove_reference
//
template <class _Ty>
struct remove_reference {
    using type = _Ty;
    using _Const_thru_ref_type = const _Ty;
};

template <class _Ty>
struct remove_reference<_Ty&> {
    using type = _Ty;
    using _Const_thru_ref_type = const _Ty&;
};

template <class _Ty>
struct remove_reference<_Ty&&> {
    using type = _Ty;
    using _Const_thru_ref_type = const _Ty&&;
};

template <class _Ty>
using remove_reference_t = typename remove_reference<_Ty>::type;

}  // namespace rtl

#endif