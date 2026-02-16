#pragma once
#include "core.h"

namespace xiaoyu {
#if _HAS_CXX20
    struct _For_overwrite_tag {
        explicit _For_overwrite_tag() = default;
    };
#endif // _HAS_CXX20

    template <class _Ty>
    class _Ref_count_obj2 : public _Ref_count_base { // handle reference counting for object in control block, no allocator
    public:
        template <class... _Types>
        explicit _Ref_count_obj2(_Types&&... _Args) : _Ref_count_base() {
#if _HAS_CXX20
            if constexpr (sizeof...(_Types) == 1 && (is_same_v<xiaoyu::_For_overwrite_tag, remove_cvref_t<_Types>> && ...)) {
                /* When using _For_overwrite_tag, default construct storage. */
#if _WIN32
                std::_Default_construct_in_place(_Storage_Value);
#else
                ::new (_Voidify_iter(_STD addressof(_Storage_Value))) _Ty();
#endif
            }
            else
#endif // _HAS_CXX20
            {
#if _WIN32
                std::_Construct_in_place(_Storage_Value, _STD forward<_Types>(_Args)...); // requires a suitable constructor
#else
                ::new (_Voidify_iter(_STD addressof(_Storage_Value))) _Ty(_STD forward<_Types>(_Args)...);
#endif
            }
        }

        ~_Ref_count_obj2() noexcept override { // TRANSITION, should be non-virtual
            // nothing to do, _Storage._Value was already destroyed in _Destroy

            // N4849 [class.dtor]/7:
            // "A defaulted destructor for a class X is defined as deleted if:
            // X is a union-like class that has a variant member with a non-trivial destructor"
        }
        union { /* default union constructor leaves members uninitialized */
            _Ty _Storage_Value;
        };

    private:
        void _Destroy() noexcept override { // destroy managed resource
#if _WIN32
            std::_Destroy_in_place(_Storage_Value);
#else
            // std::_Destroy(std::__addressof(_Storage_Value));
            std::destroy_at(std::addressof(_Storage_Value));
#endif
        }

        void _Delete_this() noexcept override { // destroy self
            delete this;
        }
    };
}