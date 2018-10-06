#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace Dynamic
{
    /* DynStorage is copyable wrapper for std::unique_ptr, with std::visit-like feature.
     *
     * When copied, it copies the underlying object. The copy is done properly even if `DynStorage<Base>` holds a derived class.
     *
     * You can't modify objects through `const DynStorage<T>` (unlike std::unique_ptr).
     *
     * How to construct:
     *     DynStorage<MyClass> x = nullptr; // Creates null pointer.
     *     DynStorage<MyClass> y(...); // Creates an object.
     *     DynStorage<MyBase> z = DynStorage<MyBase>::make<MyDerived>(...); // Creates an object of a derived type. If `MyBase == MyDerived`, it has the same effect as the line above.
     *     // `auto` can be used in the last case.
     *     // Conversion from `DynStorage<Derived>` to `DynStorage<Base>` is not supported.
     *     // Storing arrays is not supported.
     *
     * How to access contents:
     *     std::cout << bool(obj); // Checks if pointer is not null
     *     std::cout << *obj; // Obtains a reference.
     *     std::cout << obj.get(); // Obtains a pointer.
     *     std::cout << obj->foo; // Member access.
     *     std::cout << obj.bytes(); // Obtains a void pointer to the actual stored class, and not a base class (unlike `get`, `*`, and `->`, which always access the base class).
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
     *     template <typename BaseT> struct MyFuncsBase : Dynamic::FuncBase<BaseT>
     *     {
     *         using Base = MyFuncsBase;
     *         virtual void call_foo(Dynamic::Param<BaseT>) = 0; // Template parameter of `Param` can be `const`.
     *     };
     *
     *     template <typename BaseT, typename DerivedT> struct MyFuncs : MyFuncsBase<BaseT>
     *     {
     *         void call_foo(Dynamic::Param<BaseT> obj) override
     *         {
     *             foo(*obj.template get<DerivedT>());
     *         }
     *     };
     *
     *     int main()
     *     {
     *         auto x = DynStorage<A,MyFuncs>::make();
     *         auto y = DynStorage<A,MyFuncs>::make<B>();
     *
     *         x.funcs().call_foo(x); // prints foo(A)
     *         y.funcs().call_foo(y); // prints foo(B)
     *     }
     */

    namespace impl
    {
        template <bool C> struct maybe_copyable {};
        template <> struct maybe_copyable<0>
        {
            maybe_copyable() = default;
            ~maybe_copyable() = default;
            maybe_copyable(const maybe_copyable &) = delete;
            maybe_copyable &operator=(const maybe_copyable &) = delete;
            maybe_copyable(maybe_copyable &&) = default;
            maybe_copyable &operator=(maybe_copyable &&) = default;
        };

        template <typename T> struct PointerPair
        {
            std::conditional_t<std::is_const_v<T>, const T, T> *direct = 0;
            std::unique_ptr<T> unique;

            PointerPair() {}
            template <typename D> PointerPair(std::unique_ptr<D> &&other) : direct(other.get()), unique(std::move(other)) {}
        };
    }

    template <typename T, template <typename,typename> typename Funcs> class Storage;

    template <typename B> class Param
    {
        template <typename T, template <typename,typename> typename Funcs> friend class Storage;

        static constexpr bool is_const = std::is_const_v<B>;
        template <typename T> using maybe_const = std::conditional_t<is_const, const T, T>;

        maybe_const<void> *ptr;

        Param(maybe_const<void> *ptr) : ptr(ptr) {}

      public:
        template <typename D> [[nodiscard]] maybe_const<D> *get() const
        {
            static_assert(std::is_base_of_v<B, D>, "Invalid parameter access.");
            static_assert(!std::is_const_v<D>, "`const` is not permitted here, it's added automatically if necessary.");
            return static_cast<maybe_const<D> *>(ptr);
        }
    };

    template <typename B> struct FuncBase
    {
        using Base = FuncBase;
        virtual impl::PointerPair<B> _copy(Param<const B> param) = 0;
    };

    template <typename B, typename D> struct DefaultFuncs : FuncBase<B> {};

    template
    <
        typename T,
        template <typename,typename> typename Funcs = DefaultFuncs
    >
    class Storage : impl::maybe_copyable<std::is_copy_constructible_v<T>>
    {
        static_assert(!std::is_const_v<T>, "Template parameter can't be const.");
        static_assert(!std::is_array_v<T>, "Template parameter can't be an array.");

        static constexpr bool is_copyable = std::is_copy_constructible_v<T>;

        template <typename D> struct Impl : Funcs<T,D>
        {
            impl::PointerPair<T> _copy(Param<const T> param) override
            {
                if constexpr (is_copyable)
                {
                    const D *other = param.template get<D>();
                    if (!other)
                        return {};
                    return {std::make_unique<D>(*other)};
                }
                else
                {
                    (void)param;
                    return {};
                }
            }

            inline static Impl obj;
        };

        using FuncBase = typename Impl<T>::Base;

        struct Data
        {
            impl::PointerPair<T> ptrs;
            FuncBase *funcs = 0;

            Data() {}
            Data(impl::PointerPair<T> &&ptrs, FuncBase *funcs) : ptrs(std::move(ptrs)), funcs(funcs) {}
            template <typename D> Data(std::unique_ptr<D> &&other) : ptrs(std::move(other)), funcs(&Impl<D>::obj) {}
        };
        Data data;

        Storage(Data &&data) : data(std::move(data)) {}

      public:
        Storage(decltype(nullptr)) noexcept {}

        template <typename ...P, typename = std::void_t<decltype(T(std::declval<P>()...))>> // Disable if T is not constructible from these parameters.
        Storage(P &&... p) : data(std::make_unique<T>(std::forward<P>(p)...)) {}

        Storage(const Storage &other) : data(other ? Data(other.funcs()._copy(other), other.data.funcs) : Data()) {}

        Storage(Storage &&other) noexcept : data(std::exchange(other.data, {})) {}

        Storage &operator=(Storage other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        template <typename D = T, typename ...P,
                  typename = std::void_t<decltype(D(std::declval<P>()...))>> // Disable if T is not constructible from these parameters.
        [[nodiscard]] static Storage make(P &&... p)
        {
            static_assert(!std::is_const_v<D>, "Template parameter can't be const.");
            static_assert(!std::is_array_v<D>, "Template parameter can't be an array.");
            static_assert(std::is_same_v<D,T> || std::has_virtual_destructor_v<T>, "Base has to have a virtual destructor.");
            static_assert(std::is_convertible_v<D*, T*>, "This type is not derived from base, or the base is not public, or the base is duplicated.");

            return Data(std::make_unique<D>(std::forward<P>(p)...));
        }

        [[nodiscard]] explicit operator bool() const {return bool(data.ptrs.direct);}

        [[nodiscard]]       T *get()       {return data.ptrs.unique.get();}
        [[nodiscard]] const T *get() const {return data.ptrs.unique.get();}

        [[nodiscard]]       T &operator*()       {return *data.ptrs.unique;}
        [[nodiscard]] const T &operator*() const {return *data.ptrs.unique;}

        [[nodiscard]]       T *operator->()       {return data.ptrs.unique.get();}
        [[nodiscard]] const T *operator->() const {return data.ptrs.unique.get();}

        // Unlike `get()`, this returns the actual pointer to the class even if multiple inheritance is present.
        [[nodiscard]]       void *bytes()       {return data.ptrs.direct;}
        [[nodiscard]] const void *bytes() const {return data.ptrs.direct;}

        [[nodiscard]] FuncBase &funcs() const {return *data.funcs;}

        operator Param<      T>()       {return {data.ptrs.direct};}
        operator Param<const T>() const {return {data.ptrs.direct};}
    };
}

template
<
    typename T,
    template <typename,typename> typename Funcs = Dynamic::DefaultFuncs
>
using DynStorage = Dynamic::Storage<T, Funcs>;
