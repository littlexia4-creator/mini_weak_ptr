### 为何C++弱指针可以解决共享指针的循环引用问题

1. 根本原因：弱指针不增加引用计数，不参与所有权管理

2. 循环引用问题示例

``` c++
class B;

class A {
public:
    shared_ptr<B> b_ptr;
    ~A() { cout << "A destroyed" << endl; }
};

class B {
public:
    shared_ptr<A> a_ptr;  // 循环引用！
    ~B() { cout << "B destroyed" << endl; }
};

int main() {
    shared_ptr<A> a = make_shared<A>();
    shared_ptr<B> b = make_shared<B>();
    
    a->b_ptr = b;  // B引用计数=2
    b->a_ptr = a;  // A引用计数=2，循环引用！
    
    // a,b离开作用域，但引用计数只减为1，内存泄漏！
    // A和B都不会被销毁
    return 0;
}

3. 弱指针解决方案
class B;

class A {
public:
    shared_ptr<B> b_ptr;
    ~A() { cout << "A destroyed" << endl; }
};

class B {
public:
    weak_ptr<A> a_ptr;  // 使用weak_ptr打破循环
    ~B() { cout << "B destroyed" << endl; }
};

int main() {
    shared_ptr<A> a = make_shared<A>();
    shared_ptr<B> b = make_shared<B>();
    
    a->b_ptr = b;  // B引用计数=2
    b->a_ptr = a;  // A引用计数仍为1！weak_ptr不增加计数
    
    // a离开作用域：A引用计数=0，销毁A
    // A销毁时释放b_ptr：B引用计数=1
    // b离开作用域：B引用计数=0，销毁B
    return 0;
}
```