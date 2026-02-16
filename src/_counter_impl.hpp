#pragma once

#include "_counter_base.hpp"

namespace xiaoyu {

    template <class _Ty>
    class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter
    public:
        explicit _Ref_count(_Ty* _Px) : _Ref_count_base(), _Ptr(_Px) {}

    private:
        void _Destroy() noexcept override { // destroy managed resource
            delete _Ptr;
        }

        void _Delete_this() noexcept override { // destroy self
            delete this;
        }

        _Ty* _Ptr;
    };

}