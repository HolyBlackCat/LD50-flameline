#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "utils/finally.h"

namespace Dynamic
{
    /* Dyn (aka Dynamic::Storage) is copyable wrapper for std::unique_ptr, with std::visit-like features.
     *
     * When copied, it copies the underlying object. The copy is done properly even if `Dyn<Base>` holds a derived class.
     *
     * You can't modify objects through `const Dyn<T>` (unlike std::unique_ptr).
     *
     * How to construct:
     *     Dyn<MyClass> x = nullptr; // Allocates nothing.
     *     Dyn<MyClass> y(...); // Creates an object using an appropriate constructor. `Dyn<MyClass> y;` is allowed and uses the default constructor of `MyClass`.
     *     Dyn<MyBase> z = Dyn<MyBase>::make<MyDerived>(...); // Creates an object of a possibly derived type. If `MyBase == MyDerived`, it has the same effect as the line above.
     *     // `auto` can be used in the last case.
     *     // Conversion from `Dyn<Derived>` to `Dyn<Base>` is not supported.
     *     // Storing arrays is not supported.
     *
     * How to access contents:
     *     bool(obj) // Checks if pointer is not null
     *     obj.get() // Obtain a pointer
     *     *obj // Obtain a reference
     *     obj->foo // Access a member
     *     obj.bytes() // Obtains a `unsigned char *` to the actual stored class, instead of it's base specified in the template parameter.
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
     *         auto x = Dyn<A>::make(); // Makes an instance of A
     *         auto y = Dyn<A>::make<B>(); // Makes an instance of B
     *
     *         foo(*x); // Prints foo(A)
     *         foo(*y); // Prints foo(A), but we want foo(B)
     *     }
     *
     *     // Here is how we do that:
     *
     *     template <typename BaseT> struct MyFuncs
     *     {
     *         void (*call_foo)(Dynamic::Param<BaseT>);
     *
     *         template <typename DerivedT> constexpr void _make()
     *         {
     *             call_foo = [](Dynamic::Param<BaseT> param)
     *             {
     *                 foo(param.template get<DerivedT>());
     *             };
     *         }
     *     };
     *
     *     int main()
     *     {
     *         auto x = Dyn<A,MyFuncs<A>>::make();
     *         auto y = Dyn<A,MyFuncs<A>>::make<B>();
     *
     *         x.funcs().call_foo(x); // prints foo(A)
     *         y.funcs().call_foo(y); // prints foo(B)
     *     }
     */

    namespace impl
    {
        template <bool C> struct copyable_if {};
        template <> struct copyable_if<0>
        {
            copyable_if() = default;
            ~copyable_if() = default;
            copyable_if(const copyable_if &) = delete;
            copyable_if &operator=(const copyable_if &) = delete;
            copyable_if(copyable_if &&) = default;
            copyable_if &operator=(copyable_if &&) = default;
        };
    }


    template <typename, typename> class Storage;

    template <typename T> class Param
    {
        static_assert(std::is_class_v<T>, "The template parameter has to be a class.");
        static_assert(!std::is_volatile_v<T>, "The template parameter can't be `volatile`.");

        template <typename, typename> friend class Storage;

        static constexpr bool is_const = std::is_const_v<T>;
        template <typename A> using maybe_const = std::conditional_t<is_const, const A, A>;

        maybe_const<T> *base;
        maybe_const<void> *bytes;

        Param(maybe_const<T> *base, maybe_const<void> *bytes) : base(base), bytes(bytes) {}

      public:

        [[nodiscard]] maybe_const<T> &get_base() const
        {
            return *base;
        }

        // The template parameter can't have
        // `D` have to be the exact type of derived class, obtained from the template parameter of `_make()` of your function set.
        // Alternatively, `D` can be equal to `T`, stripped of cv-qualifiers. In this case, it's equivalent to `get_base()`.
        // If `D` is not derived from `T`, a `static_assert` will be triggered.
        // If `D` is derived from `T` but doesn't satisfy the conditions above, the behavior is undefined.
        template <typename D> [[nodiscard]] maybe_const<D> &get() const
        {
            static_assert(!std::is_const_v<D> && !std::is_volatile_v<D>, "The template parameter can't have cv-qualifiers. They will be copied from the template parameter of the enclosing class.");
            static_assert(std::is_base_of_v<T, D>, "Invalid parameter access."); // Note that `is_base_of_v<T, T> == true`.

            if constexpr (std::is_same_v<T, D>)
                return get_base();
            else
                return *static_cast<maybe_const<D> *>(bytes);
        }
    };


    template <typename T> struct DefaultFuncs
    {
        template <typename D> constexpr void _make() {}
    };


    template <typename T, typename UserFuncs = DefaultFuncs<T>>
    class Storage : impl::copyable_if<std::is_copy_constructible_v<T>>
    {
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "The template parameter has to have no cv-qualifiers.");
        static_assert(std::is_class_v<T>, "The template parameter has to be a structure or a class.");
        static_assert(alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__, "Overaligned types are not supported.");

      public:
        static constexpr bool is_copyable = std::is_copy_constructible_v<T>;

      private:
        struct Low
        {
            class Unique
            {
                // A poor man's `std::unique_ptr`.
                // Unlike `unique_ptr`, it also stores a downcasted pointer so that we don't have to use `dynamic_cast` every time if the base turns out to be virtual.
                // This also allows for a relatively graceful deletion even if base doesn't have a virtual destructor.
                // (If multiple inheritance is involved and the base doesn't have a virtual destructor, `unique_ptr` could attempt to `free` an invalid (not adjusted) pointer, causing a crash.
                // This is caused by naively calling `delete` on a pointer to base. We don't do that. Instead, we call the destructor via the base pointer, and then `delete` the downcasted pointer as `char` array.)

