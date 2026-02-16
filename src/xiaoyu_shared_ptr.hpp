#pragma once
#include "xiaoyu_weak_ptr.hpp"
#include "_counter_impl.hpp"
#include "_Ref_count_resource.hpp"
#include "_Ref_count_resource_alloc.hpp"
#include "bad_weak_ptr.h"
#include "_Ref_count_obj2.hpp"
#include <exception>
#include <type_traits>

namespace xiaoyu {

#define _THROW(x) throw x

    _NODISCARD inline static void _Throw_bad_weak_ptr() {
        _THROW(xiaoyu::bad_weak_ptr{});
    }

    template <class _Ty>
    constexpr _Ty* _Unfancy(_Ty* _Ptr) noexcept { // do nothing for plain pointers
        return _Ptr;
    }

    template <class _Ty>
    class shared_ptr : public _Ptr_base<_Ty> { // class for reference counted resource management
    private:
        using _Mybase = _Ptr_base<_Ty>;

    public:
        using typename _Mybase::element_type;

        constexpr shared_ptr() noexcept {
#if XIAOYU_DEBUG
            cout << "default shared_ptr()" << "\n";
#endif
        }

        constexpr shared_ptr(nullptr_t) noexcept {} // construct empty shared_ptr

        template <
            class _Ux, enable_if_t<
            conjunction_v<
            conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
            _SP_convertible<_Ux, _Ty>
            >, int
            > = 0
        >
        explicit shared_ptr(_Ux* _Px) { // construct shared_ptr object that owns _Px
#if _HAS_CXX17
            if constexpr
#else
            if
#endif  
                (is_array_v<_Ty>) {
#if XIAOYU_DEBUG
                cout << "shared_ptr(_Ux* _Px) array_v" << "\n";
#endif
                _Setpd(_Px, default_delete<_Ux[]>{});
            }
            else {
#if XIAOYU_DEBUG
                cout << "shared_ptr(_Ux* _Px)" << "\n";
#endif
                _Set_ptr_rep_and_enable_shared(_Px, new _Ref_count<_Ux>(_Px));
            }
        }

        template <class _Ux, class _Dx,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
            _SP_convertible<_Ux, _Ty>>,
            int> = 0>
            shared_ptr(_Ux* _Px, _Dx _Dt) { // construct with _Px, deleter
#if XIAOYU_DEBUG
            cout << "shared_ptr(_Ux* _Px, _Dx _Dt)" << "\n";
#endif
            _Setpd(_Px, _STD move(_Dt));
        }

        template <class _Ux, class _Dx, class _Alloc,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
            _SP_convertible<_Ux, _Ty>>,
            int> = 0>
            shared_ptr(_Ux* _Px, _Dx _Dt, _Alloc _Ax) { // construct with _Px, deleter, allocator
#if XIAOYU_DEBUG
            cout << "shared_ptr(_Ux* _Px, _Dx _Dt, _Alloc _Ax)" << "\n";
#endif
            _Setpda(_Px, _STD move(_Dt), _Ax);
        }

        template <class _Dx,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, nullptr_t&>>, int> = 0>
        shared_ptr(nullptr_t, _Dx _Dt) { // construct with nullptr, deleter
#if XIAOYU_DEBUG
            cout << "shared_ptr(nullptr_t, _Dx _Dt)" << "\n";
#endif
            _Setpd(nullptr, _STD move(_Dt));
        }

        template <class _Dx, class _Alloc,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, nullptr_t&>>, int> = 0>
        shared_ptr(nullptr_t, _Dx _Dt, _Alloc _Ax) { // construct with nullptr, deleter, allocator
            _Setpda(nullptr, _STD move(_Dt), _Ax);
        }

        template <class _Ty2>
        shared_ptr(const shared_ptr<_Ty2>& _Right, element_type* _Px) noexcept {
#if XIAOYU_DEBUG
            cout << "shared_ptr(copy construction with new pointer)" << "\n";
#endif
            // construct shared_ptr object that aliases _Right
            this->_Alias_construct_from(_Right, _Px);
        }

        template <class _Ty2>
        shared_ptr(shared_ptr<_Ty2>&& _Right, element_type* _Px) noexcept {
#if XIAOYU_DEBUG
            cout << "shared_ptr(moving construction with new pointer)" << "\n";
#endif
            // move construct shared_ptr object that aliases _Right
            this->_Alias_move_construct_from(_STD move(_Right), _Px);
        }

