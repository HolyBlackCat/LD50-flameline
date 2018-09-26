#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace DynamicStorage
{
    /* DynStorage is copyable wrapper for std::unique_ptr, with std::visit-like feature.
     *
     * When copied, copies the underlying object. The copy is done properly if `DynStorage<Base>` holds a derived class.
     *
     * You can't modify objects through `const DynStorage<T>` (unlike std::unique_ptr).
     *
     * How to construct:
     *     DynStorage<MyClass> x; // Creates null pointer.
     *     DynStorage<MyClass> y = DynStorage<MyClass>::make(params); // Creates an actual object. If the parameter list is not empty, `DynStorage<MyClass> y(params);` can be used as well.
     *     DynStorage<Base> z = DynStorage<Base>::make<Derived>(params); // Creates a derived object. If `Derived == Base`, does the same thing as the above line.
     *     // `auto` could be used here as well.
     *     // Conversion from `DynStorage<Derived>` to `DynStorage<Base>` is not supported.
     *     // Storing arrays is not supported.
     *
     * How to access contents:
     *     std::cout << bool(obj); // Checks if pointer is not null
     *     std::cout << *obj; // Obtains a reference.
     *     std::cout << obj.get(); // Obtains a pointer.
     *     std::cout << obj->foo; // Member access.
     *
     * How to do std::visit-like stuff:
     *     struct A {virtual ~A() {}};
     *     struct B : A {};
     *
     *     void foo(A) {std::cout << "foo(A)\n";}
     *     void foo(B) {std::cout << "foo(B)\n";}
     *
     *     int main()
     *     {
     *         auto x = DynStorage<A>::make(); // Makes an instance of A
     *         auto y = DynStorage<A>::make<B>(); // Makes an instance of B
     *         foo(*x); // Prints foo(A)
     *         foo(*y); // Prints foo(A), but we want foo(B)
     *     }
     *
     *     // Here is how we do that:
     *
     *     template <typename BaseT> struct MyFuncsBase : DynamicStorage::FuncBase<BaseT>
     *     {
     *         virtual void call_foo(BaseT *) = 0;
     *         using Base = MyFuncsBase;
     *     };
     *
     *     template <typename BaseT, typename DerivedT> struct MyFuncs : MyFuncsBase<BaseT>
     *     {
     *         void call_foo(BaseT *base) override
     *         {
     *             foo(*DynamicStorage::derived<DerivedT>(base));
     *         }
     *     };
     *
     *     int main()
     *     {
     *         auto x = DynStorage<A,MyFuncs>::make();
     *         auto y = DynStorage<A,MyFuncs>::make<B>();
     *
     *         x.functions().call_foo(x.get()); // prints foo(A)
     *         y.functions().call_foo(y.get()); // prints foo(B)
     *     }
     */

    namespace impl
    {
        // Type trait to check if A is static_cast'able to B.
        template <typename A, typename B, typename = void> struct can_static_cast_impl : std::false_type {};
        template <typename A, typename B> struct can_static_cast_impl<A, B, std::void_t<decltype(static_cast<B>(std::declval<A>()))>> : std::true_type {};
        template <typename A, typename B> inline constexpr bool can_static_cast_v = can_static_cast_impl<A,B>::value;

        // Type trait to check if A is dynamic_cast'able to B.
        template <typename A, typename B, typename = void> struct can_dynamic_cast_impl : std::false_type {};
        template <typename A, typename B> struct can_dynamic_cast_impl<A, B, std::void_t<decltype(dynamic_cast<B>(std::declval<A>()))>> : std::true_type {};
        template <typename A, typename B> inline constexpr bool can_dynamic_cast_v = can_dynamic_cast_impl<A,B>::value;

        template <typename A, typename B> inline constexpr bool can_static_or_dynamic_cast_v = can_static_cast_v<A,B> || can_dynamic_cast_v<A,B>;
    }

    // Downcasts a pointer. Attempts to use static_cast, falls back to dynamic_cast. If none of them work, fails with a static_assert.
    template <typename Derived, typename Base> Derived *derived(Base *ptr)
    {
        static_assert(impl::can_static_or_dynamic_cast_v<Base*, Derived*>, "Unable to downcast.");
        if constexpr (impl::can_static_cast_v<Base*, Derived*>)
            return static_cast<Derived *>(ptr); // This doesn't work if base is virtual.
        else
            return dynamic_cast<Derived *>(ptr);
    }

    template <typename B> struct FuncBase
    {
        using Base = FuncBase;
        virtual std::unique_ptr<B> copy_(const B *) = 0;
    };

    template <typename B, typename D> struct DefaultFuncImpl : FuncBase<B> {};

    template
    <
        typename T,
        template <typename,typename> typename Functions = DefaultFuncImpl
    >
    class DynStorage
    {
        static_assert(!std::is_const_v<T>, "Template parameter can't be const.");
        static_assert(!std::is_array_v<T>, "Template parameter can't be an array.");

        template <typename D> struct Implementation : Functions<T,D>
        {
            std::unique_ptr<T> copy_(const T *ptr) override
            {
                if constexpr (!std::is_abstract_v<D>)
                {
                    return ptr ? std::make_unique<D>(*derived<const D>(ptr)) : std::unique_ptr<T>();
                }
                else
                {
                    (void)ptr;
                    return std::unique_ptr<T>();
                }
            }

            inline static Implementation object;
        };

        using Pointer = std::unique_ptr<T>;
        using FunctionsBase = typename Implementation<T>::Base;

        FunctionsBase *funcs = &Implementation<T>::object;
        Pointer ptr;

      public:
        DynStorage() noexcept {}

        template <typename ...P, typename = std::void_t<decltype(T(std::declval<P>()...))>> // Disable if T is not constructible from passed parameters.
        DynStorage(P &&... p) : ptr(std::make_unique<T>(std::forward<P>(p)...)) {}

        DynStorage(const DynStorage &other) : funcs(other.funcs), ptr(funcs->copy_(other.ptr.get())) {}
        DynStorage(DynStorage &&other) noexcept : funcs(other.funcs), ptr(std::move(other.ptr)) {}

        DynStorage &operator=(const DynStorage &other) {ptr = other.funcs->copy_(other.ptr.get()); funcs = other.funcs; return *this;}
        DynStorage &operator=(DynStorage &&other) noexcept {ptr = std::move(other.ptr); funcs = other.funcs; return *this;}

        template <typename D = T, typename ...P,
                  typename = std::void_t<decltype(D(std::declval<P>()...))>> // Disable if T is not constructible from passed parameters.
        [[nodiscard]] static DynStorage make(P &&... p)
        {
            static_assert(!std::is_const_v<D>, "Template parameter can't be const.");
            static_assert(!std::is_array_v<D>, "Template parameter can't be an array.");
            static_assert(std::is_same_v<D,T> || std::has_virtual_destructor_v<T>, "Base has to have a virtual destructor.");
            static_assert(std::is_convertible_v<D*, T*>, "Unable to upcast the pointers. Probably you have non-public bases or duplicate bases.");
            static_assert(impl::can_static_or_dynamic_cast_v<T*, D*>, "Unable to downcast the pointers.");
            DynStorage ret;
            ret.ptr = std::make_unique<D>(std::forward<P>(p)...);
            ret.funcs = &Implementation<D>::object;
            return ret;
        }

        [[nodiscard]] explicit operator bool() const {return bool(ptr);}

        [[nodiscard]]       T *get()       {return ptr.get();}
        [[nodiscard]] const T *get() const {return ptr.get();}

        [[nodiscard]]       T &operator*()       {return *ptr;}
        [[nodiscard]] const T &operator*() const {return *ptr;}

        [[nodiscard]]       T *operator->()       {return ptr.get();}
        [[nodiscard]] const T *operator->() const {return ptr.get();}

        [[nodiscard]] FunctionsBase &functions() const {return *funcs;}
    };

}

using DynamicStorage::DynStorage;
