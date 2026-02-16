#include "bad_weak_ptr.h"

namespace xiaoyu {

    bad_weak_ptr::bad_weak_ptr() noexcept {}

    _NODISCARD const char* bad_weak_ptr::what() const noexcept {
        // return pointer to message string
        return "bad_weak_ptr";
    }

}