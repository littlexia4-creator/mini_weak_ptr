#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <stddef.h>

#ifndef _WIN32

#ifndef _HAS_CXX17
#if __cplusplus > 201402L 
#define _HAS_CXX17 1
#else
#define _HAS_CXX17 0
#endif
#endif // _HAS_CXX17

#ifndef _HAS_CXX20
#if _HAS_CXX17 && __cplusplus > 201703L
#define _HAS_CXX20 1
#else
#define _HAS_CXX20 0
#endif
#endif // _HAS_CXX20

#define _STD       ::std::

// Functions that became constexpr in C++20
#if _HAS_CXX20
#define _CONSTEXPR20 constexpr
#else // ^^^ constexpr in C++20 and later / inline (not constexpr) in C++17 and earlier vvv
#define _CONSTEXPR20 inline
#endif // ^^^ inline (not constexpr) in C++17 and earlier ^^^

#if _HAS_NODISCARD
#define _NODISCARD [[nodiscard]]
#else // ^^^ CAN HAZ [[nodiscard]] / NO CAN HAZ [[nodiscard]] vvv
#define _NODISCARD
#endif // _HAS_NODISCARD

#endif // _WIN32

namespace xiaoyu {

    template <class _Yty, class = void>
    struct _Can_scalar_delete : std::false_type {};
    template <class _Yty>
    struct _Can_scalar_delete<_Yty, std::void_t<decltype(delete _STD declval<_Yty*>())>> : std::true_type {}; // returns void

    template <class _Yty, class = void>
    struct _Can_array_delete : std::false_type {};
    template <class _Yty>
    struct _Can_array_delete<_Yty, std::void_t<decltype(delete[] _STD declval<_Yty*>())>> : std::true_type {}; // returns void

    template <class _Yty, class _Ty>
    struct _SP_pointer_compatible : std::is_convertible<_Yty*, _Ty*>::type {
        // N4659 [util.smartptr.shared]/5 "a pointer type Y* is said to be compatible
        // with a pointer type T* " "when either Y* is convertible to T* ..."
    };
    template <class _Uty, size_t _Ext>
    struct _SP_pointer_compatible<_Uty[_Ext], _Uty[]> : std::true_type {
        // N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
    };
    template <class _Uty, size_t _Ext>
    struct _SP_pointer_compatible<_Uty[_Ext], const _Uty[]> : std::true_type {
        // N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
    };
    template <class _Uty, size_t _Ext>
    struct _SP_pointer_compatible<_Uty[_Ext], volatile _Uty[]> : std::true_type {
        // N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
    };
    template <class _Uty, size_t _Ext>
    struct _SP_pointer_compatible<_Uty[_Ext], const volatile _Uty[]> : std::true_type {
        // N4659 [util.smartptr.shared]/5 "... or Y is U[N] and T is cv U[]."
    };

    template <class _Yty, class _Ty>
    struct _SP_convertible : std::is_convertible<_Yty*, _Ty*>::type {};
    template <class _Yty, class _Uty>
    struct _SP_convertible<_Yty, _Uty[]> : std::is_convertible<_Yty(*)[], _Uty(*)[]>::type {};
    template <class _Yty, class _Uty, size_t _Ext>
    struct _SP_convertible<_Yty, _Uty[_Ext]> : std::is_convertible<_Yty(*)[_Ext], _Uty(*)[_Ext]>::type {};

    template <class _Fx, class _Arg, class = void>
    struct _Can_call_function_object : std::false_type {};
    template <class _Fx, class _Arg>
    struct _Can_call_function_object<_Fx, _Arg, std::void_t<decltype(_STD declval<_Fx>()(_STD declval<_Arg>()))>> : std::true_type {};

    template <class _Yty, class = void>
    struct _Can_enable_shared : std::false_type {}; // detect unambiguous and accessible inheritance from enable_shared_from_this

    template <class _Yty>
    struct _Can_enable_shared<_Yty, std::void_t<typename _Yty::_Esft_type>>
        : std::is_convertible<std::remove_cv_t<_Yty>*, typename _Yty::_Esft_type*>::type {
        // is_convertible is necessary to verify unambiguous inheritance
    };

#ifndef _WIN32
    template <class _Iter>
    _NODISCARD constexpr void* _Voidify_iter(_Iter _It) noexcept {
        // Either a pointer or an iterator.
        if constexpr (std::is_pointer_v<_Iter>) {
            return const_cast<void*>(static_cast<const volatile void*>(_It));
        }
        else {
            return const_cast<void*>(static_cast<const volatile void*>(_STD addressof(*_It)));
        }
    }
#endif
}

#ifndef _WIN32
typedef int __int32;
#endif

#define XIAOYU_DEBUG 1

#ifndef _WIN32
long _InterlockedCompareExchange(long volatile* _Destination, long _Exchange, long _Comparand);

__int32 __iso_volatile_load32(const volatile __int32*);

long _InterlockedIncrement(long volatile* _Addend);

long _InterlockedDecrement(long volatile* _Addend);

#define _MT_INCR(x) _InterlockedIncrement(reinterpret_cast<volatile long*>(&x))
#define _MT_DECR(x) _InterlockedDecrement(reinterpret_cast<volatile long*>(&x))
#endif
