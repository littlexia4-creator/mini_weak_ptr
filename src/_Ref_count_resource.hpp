#pragma once

namespace xiaoyu {

    template <class _Resource, class _Dx>
    class _Ref_count_resource : public _Ref_count_base { // handle reference counting for object with deleter
    public:
        _Ref_count_resource(_Resource _Px, _Dx _Dt)
            : _Ref_count_base(), _Mypair(_STD move(_Dt), _Px) {}

#ifdef __EDG__ // TRANSITION, VSO-1292293
        ~_Ref_count_resource() noexcept override {} // TRANSITION, should be non-virtual
#else // ^^^ workaround / no workaround vvv
        ~_Ref_count_resource() noexcept override = default; // TRANSITION, should be non-virtual
#endif // ^^^ no workaround ^^^

    private:
        void _Destroy() noexcept override { // destroy managed resource
            _Mypair.first(_Mypair.second);
        }

        void _Delete_this() noexcept override { // destroy self
            delete this;
        }

        std::pair<_Dx, _Resource> _Mypair;
    };

}