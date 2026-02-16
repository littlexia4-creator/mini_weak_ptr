#pragma once

#include "_counter_base.hpp"

namespace xiaoyu {

    template <class _Resource, class _Dx, class _Alloc>
    class _Ref_count_resource_alloc : public _Ref_count_base {
        // handle reference counting for object with deleter and allocator
    public:
        _Ref_count_resource_alloc(_Resource _Px, _Dx _Dt, const _Alloc& _Ax)
            : _Ref_count_base(),
            _Mypair(_STD move(_Dt), { _Ax, _Px }) {}

#ifdef __EDG__ // TRANSITION, VSO-1292293
        ~_Ref_count_resource_alloc() noexcept override {} // TRANSITION, should be non-virtual
#else // ^^^ workaround / no workaround vvv
        ~_Ref_count_resource_alloc() noexcept override = default; // TRANSITION, should be non-virtual
#endif // ^^^ no workaround ^^^

    private:
#if _WIN32
        using _Myalty = std::_Rebind_alloc_t<_Alloc, _Ref_count_resource_alloc>;
#else
        using _Myalty = std::allocator<_Ref_count_resource_alloc>;
#endif

        void _Destroy() noexcept override { // destroy managed resource
            _Mypair.first(_Mypair.second.second);
        }

        void _Delete_this() noexcept override { // destroy self; == dele
            _Myalty _Al = _Mypair.second.first;
            this->~_Ref_count_resource_alloc();
#if _WIN32
            std::_Deallocate_plain(_Al, this);
#else
            _Al.deallocate(this, sizeof(*this));
#endif
        }

        std::pair<_Dx, std::pair<_Myalty, _Resource>> _Mypair;
    };

}