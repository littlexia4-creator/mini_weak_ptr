#pragma once

#include <cassert>
#include "core.h"

namespace xiaoyu {

    class  _Ref_count_base { // common code for reference counting

        /* TRANSITION, VSO-1592329 */
        using _Atomic_counter_t = unsigned long;

    private:
        virtual void _Destroy() noexcept = 0; // destroy managed resource
        virtual void _Delete_this() noexcept = 0; // destroy self

        _Atomic_counter_t _Uses = 1; // strong pointer count, used to manage the resource
        _Atomic_counter_t _Weaks = 1; // weak count plus whether a strong pointer exists (1 yes, 0 no), used to manage self

    protected:
        constexpr _Ref_count_base() noexcept = default; // non-atomic initializations

    public:
        _Ref_count_base(const _Ref_count_base&) = delete;
        _Ref_count_base(_Ref_count_base&&) = delete;
        _Ref_count_base& operator=(const _Ref_count_base&) = delete;
        _Ref_count_base& operator=(_Ref_count_base&&) = delete;

        virtual ~_Ref_count_base() noexcept {} // TRANSITION, should be non-virtual

        bool _Incref_nz() noexcept { // increment use count if not zero, return true if successful
            /// Lock-free CAS increment.
            auto& _Volatile_uses = reinterpret_cast<volatile long&>(_Uses);
#ifdef _M_CEE_PURE
            long _Count = *_Atomic_address_as<const long>(&_Volatile_uses);
#else
#ifndef _WIN32
            long _Count = _Volatile_uses;
#else
            long _Count = __iso_volatile_load32(reinterpret_cast<volatile int*>(&_Volatile_uses));
#endif
#endif
            while (_Count != 0) {
                const long _Old_value = _InterlockedCompareExchange(&_Volatile_uses, _Count + 1, _Count);
                if (_Old_value == _Count) {
                    return true;
                }

                _Count = _Old_value;
            }

            return false;
        }

        void _Incref() noexcept { // increment use count
            _MT_INCR(this->_Uses);
        }

        void _Incwref() noexcept { // increment weak reference count
            _MT_INCR(this->_Weaks);
        }

        void _Decwref() noexcept { // decrement weak reference count
            if (_MT_DECR(this->_Weaks) == 0) {
                assert(_Uses == 0);
                _Delete_this(); // delete this; deleting the base also deletes the derived
            }
        }

        void _Decref() noexcept { // decrement use count
            if (_MT_DECR(this->_Uses) == 0) {
                _Destroy(); // destroy resource; separate resource management from pointer management
                /*
                Why do weak and use become linked when use == 0? The ref-count object (_Rep) starts with
                weak == 1 and use == 1. If only shared_ptr participates, weak would stay at 1 forever.
                When use reaches 0, we need to reclaim that weak count as well.
                */
                _Decwref();
            }
        }

        long _Use_count() const noexcept {
            return static_cast<long>(this->_Uses);
        }
    };
}
