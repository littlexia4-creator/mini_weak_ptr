#include <iostream>
#include <thread>
#include <memory>
#include "xiaoyu_shared_ptr.hpp"
#include "enable_shared_from_this.hpp"
#include "test.h"

using namespace std;

static int test0() {
    long num = 1234;
    cout << _InterlockedIncrement(&num) << endl;
    cout << num << endl;
    cout << _InterlockedDecrement(&num) << endl;
    cout << num << endl;
    return 0;
}

class B;
class A
{
public:
    ~A() { cout << "A destruction \n"; }
    xiaoyu::shared_ptr<B>pointer;
};

class B {
public:
    ~B() { cout << "B destruction \n"; }
    xiaoyu::shared_ptr<A>pointer;
};

static int test1()
{
    xiaoyu::shared_ptr<A>pA(new A);
    xiaoyu::shared_ptr<B>pB(new B);
    pA->pointer = pB;
    pB->pointer = pA;

    return 0;
}

class D;
class C
{
public:
    ~C() { cout << "C destruction \n"; }
    xiaoyu::shared_ptr<D>pointer;
};

class D {
public:
    ~D() { cout << "D destruction \n"; }
    xiaoyu::weak_ptr<C>pointer;  // weak_ptr needs a shared_ptr to be created
};

static int test2()
{
    /* Solve strong-pointer cyclic reference issues. */
    xiaoyu::shared_ptr<C>pC(new C);
    xiaoyu::shared_ptr<D>pD(new D);
    pC->pointer = pD;
    pD->pointer = pC;

    return 0;
}

static int test3()
{
    typedef int T[10];
    xiaoyu::shared_ptr<T>p(new T);
    int* ux = new int;
    xiaoyu::shared_ptr<int>p2(ux, default_delete<int>(), std::allocator<int>());

    return 0;
}

static int test4() {
    struct Default
    {
        Default() = default;
        int foo() const { return 1; }
    };

    struct NonDefault
    {
        NonDefault() = delete;
        int foo() const { return 1; }
    };

    /// std::declval is not a language feature or keyword; it is a template function.
    decltype(Default().foo()) n1 = 1;                   // n1's type is int
    decltype(std::declval<NonDefault>().foo()) n2 = n1; // n2's type is int
    std::cout << "n1 = " << n1 << '\n'
        << "n2 = " << n2 << '\n';

    return 0;
}

struct TEST {
    //constexpr TEST()=default; 
    ~TEST() {
        cout << "TEST destruction \n";
    }
};

static int test5() {
#if 0
    auto p1 = new TEST; /* allocate heap memory */
    std::shared_ptr<TEST> sp1(p1); /* create smart pointer */
    std::shared_ptr<TEST> sp2(p1); /* create another smart pointer; triggers assert false */
#endif

    /*
    The standard recommends using make_shared instead of manual new,
    which prevents creating multiple ref-count control blocks from raw pointers.
    make_shared needs a default constructor or a matching initializer list constructor.
    */
    auto sp3 = std::make_shared<TEST>();
    auto sp4 = sp3;

    auto sp5 = std::make_shared<std::string>(9, 'A');
    cout << *sp5 << endl;

    return 0;
}

static int test6() {
#if 0
    auto p1 = new TEST; /* allocate heap memory */
    xiaoyu::shared_ptr<TEST> sp1(p1); /* create smart pointer */
    xiaoyu::shared_ptr<TEST> sp2(p1); /* create another smart pointer; triggers assert false */
#endif

    auto sp3 = xiaoyu::make_shared<TEST>();
    auto sp4 = sp3;

    auto sp5 = xiaoyu::make_shared<std::string>(9, 'A');
    cout << *sp5 << endl;

    return 0;
}

static int test7() {
    class TestB
    {
    public:
        TestB()
        {
            cout << "TestB create" << endl;
        }
        ~TestB()
        {
            cout << "TestB destory" << endl;
        }

        shared_ptr<TestB> getSharedFromThis() {
            /* Essentially two shared_ptrs point to the same object, but counters are not shared, causing double delete. */
            return  shared_ptr<TestB>(this);
        }
    };

    {
        std::shared_ptr<TestB> ptr3 = std::make_shared<TestB>();
        std::shared_ptr<TestB> ptr4 = ptr3->getSharedFromThis();
        cout << (ptr3 == ptr4) << endl;
    }

    return 0;
}

static int test8() {
#define NAMESPACE xiaoyu
    class TestB :public NAMESPACE::enable_shared_from_this<TestB>
    {
    public:
        TestB()
        {
            cout << "TestB create" << endl;
        }
        ~TestB()
        {
            cout << "TestB destory" << endl;
        }

        NAMESPACE::shared_ptr<TestB> getSharedFromThis() {
            /* Essentially two shared_ptrs point to the same object, but counters are not shared, causing double delete. */
            return this->shared_from_this();
        }
    };

#if 0
    typedef volatile TestB _Ty; // volatile types cannot use shared_from_this (no volatile smart ptr support).
#endif

#if 1
    typedef TestB _Ty;
#endif

    {
        NAMESPACE::shared_ptr<_Ty> ptr3 = NAMESPACE::make_shared<_Ty>();
        NAMESPACE::shared_ptr<_Ty> ptr4 = ptr3->getSharedFromThis();
        cout << (ptr3 == ptr4) << endl;
    }
#undef NAMESPACE
    return 0;
}

#ifndef _WIN32
/// @brief 
/// @param [in, out] _Addend A pointer to the variable to be incremented.
/// @return The function returns the resulting incremented value.
long _InterlockedIncrement(long volatile* _Addend) {
    /// InterLockedIncrement keeps access atomic across threads.
    return __sync_add_and_fetch(_Addend, 1L);
}

long _InterlockedDecrement(long volatile* _Addend) {
    return __sync_sub_and_fetch(_Addend, 1L);
}

// Compares the destination with the comparand; if equal, swaps with exchange and returns the old value.
long _InterlockedCompareExchange(long volatile* _Destination, long _Exchange, long _Comparand) {
    return __sync_val_compare_and_swap(_Destination, _Exchange, _Comparand);
}
#endif

int main() {
    /// g++ *.cxx -std=gnu++20

    cout << "has cxx17:" << std::boolalpha << _HAS_CXX17 << "\n";
    cout << "has cxx20:" << std::boolalpha << _HAS_CXX20 << "\n";

#if 1
    int ans = test2();
#else
    int ans = test_all();
#endif

#ifdef _WIN32
    system("pause");
#endif

    return ans;
}