        shared_ptr(const shared_ptr& _Other) noexcept { // construct shared_ptr object that owns same resource as _Other
#if XIAOYU_DEBUG
            cout << "shared_ptr(copy construction)" << "\n";
#endif
            this->_Copy_construct_from(_Other);
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        shared_ptr(const shared_ptr<_Ty2>& _Other) noexcept {
            // construct shared_ptr object that owns same resource as _Other
            this->_Copy_construct_from(_Other);
        }

        shared_ptr(shared_ptr&& _Right) noexcept { // construct shared_ptr object that takes resource from _Right
#if XIAOYU_DEBUG
            cout << "shared_ptr(moving copy construction)" << "\n";
#endif
            this->_Move_construct_from(_STD move(_Right));
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        shared_ptr(shared_ptr<_Ty2>&& _Right) noexcept { // construct shared_ptr object that takes resource from _Right
            this->_Move_construct_from(_STD move(_Right));
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        explicit shared_ptr(const weak_ptr<_Ty2>& _Other) { // construct shared_ptr object that owns resource *_Other
#if XIAOYU_DEBUG
            cout << "shared_ptr(const weak_ptr<_Ty2>& _Other)" << "\n";
#endif
            if (!this->_Construct_from_weak(_Other)) {
                _Throw_bad_weak_ptr();
            }
        }

        ~shared_ptr() noexcept { // release resource
            this->_Decref();
        }

        shared_ptr& operator=(const shared_ptr& _Right) noexcept {
#if XIAOYU_DEBUG
            cout << "shared_ptr operator=" << "\n";
#endif
            shared_ptr(_Right).swap(*this);
            return *this;
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        shared_ptr& operator=(const shared_ptr<_Ty2>& _Right) noexcept {
#if XIAOYU_DEBUG
            cout << "shared_ptr(const shared_ptr<_Ty2>&)" << "\n";
#endif
            shared_ptr(_Right).swap(*this);
            return *this;
        }

        shared_ptr& operator=(shared_ptr&& _Right) noexcept { // take resource from _Right
#if XIAOYU_DEBUG
            cout << "shared_ptr moving operator=" << "\n";
#endif
            shared_ptr(_STD move(_Right)).swap(*this);
            return *this;
        }

        template <class _Ty2, enable_if_t<_SP_pointer_compatible<_Ty2, _Ty>::value, int> = 0>
        shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept { // take resource from _Right
#if XIAOYU_DEBUG
            cout << "shared_ptr(shared_ptr<_Ty2>&&)" << "\n";
#endif
            shared_ptr(_STD move(_Right)).swap(*this);
            return *this;
        }

        void swap(shared_ptr& _Other) noexcept {
            this->_Swap(_Other);
        }

        void reset() noexcept { // release resource and convert to empty shared_ptr object
            shared_ptr().swap(*this);
        }

        template <class _Ux,
            enable_if_t<conjunction_v<conditional_t<is_array_v<_Ty>, _Can_array_delete<_Ux>, _Can_scalar_delete<_Ux>>,
            _SP_convertible<_Ux, _Ty>>,
            int> = 0>
            void reset(_Ux* _Px) { // release, take ownership of _Px
            shared_ptr(_Px).swap(*this);
        }

        template <class _Ux, class _Dx,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
            _SP_convertible<_Ux, _Ty>>,
            int> = 0>
            void reset(_Ux* _Px, _Dx _Dt) { // release, take ownership of _Px, with deleter _Dt
            shared_ptr(_Px, _Dt).swap(*this);
        }

        template <class _Ux, class _Dx, class _Alloc,
            enable_if_t<conjunction_v<is_move_constructible<_Dx>, _Can_call_function_object<_Dx&, _Ux*&>,
            _SP_convertible<_Ux, _Ty>>,
            int> = 0>
            void reset(_Ux* _Px, _Dx _Dt, _Alloc _Ax) { // release, take ownership of _Px, with deleter _Dt, allocator _Ax
            shared_ptr(_Px, _Dt, _Ax).swap(*this);
        }

        using _Mybase::get;

        template <class _Ty2 = _Ty, enable_if_t<!disjunction_v<is_array<_Ty2>, is_void<_Ty2>>, int> = 0>
        _NODISCARD _Ty2& operator*() const noexcept {
            return *get();
        }

        template <class _Ty2 = _Ty, enable_if_t<!is_array_v<_Ty2>, int> = 0>
        _NODISCARD _Ty2* operator->() const noexcept {
            return get();
        }

        template <class _Ty2 = _Ty, class _Elem = element_type, enable_if_t<is_array_v<_Ty2>, int> = 0>
        _NODISCARD _Elem& operator[](ptrdiff_t _Idx) const noexcept /* strengthened */ {
            return get()[_Idx];
        }

        explicit operator bool() const noexcept {
            return get() != nullptr;
        }

    private:
        template <class _UxptrOrNullptr, class _Dx>
        void _Setpd(const _UxptrOrNullptr _Px, _Dx _Dt) { // take ownership of _Px, deleter _Dt
            _Set_ptr_rep_and_enable_shared(_Px, new _Ref_count_resource<_UxptrOrNullptr, _Dx>(_Px, _STD move(_Dt)));
        }

        template <class _Alloc>
        using _Alloc_ptr_t = typename allocator_traits<_Alloc>::pointer;

        template <class _UxptrOrNullptr, class _Dx, class _Alloc>
        void _Setpda(const _UxptrOrNullptr _Px, _Dx _Dt, _Alloc _Ax) { // take ownership of _Px, deleter _Dt, allocator _Ax
#if _WIN32
            using _Alref_alloc = std::_Rebind_alloc_t<_Alloc, xiaoyu::_Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>>;
            // _Rebind_alloc_t usage: e.g. converts allocator<int> to
            // allocator<xiaoyu::_Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>>.
#else
            using _Alref_alloc = std::allocator<xiaoyu::_Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>>;
#endif
            using pointer = _Alloc_ptr_t<_Alref_alloc>;

            _Alref_alloc _Alref(_Ax);
            pointer _Ptr = _Alref.allocate(1);

#if _WIN32
            std::_Construct_in_place(*_Ptr, _Px, _STD move(_Dt), _Ax);
#else
            ::new (_Voidify_iter(_STD addressof(*_Ptr))) xiaoyu::_Ref_count_resource_alloc<_UxptrOrNullptr, _Dx, _Alloc>(_Px, _STD move(_Dt), _Ax);
#endif
            _Set_ptr_rep_and_enable_shared(_Px, xiaoyu::_Unfancy(_Ptr)); /* used to obtain _Ref_count_base* */
            //_Ptr    = nullptr;
        }

        template <class _Ux>
        void _Set_ptr_rep_and_enable_shared(_Ux* const _Px, _Ref_count_base* const _Rx) noexcept { // take ownership of not nullptr
            this->_Ptr = _Px;
            this->_Rep = _Rx;

#if _HAS_CXX17
            if constexpr
#else
            if
#endif  
                /*
                Works with enable_shared_from_this.
                No is_array because array types are indexed, and no is_volatile because volatile types
                are not allowed to use shared_from_this.
                */

                (std::conjunction_v<std::negation<is_array<_Ty>>, std::negation<is_volatile<_Ux>>, xiaoyu::_Can_enable_shared<_Ux>>) {
                if (_Px and _Px->_Wptr.expired()) {
                    typedef std::remove_cv_t<_Ux> remove_cv_Ux;
                    _Px->_Wptr = shared_ptr<remove_cv_Ux>(*this, const_cast<remove_cv_Ux*>(_Px));
                }
            }
        }

        void _Set_ptr_rep_and_enable_shared(std::nullptr_t, _Ref_count_base* const _Rx) noexcept { // take ownership of nullptr
            this->_Ptr = nullptr;
            this->_Rep = _Rx;
        }

#if _HAS_CXX20
        template <class _Ty0, class... _Types>
        friend std::enable_if_t<!std::is_array_v<_Ty0>, shared_ptr<_Ty0>> make_shared(_Types&&... _Args);
#else
        template <class _Ty0, class... _Types>
        friend shared_ptr<_Ty0> make_shared(_Types&&... _Args);
#endif // !_HAS_CXX20

    };

    template <class _Ty, class... _Types>
#if _HAS_CXX20  /* C++20 forbids make_shared for arrays. */
    std::enable_if_t<!std::is_array_v<_Ty>, shared_ptr<_Ty>> // make a shared_ptr to non-array object
#else // _HAS_CXX20
    shared_ptr<_Ty>
#endif // _HAS_CXX20
        make_shared(_Types&&... _Args) {
        /* make_shared uses variadic templates and forward (perfect forwarding) to pass args unchanged. */
        const auto _Rx = new xiaoyu::_Ref_count_obj2<_Ty>(_STD forward<_Types>(_Args)...); /* _Ptr and _Rep2 are contiguous. */
        shared_ptr<_Ty> _Ret; /* Default construct + assign heap ptr + assign counter + build enable_shared_from_this. */
        _Ret._Set_ptr_rep_and_enable_shared(_STD addressof(_Rx->_Storage_Value), _Rx);
        return _Ret;

        /*
        std::addressof is a C++11 template for getting the address of an lvalue expression.
        std::shared_ptr constructors may allocate multiple times, while ::make_shared allocates once and is more efficient.
        Put simply: _Ref_count_base + _Storage_Value == _Ref_count_obj2
        The original form was: _Set_ptr_rep_and_enable_shared(_Px, new _Ref_count<_Ux>(_Px));
        */
    }

    template <class _Ty1, class _Ty2>
    _NODISCARD bool operator==(const shared_ptr<_Ty1>& _Left, const shared_ptr<_Ty2>& _Right) noexcept {
        return _Left.get() == _Right.get();
    }

}
