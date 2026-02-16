#include <iostream>
#include <thread>
#include <memory>
#ifndef _WIN32
#include <cxxabi.h>
#endif
#include "core.h"

using namespace std;

template<typename type>
void print_type() {
#ifdef __GNUC__   // linux  
    cout << abi::__cxa_demangle(typeid(type).name(), nullptr, nullptr, nullptr) << endl;
#else   // vc 
    cout << typeid(type).name() << endl;
#endif  
}

template<typename type>
void print_type(type) {
#ifdef __GNUC__   // linux  
    cout << abi::__cxa_demangle(typeid(type).name(), nullptr, nullptr, nullptr) << endl;
#else   // vc 
    cout << typeid(type).name() << endl;
#endif  
}

/*
conjunction_v is logical AND.
*/

// func is enabled if all Ts... have the same type as T
template<typename T, typename... Ts>
std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>
func(T, Ts...)
{
    std::cout << "all types in pack are T\n";
}

// otherwise
template<typename T, typename... Ts>
std::enable_if_t<!std::conjunction_v<std::is_same<T, Ts>...>>
func(T, Ts...)
{
    std::cout << "not all types in pack are T\n";
}

static int test1() {
    func(1, 2, 3);
    func(1, 2, "hello!");

    return 0;
}

static int test2() {
    print_type<std::enable_if_t<true, int>>();
    print_type<std::enable_if_t<true>>();// default return type is void
#if 0
    print_type<std::enable_if_t<false, int>>();// will not compile; type is undefined
#endif

    return 0;
}

/*
conditional_t<flag, value1, vlaue2>
If flag is true, pick value1; if false, pick value2.
*/
static int test3() {
    print_type<conditional_t<true, int, char*>>();
    print_type<conditional_t<false, int, char*>>();
    return 0;
}

/*

*/
template<bool T>
void convertible() {
    std::cout << "convertible\n";
}
template<>
void convertible<false>() {
    std::cout << "not convertible\n";
}

class A {
public:
    virtual ~A() = 0;
};
class C :public A {};

static int test4() {
    convertible<is_convertible_v<A, A>>();
    convertible<is_convertible_v<std::shared_ptr<A>, std::weak_ptr<A>>>();
    convertible<is_convertible_v<std::weak_ptr<A>, std::shared_ptr<A>>>();
    convertible<is_convertible_v<A*, C*>>();
    convertible<is_convertible_v<C*, A*>>(); // should be true
    return 0;
}

/* =0 guess: only used to test template type traits. */
template <class _Ux, class _Ty, enable_if_t<conjunction_v<xiaoyu::_SP_convertible<_Ux, _Ty>>, int> = 0>
void FOO() {
    cout << "FOO()\n";
}

static int test5() {
    typedef A B;
    FOO<int, int>();
    FOO<A, B, 1>();
    FOO<B, A, 1>(); // checks whether A* and B* can convert to each other
    return 0;
}

template <class _Ty, class _Ty2, class = const _Ty2*>
static constexpr bool _Must_avoid_expired_conversions_from = true;

template <class _Ty, class _Ty2>
static constexpr bool _Must_avoid_expired_conversions_from<_Ty, _Ty2, decltype(static_cast<const _Ty2*>(static_cast<_Ty*>(nullptr)))> = false;

static int test6() {
    // This trait intentionally checks whether _Ty* cannot be statically cast to const _Ty2*.
    cout << _Must_avoid_expired_conversions_from<A, C> << endl;
    auto tmp1 = static_cast<const C*>(static_cast<A*>(nullptr)); // success =0
    cout << _Must_avoid_expired_conversions_from<C, A> << endl;
    auto tmp2 = static_cast<const A*>(static_cast<C*>(nullptr)); // success =0
    cout << _Must_avoid_expired_conversions_from<int, int> << endl;
    auto tmp3 = static_cast<const int*>(static_cast<int*>(nullptr)); // success =0

    cout << _Must_avoid_expired_conversions_from<int, double> << endl;
    // auto tmp4=static_cast<const double*>(static_cast<int*>(nullptr)); // failure =1
    cout << _Must_avoid_expired_conversions_from<double, int> << endl;
    // auto tmp5=static_cast<const int*>(static_cast<double*>(nullptr)); // failure =1

    return 0;
}

static int test7() {

    typedef int _Yty;
    print_type<decltype(delete[] _STD declval<_Yty*>())>();

    return 0;
}

static int test8() {
    print_type<int(int)>();  // int __cdecl(int)
#if _HAS_CXX20
    std::cout << std::boolalpha
        << std::is_same_v<std::remove_cv_t<int>, int> << '\n'
        << std::is_same_v<std::remove_cv_t<int&>, int> << '\n'                      // false 
        << std::is_same_v<std::remove_cv_t<int&&>, int> << '\n'                     // false
        << std::is_same_v<std::remove_cv_t<const int&>, int> << '\n'                // false
        << std::is_same_v<std::remove_cv_t<const int[2]>, int[2]> << '\n'           // true
        << std::is_same_v<std::remove_cv_t<const int(&)[2]>, int[2]> << '\n'        // false
        << std::is_same_v<std::remove_cv_t<int(int)>, int(int)> << '\n'
        << '\n'
        << std::is_same_v<std::remove_reference_t<int>, int> << '\n'
        << std::is_same_v<std::remove_reference_t<int&>, int> << '\n'               // true
        << std::is_same_v<std::remove_reference_t<int&&>, int> << '\n'              // true
        << std::is_same_v<std::remove_reference_t<const int&>, int> << '\n'         // false
        << std::is_same_v<std::remove_reference_t<const int[2]>, int[2]> << '\n'    // false
        << std::is_same_v<std::remove_reference_t<const int(&)[2]>, int[2]> << '\n' // false
        << std::is_same_v<std::remove_reference_t<int(int)>, int(int)> << '\n'
        << '\n'
        << std::is_same_v<std::remove_cvref_t<int>, int> << '\n'
        << std::is_same_v<std::remove_cvref_t<int&>, int> << '\n'
        << std::is_same_v<std::remove_cvref_t<int&&>, int> << '\n'
        << std::is_same_v<std::remove_cvref_t<const int&>, int> << '\n'
        << std::is_same_v<std::remove_cvref_t<const int[2]>, int[2]> << '\n'
        << std::is_same_v<std::remove_cvref_t<const int(&)[2]>, int[2]> << '\n'
        << std::is_same_v<std::remove_cvref_t<int(int)>, int(int)> << '\n'
        ;
#endif // _HAS_CXX20

    return 0;
}

template <class _Ty>
struct M_Wrap {
    _Ty _Value; // workaround for VSO-586813 "T^ is not allowed in a union"
};

static int test9() {
    struct _Ty {
        _Ty(int, int);
        auto getData() -> int { return 1024; };
    };
    class OBJ {
    public:
        union {
            M_Wrap<_Ty> _Storage;
        };
    };

#if 0
    OBJ obj;/* compile fails; needs a default constructor */
#endif

    return 0;
}


static int test10() {
    std::cout << std::boolalpha
        << std::is_pointer_v<int> << '\n'
        << std::is_pointer_v<int*> << '\n'
        << std::is_pointer_v<const int*> << '\n'
        << std::is_pointer_v<int* const> << '\n'
        << std::is_pointer_v<const int* const> << '\n'
        << std::is_pointer_v<int* volatile> << '\n'
        ;
    return 0;
}

int test_all() {
    return test10();
}
