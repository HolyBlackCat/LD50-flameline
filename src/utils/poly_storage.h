#pragma once

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "utils/finally.h"
#include "utils/meta.h"

namespace Poly
{
    /* Poly::Storage is copyable wrapper for std::unique_ptr, with std::visit-like features.
     *
     * When copied, it copies the underlying object. The copy is done properly even if `Poly::Storage<Base>` holds a derived class.
     *
     * You can't modify objects through `const Poly::Storage<T>` (unlike std::unique_ptr).
     *
     * How to construct:
     *     Poly::Storage<MyClass> x; // Allocates nothing.
     *     Poly::Storage<MyClass> x = nullptr; // Same as above.
     *     Poly::Storage<MyClass> x(Poly::base, ...); // Creates an object using an appropriate constructor.
     *     Poly::Storage<MyBase> x(Poly::derived<MyDerived>, ...); // Creates an object of a possibly derived type. If `MyBase == MyDerived`, it has the same effect as the line above.
     *     Poly::Storage<MyBase> x = Poly::Storage<MyBase>::make<MyDerived>(...); // Same as above. If the template argument for `make` is absent, the type defaults to the base class.
     *     Poly::Storage<MyBase> x = x.make<MyDerived>(...); // Same as above.
     *
     * You can also assign to an existing object via `.assign<MyDerived>(...)`. The template parameter is optional. A reference to the resulting derived object is returned.
     *
     * Conversion from `Poly::Storage<Derived>` to `Poly::Storage<Base>` is not supported.
     * Storing arrays is not supported.
     *
     * How to access contents:
     *     bool(obj) // Checks if pointer is not null
     *     obj.base() // Obtain a reference
     *     *obj // Obtain a reference
     *     obj->foo // Access a member
     *     obj.bytes() // Obtains a `unsigned char *` to the actual stored class (rather than its base specified in the template parameter).
     *     obj.derived<MyDerived>() // Obtains a reference to the stored class. If the stored type differs from what you specified, an assertion is triggered (in release builds you'll have a segfault instead).
     *     obj.derived_or_throw<MyDerived() // Same as above, but throws on failure.
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
     *         auto x = Poly::Storage<A>::make(); // Makes an instance of A
     *         auto y = Poly::Storage<A>::make<B>(); // Makes an instance of B
     *
     *         foo(*x); // Prints foo(A)
     *         foo(*y); // Prints foo(A), but we want foo(B)
     *     }
     *
     *     // Here is how we do that:
     *
     *     template <typename BaseT> struct MyFuncs
     *     {
     *         void (*call_foo)(Poly::Storage<A, MyFuncs> &); // You could use a const reference as well.
     *
     *         template <typename DerivedT> constexpr void _make()
     *         {
     *             call_foo = [](Poly::Storage<A, MyFuncs> &param)
     *             {
     *                 foo(param.template derived<DerivedT>());
     *             };
     *         }
     *     };
     *
     *     int main()
     *     {
     *         auto x = Poly::Storage<A,MyFuncs<A>>::make();
     *         auto y = Poly::Storage<A,MyFuncs<A>>::make<B>();
     *
     *         x.dynamic().call_foo(x); // prints foo(A)
     *         y.dynamic().call_foo(y); // prints foo(B)
     *     }
     */


    template <typename T> struct DefaultData
    {
        /* Optional flags:
         *
         * `using _use_fake_copying_if_needed = void;`
         *     Forces `class Storage` to be copyable even if the template parameter is not copyable.
         *     Actual attempt to copy it should cause a segfault.
         */

        template <typename D> constexpr void _make() {}
    };

    template <typename T> using DetectDataFlag_fake_copying = typename T::_use_fake_copying_if_needed;


    template <typename T, typename D> inline static constexpr T type_erasure_data_storage = []{T ret{}; ret.template _make<D>(); return ret;}();


    inline constexpr struct base_tag {} base;

    template <typename T> struct derived_tag {};
    template <typename T> inline constexpr derived_tag<T> derived;


    template <typename T, typename UserData = DefaultData<T>>
    class Storage
        : Meta::copyable_if<std::is_copy_constructible_v<T> || Meta::is_detected<DetectDataFlag_fake_copying, UserData>>
    {
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "The template parameter has to have no cv-qualifiers.");
        static_assert(std::is_class_v<T>, "The template parameter has to be a structure or a class.");
        static_assert(alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__, "Overaligned types are not supported.");

      public:
        static constexpr bool is_copyable = std::is_copy_constructible_v<T>;
        static constexpr bool is_fake_copyable = !is_copyable && Meta::is_detected<DetectDataFlag_fake_copying, UserData>;

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

                    ret.data.bytes = new unsigned char[sizeof(D)];
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
                        // Note that qualifying the destructor call with `T::` silences a clang warning about calling a non-virtual destructor of an abstract class, if we use such class as the template parameter.
                        data.base->T::~T();
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

            struct Table : UserData
            {
                Unique (*_copy)(const Storage::Low &);