                struct Data
                {
                    unsigned char *bytes = 0;
                    T *base = 0;
                };
                Data data;

              public:
                Unique() {}

                template <typename D, typename ...P> static Unique make(P &&... params)
                {
                    Unique ret;

                    ret.data.bytes = new(std::align_val_t(alignof(D))) unsigned char[sizeof(D)];
                    FINALLY_ON_THROW( delete[] ret.data.bytes; )

                    D *derived = new(ret.data.bytes) D(std::forward<P>(params)...);
                    // Not needed because nothing below this point can throw:
                    // FINALLY_ON_THROW( derived->~D(); )

                    ret.data.base = derived;

                    return ret;
                }

                Unique(Unique &&other) noexcept : data(std::exchange(other.data, {})) {}
                Unique &operator=(Unique other) noexcept {std::swap(data, other.data); return *this;}

                ~Unique()
                {
                    if (data.bytes)
                    {
                        data.base->~T();
                        delete[] data.bytes;
                    }
                }

                explicit operator bool() const
                {
                    return bool(data.bytes);
                }

                      unsigned char *bytes()       {return data.bytes;}
                const unsigned char *bytes() const {return data.bytes;}

                      T *base()       {return data.base;}
                const T *base() const {return data.base;}
            };

            struct Table : UserFuncs
            {
                Unique (*_copy)(Param<const T>);

                template <typename D> constexpr void _make()
                {
                    UserFuncs::template _make<D>();

                    if constexpr (is_copyable)
                    {
                        _copy = [](Param<const T> param) -> Unique
                        {
                            return Unique::template make<D>(param.template get<D>());
                        };
                    }
                    else
                    {
                        _copy = 0;
                    }
                }
            };

            template <typename D> inline static constexpr Table table_storage = []{Table ret{}; ret.template _make<D>(); return ret;}();

            struct Data
            {
                Unique pointers;
                const Table *table = 0;
            };
            Data data;

            Low() {}

            Low(Low &&other) noexcept : data(std::exchange(other.data, {})) {}
            Low &operator=(Low other) noexcept {std::swap(data, other.data); return *this;};

            ~Low() {}

            Low(const Low &other)
            {
                if (other)
                {
                    data.pointers = other.data.table->_copy(other);
                    data.table = other.data.table;
                }
            }

            template <typename D, typename ...P> static Low make(P &&... params)
            {
                static_assert(!std::is_const_v<D> && !std::is_volatile_v<D>, "The template parameter has to have no cv-qualifiers.");
                static_assert(std::is_base_of_v<T, D>, "The template parameter has to be equal to T or to be derived from T.");
                static_assert(is_copyable <= std::is_copy_constructible_v<D>, "The base class is copy constructible, so the derived class has to be copy constructible as well.");
                static_assert(std::is_same_v<T, D> || std::has_virtual_destructor_v<T> || std::is_trivially_destructible_v<D>,
                              "If you want to store derived classes, the base class has to have a virtual destructor. Alternatively, those derived classes have to have trivial destructors.");
                static_assert(alignof(D) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__, "Overaligned types are not supported.");

                Low ret;
                ret.data.pointers = Unique::template make<D>(std::forward<P>(params)...);
                ret.data.table = &table_storage<D>;
                return ret;
            }

            explicit operator bool() const {return bool(data.pointers);}

            operator Param<      T>()       {return {data.pointers.base(), data.pointers.bytes()};};
            operator Param<const T>() const {return {data.pointers.base(), data.pointers.bytes()};};
        };

        Low low;

      public:
        Storage(decltype(nullptr)) {}

        template <typename ...P, typename = decltype(T(std::declval<P>()...), void())>
        Storage(P &&... params) : low(Low::template make<T>(std::forward<P>(params)...)) {}

        template <typename D = T, typename ...P, typename = decltype(D(std::declval<P>()...), void())>
        [[nodiscard]] static Storage make(P &&... params)
        {
            Storage ret = nullptr;
            ret.low = Low::template make<D>(std::forward<P>(params)...);
            return ret;
        }

        [[nodiscard]] explicit operator bool() const {return bool(low);}

        [[nodiscard]]       T *get()       {return low.data.pointers.base();}
        [[nodiscard]] const T *get() const {return low.data.pointers.base();}

        [[nodiscard]]       T &operator*()       {return *get();}
        [[nodiscard]] const T &operator*() const {return *get();}

        [[nodiscard]]       T *operator->()       {return get();}
        [[nodiscard]] const T *operator->() const {return get();}

        // Unlike `get()`, this returns the actual pointer to the class even if multiple inheritance is present.
        [[nodiscard]]       unsigned char *bytes()       {return low.data.pointers.bytes();}
        [[nodiscard]] const unsigned char *bytes() const {return low.data.pointers.bytes();}

        [[nodiscard]] const UserFuncs &funcs() const {return *low.data.table;}

        [[nodiscard]] operator Param<      T>()       {return low;};
        [[nodiscard]] operator Param<const T>() const {return low;};
    };
}

template <typename T, typename UserFuncs = Dynamic::DefaultFuncs<T>> using Dyn = Dynamic::Storage<T, UserFuncs>;
