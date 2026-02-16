#pragma once
#include "Ptr_base.hpp"

namespace xiaoyu {

    template <class _Ty>
    class weak_ptr : public _Ptr_base<_Ty> { // class for pointer to reference counted resource
    public:
        // When a constructor is converting from weak_ptr<_Ty2> to weak_ptr<_Ty>,
        // the below type trait intentionally asks
        // whether it would be possible to static_cast from _Ty* to const _Ty2*;
        // the trait intentionally checks whether _Ty* can be statically cast to const _Ty2*.
        // see N4901 [expr.static.cast]/11.

        // Primary template, the value is used when the substitution fails.
        template <class _Ty2, class = const _Ty2*>
        static constexpr bool
            _Must_avoid_expired_conversions_from =
            true;

#ifdef _WIN32
        // Template specialization, the value is used when the substitution succeeds.
        template <class _Ty2>
        static constexpr bool
            _Must_avoid_expired_conversions_from<_Ty2, decltype(static_cast<const _Ty2*>(static_cast<_Ty*>(nullptr)))> =
            false;
#endif

        constexpr weak_ptr() noexcept {}

        weak_ptr(const weak_ptr& _Other) noexcept {
#if XIAOYU_DEBUG
            cout << "weak_ptr(const weak_ptr& _Other) no conversion" << "\n";
#endif
            this->_Weakly_construct_from(_Other); // same type, no conversion
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr(const shared_ptr<_Ty2>& _Other) noexcept {
#if XIAOYU_DEBUG
            cout << "weak_ptr(const shared_ptr<_Ty2>& _Other) _SP_pointer_compatible" << "\n";
#endif
            this->_Weakly_construct_from(_Other); // shared_ptr keeps resource alive during conversion
        }

        // Derived (_Ty2) to base (_Ty).
        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr(const weak_ptr<_Ty2>& _Other) noexcept {
#if XIAOYU_DEBUG
            cout << "weak_ptr(const weak_ptr<_Ty2>& _Other) _SP_pointer_compatible" << "\n";
#endif        
            constexpr bool _Avoid_expired_conversions = _Must_avoid_expired_conversions_from<_Ty2>;

#if _HAS_CXX17
            if constexpr
#else
            if
#endif    
                (_Avoid_expired_conversions) {
                // Why does this branch run when implicit conversion is not allowed?
                assert(false); // TODO: investigate
                this->_Weakly_convert_lvalue_avoiding_expired_conversions(_Other);
            }
            else {
                this->_Weakly_construct_from(_Other);
            }
        }

        weak_ptr(weak_ptr&& _Other) noexcept {
            this->_Move_construct_from(_STD move(_Other));
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr(weak_ptr<_Ty2>&& _Other) noexcept {
            constexpr bool _Avoid_expired_conversions = _Must_avoid_expired_conversions_from<_Ty2>;

#if _HAS_CXX17
            if constexpr
#else
            if
#endif  
                (_Avoid_expired_conversions) {
                this->_Weakly_convert_rvalue_avoiding_expired_conversions(_STD move(_Other));
            }
            else {
                this->_Move_construct_from(_STD move(_Other));
            }
        }

        ~weak_ptr() noexcept {
            this->_Decwref();
        }

        weak_ptr& operator=(const weak_ptr& _Right) noexcept {
            weak_ptr(_Right).swap(*this);
            return *this;
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr& operator=(const weak_ptr<_Ty2>& _Right) noexcept {
            weak_ptr(_Right).swap(*this);
            return *this;
        }

        weak_ptr& operator=(weak_ptr&& _Right) noexcept {
            weak_ptr(_STD move(_Right)).swap(*this);
            return *this;
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr& operator=(weak_ptr<_Ty2>&& _Right) noexcept {
            weak_ptr(_STD move(_Right)).swap(*this);
            return *this;
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept {
            weak_ptr(_Right).swap(*this);
            return *this;
        }

        void reset() noexcept { // release resource, convert to null weak_ptr object
            weak_ptr().swap(*this);
        }

        void swap(weak_ptr& _Other) noexcept {
            this->_Swap(_Other);
        }

        _NODISCARD bool expired() const noexcept {
            return this->use_count() == 0;
        }

        _NODISCARD shared_ptr<_Ty> lock() const noexcept { // convert to shared_ptr
            shared_ptr<_Ty> _Ret;
            assert(_Ret._Construct_from_weak(*this));
            return _Ret;
        }
    };

#ifndef _WIN32
    template <class _Ty>
    template <class _Ty2>
    constexpr bool
        // Template specialization, the value is used when the substitution succeeds.
        weak_ptr<_Ty>::_Must_avoid_expired_conversions_from<_Ty2, decltype(static_cast<const _Ty2*>(static_cast<_Ty*>(nullptr)))> =
        false; /// _Ty* can convert to _Ty2* (same type or base/derived).
#endif

}