                template <typename D> constexpr void _make()
                {
                    UserData::template _make<D>();

                    if constexpr (is_copyable)
                    {
                        _copy = [](const Storage::Low &param) -> Unique
                        {
                            return Unique::template make<D>(param.template derived_or_assert<D>());
                        };
                    }
                    else
                    {
                        _copy = 0;
                    }
                }
            };

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

            template <typename D, typename ...P> static Low make(D **out_ptr, P &&... params)
            {
                static_assert(!std::is_const_v<D> && !std::is_volatile_v<D>, "The template parameter has to have no cv-qualifiers.");
                static_assert(std::is_base_of_v<T, D>, "The template parameter has to be equal to T or to be derived from T.");
                static_assert(is_copyable <= std::is_copy_constructible_v<D>, "The base class is copy constructible, so the derived class has to be copy constructible as well.");
                static_assert(std::is_same_v<T, D> || std::has_virtual_destructor_v<T> || std::is_trivially_destructible_v<D>,
                              "If you want to store derived classes, the base class has to have a virtual destructor. Alternatively, those derived classes have to have trivial destructors.");
                static_assert(alignof(D) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__, "Overaligned types are not supported.");

                Low ret;
                ret.data.pointers = Unique::template make<D>(std::forward<P>(params)...);
                ret.data.table = &type_erasure_data_storage<Table, D>;

                if (out_ptr)
                    *out_ptr = reinterpret_cast<D *>(ret.data.pointers.bytes());

                return ret;
            }

            explicit operator bool() const {return bool(data.pointers);}

            template <typename D> bool contains() const
            {
                static_assert(std::is_base_of_v<T, D>, "This type is not derived from the base.");
                static_assert(!std::is_const_v<D> && !std::is_volatile_v<D>, "The template parameter has to have no cv-qualifiers.");
                return data.table == &type_erasure_data_storage<Table, D>;
            }

            template <typename D> D &derived_or_throw()
            {
                return const_cast<D &>(std::as_const(*this).template derived_or_throw<D>());
            }
            template <typename D> const D &derived_or_throw() const
            {
                if (!contains<D>())
                    throw std::runtime_error("Invalid `Poly::Storage` access.");
                return reinterpret_cast<const D &>(data.pointers.bytes());
            }

            template <typename D> D &derived_or_assert()
            {
                return const_cast<D &>(std::as_const(*this).template derived_or_assert<D>());
            }
            template <typename D> const D &derived_or_assert() const
            {
                assert(contains<D>() && "Invalid Poly::Storage access.");
                return *reinterpret_cast<const D *>(data.pointers.bytes());
            }
        };

        Low low;

      public:
        using base_type = T;

        Storage(decltype(nullptr) = nullptr) {}

        template <typename ...P, typename = decltype(T(std::declval<P>()...), void())>
        Storage(base_tag, P &&... params) : low(Low::template make<T>(nullptr, std::forward<P>(params)...)) {}

        template <typename D, typename ...P, typename = decltype(D(std::declval<P>()...), void())>
        Storage(derived_tag<D>, P &&... params) : low(Low::template make<D>(nullptr, std::forward<P>(params)...)) {}

        template <typename D = T, typename ...P, typename = decltype(D(std::declval<P>()...), void())>
        D &assign(P &&... params)
        {
            D *ret;
            low = Low::template make<D>(&ret, std::forward<P>(params)...);
            return *ret;
        }

        template <typename D = T, typename ...P, typename = decltype(D(std::declval<P>()...), void())>
        [[nodiscard]] static Storage make(P &&... params)
        {
            Storage ret;
            ret.low = Low::template make<D>(nullptr, std::forward<P>(params)...);
            return ret;
        }

        [[nodiscard]] explicit operator bool() const {return bool(low);}

        [[nodiscard]]       T &base()       {return *low.data.pointers.base();}
        [[nodiscard]] const T &base() const {return *low.data.pointers.base();}

        [[nodiscard]]       T &operator*()       {return base();}
        [[nodiscard]] const T &operator*() const {return base();}

        [[nodiscard]]       T *operator->()       {return &base();}
        [[nodiscard]] const T *operator->() const {return &base();}

        // Unlike `get()`, this returns the actual pointer to the class even if multiple inheritance is present.
        [[nodiscard]]       unsigned char *bytes()       {return low.data.pointers.bytes();}
        [[nodiscard]] const unsigned char *bytes() const {return low.data.pointers.bytes();}

        [[nodiscard]] const UserData &dynamic() const {return *low.data.table;}

        template <typename D>
        [[nodiscard]] bool contains() const {return low.template contains<D>();}

        template <typename D> [[nodiscard]]       D &derived()       {return low.template derived_or_assert<D>();}
        template <typename D> [[nodiscard]] const D &derived() const {return low.template derived_or_assert<D>();}

        template <typename D> [[nodiscard]]       D &derived_or_throw()       {return low.template derived_or_throw<D>();}
        template <typename D> [[nodiscard]] const D &derived_or_throw() const {return low.template derived_or_throw<D>();}
    };
}
