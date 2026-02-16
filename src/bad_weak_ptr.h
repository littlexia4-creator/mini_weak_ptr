#pragma once
#include "core.h"
#include <exception>

namespace xiaoyu {

    class bad_weak_ptr : public std::exception { // exception type for invalid use of expired weak_ptr object
    public:
        bad_weak_ptr() noexcept;
        _NODISCARD virtual const char* what() const noexcept override;
    };

}