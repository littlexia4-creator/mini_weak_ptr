#pragma once

#include "core.h"
#include "xiaoyu_weak_ptr.hpp"

namespace xiaoyu {
    template <class _Ty>
    class enable_shared_from_this { // provide member functions that create shared_ptr to this
    public:
        using _Esft_type = enable_shared_from_this; // used by _Can_enable_shared helper for detection

        _NODISCARD shared_ptr<_Ty> shared_from_this() {
            return shared_ptr<_Ty>(_Wptr);
        }

        _NODISCARD shared_ptr<const _Ty> shared_from_this() const {
            return shared_ptr<const _Ty>(_Wptr);
        }

        _NODISCARD weak_ptr<_Ty> weak_from_this() noexcept {
            return _Wptr;
        }

        _NODISCARD weak_ptr<const _Ty> weak_from_this() const noexcept {
            return _Wptr;
        }

    protected:
        constexpr enable_shared_from_this() noexcept : _Wptr() {}

        enable_shared_from_this(const enable_shared_from_this&) noexcept = delete;
        // construct (must value-initialize _Wptr)

        enable_shared_from_this& operator=(const enable_shared_from_this&) noexcept = delete;
        // assign (must not change _Wptr)

        ~enable_shared_from_this() = default;

    private:
        template <class _Yty>
        friend class shared_ptr;

        mutable xiaoyu::weak_ptr<_Ty> _Wptr;
        // Why use mutable?
    };

}