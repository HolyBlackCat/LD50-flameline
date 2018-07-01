#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "mat.h"
#include "dynamic_storage.h"
#include "strings.h"

#define main SDL_main


struct A
{
    char x[1000];
    A() {std::cout << "A()\n";}
    A(const A &) {std::cout << "A(const A &)\n";}
    A(A &&) {std::cout << "A(A &&)\n";}
    A &operator=(const A &) {std::cout << "A &operator=(const A &)\n"; return *this;}
    A &operator=(A &&) {std::cout << "A &operator=(A &&)\n"; return *this;}
    virtual ~A() {std::cout << "~A()\n";}
};

struct B : A
{
    char x[1000];
    B() {std::cout << "B()\n";}
    B(const B &) : A() {std::cout << "B(const B &)\n";}
    B(B &&) : A() {std::cout << "B(B &&)\n";}
    B &operator=(const B &) {std::cout << "B &operator=(const B &)\n"; return *this;}
    B &operator=(B &&) {std::cout << "B &operator=(B &&)\n"; return *this;}
    ~B() {std::cout << "~B()\n";}
};

template <typename B> struct MyFuncBase : DynamicStorage::FuncBase<B>
{
    using Base = MyFuncBase;
    virtual void foo(const B *ptr);
};

template <typename B, typename D> struct MyFunc : MyFuncBase<B>
{
    virtual void foo(const B *ptr)
    {
        std::cout << "SIZE: " << sizeof(*DynamicStorage::derived<D>(ptr)) << '\n';
    }
};

int main(int, char**)
{
    using S = DynStorage<A, MyFunc>;

    std::vector<S> v;
    v.push_back(S::make<A>());
    v.push_back(S::make<B>());

    for (const auto &it : v)
        it.functions().foo(it.get());

    //x = std::move(y);
    //x = z;
    //x = std::move(z);

    return 0;
}
