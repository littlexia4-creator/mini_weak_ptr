#pragma once

#include <iostream>
using namespace std;
#include <cassert>

#include "_counter_base.hpp"

namespace xiaoyu {

    template <class _Ty>
    class shared_ptr;

    template <class _Ty>
    class weak_ptr;

    template <class _Ty>
    class _Ptr_base { // base class for shared_ptr and weak_ptr
        /**
         * Creating a strong pointer uses _Incref or _Incref_nz.
         * Creating a weak pointer uses _Incwref.
         */
    public:
        using element_type = remove_extent_t<_Ty>; // remove array extent

        _NODISCARD long use_count() const noexcept {
            return _Rep ? _Rep->_Use_count() : 0;
        }

        _Ptr_base(const _Ptr_base&) = delete;
        _Ptr_base(_Ptr_base&&) = delete;
        _Ptr_base& operator=(const _Ptr_base&) = delete;
        _Ptr_base& operator=(_Ptr_base&&) = delete;

    protected:
        _NODISCARD element_type* get() const noexcept {
            return _Ptr;
        }

        constexpr _Ptr_base() noexcept = default;

        ~_Ptr_base() = default;

        template <class _Ty2>
        void _Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept {
            // implement shared_ptr's (converting) move ctor and weak_ptr's move ctor
            _Ptr = _Right._Ptr;
            _Rep = _Right._Rep;

            _Right._Ptr = nullptr;
            _Right._Rep = nullptr;
        }

        template <class _Ty2>
        void _Copy_construct_from(const shared_ptr<_Ty2>& _Other) noexcept {
            // implement shared_ptr's (converting) copy ctor
            _Alias_construct_from(_Other, _Other._Ptr);
        }

        template <class _Ty2>
        void _Alias_move_construct_from(shared_ptr<_Ty2>&& _Other, element_type* _Px) noexcept {
            // implement shared_ptr's aliasing move ctor
            _Ptr = _Px;
            _Rep = _Other._Rep;

            _Other._Ptr = nullptr;
            _Other._Rep = nullptr;
        }

        template <class _Ty2>
        void _Alias_construct_from(const shared_ptr<_Ty2>& _Other, element_type* _Px) noexcept {
            // implement shared_ptr's aliasing ctor
            _Other._Incref();

            _Ptr = _Px;
            _Rep = _Other._Rep;
        }

        template <class _Ty0>
        friend class weak_ptr; // specifically, weak_ptr::lock()

        template <class _Ty2>
        bool _Construct_from_weak(const weak_ptr<_Ty2>& _Other) noexcept {
            // implement shared_ptr's ctor from weak_ptr, and weak_ptr::lock()
            // called when converting weak to strong
            assert(_Other._Rep);
            if (_Other._Rep && _Other._Rep->_Incref_nz()) {
                _Ptr = _Other._Ptr;
                _Rep = _Other._Rep;
                return true;
            }

            return false;
        }

        void _Incref() const noexcept {
            if (_Rep) {
                _Rep->_Incref();
            }
        }

        void _Decref() noexcept { // decrement reference count
            if (_Rep) {
                _Rep->_Decref();
            }
        }

        void _Swap(_Ptr_base& _Right) noexcept { // swap pointers
            _STD swap(_Ptr, _Right._Ptr);
            _STD swap(_Rep, _Right._Rep);
        }

        template <class _Ty2>
        void _Weakly_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept { // implement weak_ptr's ctors
            if (_Other._Rep) {
                _Rep = _Other._Rep;
                _Rep->_Incwref();
                _Ptr = _Other._Ptr;
            }
        }

        template <class _Ty2>
        void _Weakly_convert_lvalue_avoiding_expired_conversions(const _Ptr_base<_Ty2>& _Other) noexcept {
            // implement weak_ptr's copy converting ctor
            if (_Other._Rep) {
                _Rep = _Other._Rep; // always share ownership
                _Rep->_Incwref();

                if (_Rep->_Incref_nz()) {
                    // Why can pointers of different types be implicitly converted?
                    _Ptr = _Other._Ptr; // keep resource alive during conversion, handling virtual inheritance
                    _Rep->_Decref();
                }
            }
        }

        template <class _Ty2>
        void _Weakly_convert_rvalue_avoiding_expired_conversions(_Ptr_base<_Ty2>&& _Other) noexcept {
            // implement weak_ptr's move converting ctor
            _Rep = _Other._Rep; // always transfer ownership

            /*
            To ensure assigning _Ptr succeeds, first confirm the resource is alive.
            We do this by calling _Incref_nz (increment use count if nonzero) before assignment,
            then decrement after assignment to restore the original count.
            Because the resource is freed when use reaches 0, keeping it nonzero prevents release.
            This matters for virtual inheritance: assigning a derived pointer to a base pointer may
            require vtable adjustment, which must not be interrupted for thread safety.
            */
            if (_Rep) {
                if (_Rep->_Incref_nz()) {
                    _Ptr = _Other._Ptr; // keep resource alive during conversion, handling virtual inheritance
                    _Rep->_Decref();
                }
            }

            _Other._Rep = nullptr;
            _Other._Ptr = nullptr;
        }

        void _Decwref() noexcept { // decrement weak reference count
            /* called when weak_ptr is destroyed */
            if (_Rep) {
                _Rep->_Decwref();
            }
        }

    private:
        element_type* _Ptr{ nullptr };
        _Ref_count_base* _Rep{ nullptr };

        template <class _Ty0>
        friend class _Ptr_base;

        friend shared_ptr<_Ty>;
    };


}
