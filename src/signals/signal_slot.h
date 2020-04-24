#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "macros/check.h"
#include "macros/finally.h"
#include "meta/assignable_wrapper.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "signals/connection.h"
#include "signals/connection_list.h"
#include "signals/connection_state.h"

/*   Signals and slots
 * Signals and slots is a convenient way to send messages between objects.
 *
 * Signals can be *connected* to slots.
 * Signals have type `Sig::[Mono]Signal<return_type(param_types...)>`. Slots have type `Sig::[Mono]Slot`.
 * `Mono...` signals and slots can only be connected to a single slot or signal respectively, but are slightly more optimized.
 *
 * Signals and slots are connected using `Sig::Connect()`.
 * Usage:
 *     Sig::Connect(signal, slot_or_object, [mode, object,] callback);
 * Where:
 * - `signal` is an lvalue of type `[Mono]Signal<R(P...)>`.
 *   If it's a `MonoSignal`, then it must be non-const. It's required because connecting to a `MonoSignal` is
 *   potentially destructive, since it removes the previous connection.
 * - `slot_or_object` is a non-const lvalue of type `[Mono]Slot`, or a class derived from those.
 *   If it's a `MonoSlot` or derived from one, then connecting to it removes the previous connection.
 *   If it's derived from `[Mono]Slot` (rather than being one), then the call is equivalent to
 *   `Sig::Connect(signal, static_cast<[Mono]Slot &>(slot_or_object), Sig::relative_to_slot, slot_or_object, callback)`.
 *   See the description of `callback` for details.
 * - `mode`, if it's present, must be `Sig::relative_to_slot` or `Sig::fixed_location`.
 *   See the description of `callback` for details.
 * - `object`, if it's present, is a reference to an arbitrary object that will be passed to the callback.
 *   See the description of `callback` for details.
 * - `callback` is the callback that will be associated with this signal/slot connection.
 *   First of all, the callback with be invoked using `std::invoke`, which makes it possible to use
 *   member function pointers (and even pointers to members) as callbacks.
 *   If `Sig::Connect` was called without specifying `mode` and `object` (and if `slot_or_object` is
 *   a `[Mono]Slot` rather than class derived from one), then the callback will invoked with the parameters
 *   specified in the template parameters of the signal.
 *   If `mode` and `object` are specified (of if `slot_or_object` is derived from a slot), then `object`
 *   (or `slot_or_object` if `object` is not specified) will be passed as an additional first parameter to the callback.
 *   `mode` describes what should happen if the slot is moved. If `mode` is `Sig::fixed_location`, then nothing happens
 *   to the address of `object`. If it's `Sig::relative_to_slot`, then the `object` is assumed to have been moved with
 *   the slot (it's assumed to be located at a fixed offset relative to the address of the slot).
 *
 * Signals and slots are disconnected automatically when destructed. They can also be disconnected manually, see class definitions for details.
 *
 * Signals have `operator()` overloaded, with the parameters specified in their template parameters (roughly, see class definitions for details).
 * Calling `operator()` invokes callbacks associated with all connections attached to the signal.
 *
 * Example:
 *     struct A
 *     {
 *         Sig::Signal<void(int)> signal;
 *     };
 *
 *     struct B : Sig::Slot
 *     {
 *         int value = 42;
 *
 *         void foo(int param)
 *         {
 *             std::cout << value * param << '\n';
 *         }
 *     };
 *
 *     int main()
 *     {
 *         A a;
 *         B b;
 *
 *         a.signal(10); // Does nothing.
 *         Sig::Connect(a.signal, b, &B::foo);
 *         a.signal(10); // Prints `420`.
 *     }
 */


namespace Sig
{
    namespace impl
    {
        template <typename F> struct return_type {};
        template <typename R, typename ...P> struct return_type<R(P...)> {using type = R;};
        template <typename F> using return_type_t = typename return_type<F>::type;


        template <typename A, typename B, typename = void> struct pack_is_convertible : std::false_type {};
        template <typename ...A, typename ...B> struct pack_is_convertible<Meta::type_list<A...>, Meta::type_list<B...>, std::enable_if_t<sizeof...(A) == sizeof...(B)>>
            : std::bool_constant<(std::is_convertible_v<A, B> && ...)> {};
        template <typename A, typename B> inline constexpr bool pack_is_convertible_v = pack_is_convertible<A, B>::value;


        template <typename C, typename O> using detect_insert = decltype(std::declval<C &>().insert(std::declval<O>()));
        template <typename C, typename O> using detect_push_back = decltype(std::declval<C &>().push_back(std::declval<O>()));

        template <typename C, typename O> inline constexpr bool can_append_to = Meta::is_detected<detect_insert, C, O> || Meta::is_detected<detect_push_back, C, O>;

        template <typename C, typename O> void AppendTo(C &container, O &&object)
        {
            static_assert(can_append_to<C, O>, "C is not a suitable container.");
            if constexpr (Meta::is_detected<detect_insert, C, O>)
                container.insert(std::forward<O>(object));
            else
                container.push_back(std::forward<O>(object));
        }


        struct TagSig {};
        struct TagSlot {};

        using SigCon  = Connection<TagSig, TagSlot>;
        using SlotCon = Connection<TagSlot, TagSig>;
        using BasicSigCon  = SigCon ::basic_connection_t;
        using BasicSlotCon = SlotCon::basic_connection_t;

        template <typename F> struct SigState;
        template <typename F> using SigConWithState = ConnectionWithState<SigCon, SigState<F>>;
        template <typename F> using SigList = ConnectionList<SigCon, SigState<F>>;

        struct SlotState;
        using SlotConWithState = ConnectionWithState<SlotCon, SlotState>;
        using SlotList = ConnectionList<SlotCon, SlotState>;


        template <typename F> struct SigState {~SigState() = delete;};

        template <typename R, typename ...P>
        struct SigState<R(P...)>
        {
            SigState() {}

            std::function<R(void *object, P...)> func;
        };

        struct SlotState
        {
            SlotState() {}

            enum class Mode
            {
                // `pointer_or_offset` is a pointer.
                absolute,
                // `pointer_or_offset` is an offset relative to the current `SlotState`.
                relative_to_state,
                // `pointer_or_offset` is an offset relative to the enclosing `ConnectionList`.
                relative_to_list,
            };
            Mode mode = Mode::absolute;

            std::uintptr_t pointer_or_offset = 0;
        };

        // Assumes `sig_con` is connected.
        template <typename R, typename ...P, typename ...Params>
        R InvokeSignal(BasicSigCon &sig_con, Params &&... params)
        {
            BasicSlotCon &slot_con = sig_con.Remote();

            SigState<R(P...)> &sig_state = GetConnectionState<SigState<R(P...)>>(sig_con);
            SlotState &slot_state = GetConnectionState<SlotState>(slot_con);

            void *target_object = nullptr;
            switch (slot_state.mode)
            {
              case SlotState::Mode::absolute:
                target_object = reinterpret_cast<void *>(slot_state.pointer_or_offset);
                break;
              case SlotState::Mode::relative_to_state:
                target_object = reinterpret_cast<char *>(&slot_state) + slot_state.pointer_or_offset;
                break;
              case SlotState::Mode::relative_to_list:
                target_object = reinterpret_cast<char *>(&SlotList::DowncastConnectionToList(slot_con)) + slot_state.pointer_or_offset;
                break;
            }

            return sig_state.func(target_object, std::forward<Params>(params)...);
        }


        // This is defined later.
        struct SigSlotHelper;
    }


    // Tag dispatch helper.
    struct tag_pass_to_t {};
    // Can be passed as the first argument to `Signal::operator()`.
    // Means that the next argument is a callback applied to the return value of each invoked slot.
    inline constexpr tag_pass_to_t pass_to;

    // Tag dispatch helper.
    struct tag_insert_into_t {};
    // Can be passed as the first argument to `Signal::operator()`.
    // Means that the next argument is a callback applied to the return value of each invoked slot.
    inline constexpr tag_insert_into_t insert_into;


    // A signal that can only be connected to a single slot.
    template <typename F = void()>
    class MonoSignal {~MonoSignal() = delete;};

    template <typename R, typename ...P>
    class MonoSignal<R(P...)>
    {
        friend impl::SigSlotHelper;

        mutable impl::SigConWithState<R(P...)> con_with_state;

        using optional_ret_or_void_t = std::conditional_t<std::is_void_v<R>, void, std::optional<R>>;

        template <typename ...Params> static constexpr bool parameters_are_usable = impl::pack_is_convertible_v<Meta::type_list<Params...>, Meta::type_list<P...>>;

      public:
        [[nodiscard]] explicit operator bool() const
        {
            return bool(con_with_state.connection);
        }

        void DisconnectSlot()
        {
            con_with_state.connection.Unbind();
        }

        template <typename ...Params, CHECK(parameters_are_usable<Params...>)>
        optional_ret_or_void_t operator()(Params &&... params)
        {
            if (!con_with_state.connection)
                return optional_ret_or_void_t(); // `return {};` wouldn't work for `R = void`.

            return impl::InvokeSignal<R, P...>(con_with_state.connection, std::forward<Params>(params)...);
        }
    };


    // A signal that can be connected to any number of slots.
    template <typename F = void()>
    class Signal {~Signal() = delete;};

    template <typename R, typename ...P>
    class Signal<R(P...)>
    {
        friend impl::SigSlotHelper;

        mutable impl::SigList<R(P...)> connections;

        // Note `Params &` instead of `Params`. We can't forward the parameters because the callback can be called several times.
        template <typename ...Params> static constexpr bool parameters_are_usable = impl::pack_is_convertible_v<Meta::type_list<Params &...>, Meta::type_list<P...>>;

      public:
        [[nodiscard]] explicit operator bool() const
        {
            return bool(connections.ConnectionCount());
        }

        void DisconnectSlots()
        {
            connections.Clear();
        }
        void DisconnectLastAttachedSlots(std::size_t count)
        {
            connections.RemoveLastConnections(count);
        }

        // Invokes each connected slot, and applies the callback to the returned values.
        // `callback` is `void|bool callback(const R & <or equivalent>)`. Returning `void` is equivalent to returning true.
        // If the callback returns false, the function exits immediately and returns false. Otherwise it returns true.
        template <typename ...Params, typename C, CHECK(parameters_are_usable<Params...> && Meta::is_same_or_void_v<decltype(std::declval<C>()(std::declval<R>())), bool>)>
        bool operator()(tag_pass_to_t, C &&callback, Params &&... params)
        {
            return connections.ForEachConnection([&](impl::BasicSigCon &sig_con)
            {
                return callback(impl::InvokeSignal<R, P...>(sig_con, params...)); // Note lack of forwarding.
            });
        }

        // Invokes each connected slot, and inserts the returned values into a container.
        // The container class must provide a suitable `insert` or `push_back` function.
        template <typename ...Params, typename C, CHECK(parameters_are_usable<Params...> && impl::can_append_to<C, R>)>
        void operator()(tag_insert_into_t, C &container, Params &&... params)
        {
            connections.ForEachConnection([&](impl::BasicSigCon &sig_con)
            {
                impl::AppendTo(container, impl::InvokeSignal<R, P...>(sig_con, params...)); // Note lack of forwarding.
            });
        }

        // Invokes each connected slot, and discards the return values.
        template <typename ...Params, CHECK(parameters_are_usable<Params...>)>
        void operator()(Params &&... params)
        {
            connections.ForEachConnection([&](impl::BasicSigCon &sig_con)
            {
                impl::InvokeSignal<R, P...>(sig_con, params...); // Note lack of forwarding.
            });
        }
    };


    // An alternative to `Slot` can only be attached to a single signal.
    class MonoSlot
    {
        friend impl::SigSlotHelper;

        impl::SlotConWithState con_with_state;

      public:
        [[nodiscard]] explicit operator bool() const
        {
            return bool(con_with_state.connection);
        }

        void DisconnectSignal()
        {
            con_with_state.connection.Unbind();
        }
    };

    class Slot
    {
        friend impl::SigSlotHelper;

        impl::SlotList connections;

      public:
        [[nodiscard]] explicit operator bool() const
        {
            return bool(connections.ConnectionCount());
        }

        void DisconnectSignals()
        {
            connections.Clear();
        }
        void DisconnectLastAttachedSignals(std::size_t count)
        {
            connections.RemoveLastConnections(count);
        }
    };


    namespace impl
    {
        struct SigSlotHelper
        {
            template <typename R, typename ...P>
            static BasicSigCon &GetSigCon(const MonoSignal<R(P...)> &sig)
            {
                return sig.con_with_state.connection;
            }

            template <typename R, typename ...P>
            static SigList<R(P...)> &GetSigCon(const Signal<R(P...)> &sig)
            {
                return sig.connections;
            }

            template <typename S>
            static BasicSigCon &Connect(const S &sig, MonoSlot &slot, bool relative, std::uintptr_t pointer_or_offset)
            {
                BasicSigCon &ret = Bind(SigSlotHelper::GetSigCon(sig), slot.con_with_state.connection);
                SlotState &state = slot.con_with_state.state.value;
                state.pointer_or_offset = pointer_or_offset;
                state.mode = relative ? SlotState::Mode::relative_to_state : SlotState::Mode::absolute;
                return ret;
            }

            template <typename S>
            static BasicSigCon &Connect(const S &sig, Slot &slot, bool relative, std::uintptr_t pointer_or_offset)
            {
                BasicSigCon &ret = Bind(SigSlotHelper::GetSigCon(sig), slot.connections);
                SlotState &state = GetConnectionState<SlotState>(ret.Remote());
                state.pointer_or_offset = pointer_or_offset;
                state.mode = relative ? SlotState::Mode::relative_to_list : SlotState::Mode::absolute; // Note that we use `relative_to_list` here, instead of `relative_to_state` used in the other overload.
                return ret;
            }

            // Relative addresses are computed relative to the return value of this function.
            static void *GetSlotBaseAddress(MonoSlot &slot)
            {
                return &slot.con_with_state.state.value;
            }
            static void *GetSlotBaseAddress(Slot &slot)
            {
                return &slot;
            }
        };


        // Check if `T` is a specialization of `MonoSignal` (strictly non-const) or `Signal` (possibly const).
        // We require `MonoSignal`s to be non-const because connecting to them can be destructive (removes previous connection).
        template <typename T> inline constexpr bool is_connectable_signal_v = Meta::is_specialization_of<T, MonoSignal> || Meta::is_specialization_of<std::remove_const_t<T>, Signal>;

        // Check if `T` is `MonoSlot` or `Slot`.
        template <typename T> inline constexpr bool is_slot_v = std::is_same_v<T, MonoSlot> || std::is_same_v<T, Slot>;

        // Check if `T` is derived from `MonoSlot` or `Slot`.
        template <typename T> inline constexpr bool is_derived_from_slot_v = !is_slot_v<T> && !std::is_const_v<T> && (std::is_base_of_v<MonoSlot, T> || std::is_base_of_v<Slot, T>);

        // If `T` is derived from `MonoSlot` or `Slot`, expands to `MonoSlot` or `Slot` respectively. Otherwise expands to `void`.
        template <typename T> using slot_base_t = std::conditional_t<is_derived_from_slot_v<T>, std::conditional_t<std::is_base_of_v<MonoSlot, T>, MonoSlot, Slot>, void>;


        enum class AddressMode
        {
            unused,
            relative,
            absolute,
        };

        template <typename R, typename ...P, typename Sig, typename Slot, typename O, typename C, AddressMode AddrMode>
        void ConnectLow(const Sig &signal, Slot &slot, O *object, C &&callback, Meta::value_tag<AddrMode>)
        {
            if constexpr (AddrMode == AddressMode::unused)
                static_assert(std::is_invocable_r_v<R, std::remove_cvref_t<C>, P...>, "Invalid callback.");
            else
                static_assert(std::is_invocable_r_v<R, std::remove_cvref_t<C>, O &, P...>, "Invalid callback.");

            std::uintptr_t address_or_offset = 0;
            bool address_is_relative = false;
            switch (AddrMode)
            {
              case AddressMode::unused:
                // Nothing.
                break;
              case AddressMode::relative:
                address_or_offset = std::uintptr_t(object) - std::uintptr_t(SigSlotHelper::GetSlotBaseAddress(slot));
                address_is_relative = true;
                break;
              case AddressMode::absolute:
                address_or_offset = std::uintptr_t(object);
                break;
            }

            BasicSigCon &con = SigSlotHelper::Connect(signal, slot, address_is_relative, address_or_offset);
            SigState<R(P...)> &state = GetConnectionState<SigState<R(P...)>>(con);

            struct Functor
            {
                Meta::AssignableWrapper<std::remove_cvref_t<C>> callback;

                Functor(C &&callback) : callback(std::forward<C>(callback)) {}

                Functor(Functor &&) = default;
                Functor &operator=(Functor &&) = default;

                Functor(const Functor &other) : callback(std::move(other.callback))
                {
                    DebugAssert("Attempt to copy a signal/slot internal callback.", false);
                }
                Functor &operator=(const Functor &other)
                {
                    DebugAssert("Attempt to copy a signal/slot internal callback.", false);
                    callback = std::move(other.callback);
                    return *this;
                }

                R operator()(void *object, P ... params)
                {
                    if constexpr (AddrMode == AddressMode::unused)
                        return std::invoke(callback, std::forward<P>(params)...);
                    else
                        return std::invoke(callback, *static_cast<O *>(object), std::forward<P>(params)...);
                }
            };

            state.func = Functor(std::forward<C>(callback));
        }

        template <typename R, typename ...P, typename ...Params>
        void Connect(const MonoSignal<R(P...)> &signal, Params &&... params)
        {
            // `impl::` prevents ADL.
            impl::ConnectLow<R, P...>(signal, std::forward<Params>(params)...);
        }
        template <typename R, typename ...P, typename ...Params>
        void Connect(const Signal<R(P...)> &signal, Params &&... params)
        {
            // `impl::` prevents ADL.
            impl::ConnectLow<R, P...>(signal, std::forward<Params>(params)...);
        }
    }

    // Connect a signal to an object derived from a slot. The derived object will be passed to the callback.
    template <typename Sig, typename Slot, typename C, CHECK(impl::is_connectable_signal_v<Sig> && impl::is_derived_from_slot_v<Slot>)>
    void Connect(Sig &signal, Slot &slot_like, C &&callback)
    {
        impl::Connect(signal, static_cast<impl::slot_base_t<Slot> &>(slot_like), &slot_like, std::forward<C>(callback), Meta::value_tag<impl::AddressMode::relative>{});
    }

    struct tag_relative_to_slot_t {};
    inline constexpr tag_relative_to_slot_t relative_to_slot;

    // Connect a signal to a slot, and pass a custom object to the callback. That object is assumed to always be located at a fixed offset relative to the slot.
    template <typename Sig, typename Slot, typename O, typename C, CHECK(impl::is_connectable_signal_v<Sig> && impl::is_slot_v<Slot>)>
    void Connect(Sig &signal, Slot &slot, tag_relative_to_slot_t, O &object, C &&callback)
    {
        impl::Connect(signal, slot, &object, std::forward<C>(callback), Meta::value_tag<impl::AddressMode::relative>{});
    }

    struct tag_fixed_location_t {};
    inline constexpr tag_fixed_location_t fixed_location;

    // Connect a signal to a slot, and pass a custom object to the callback. That object is assumed to have a fixed address.
    template <typename Sig, typename Slot, typename O, typename C, CHECK(impl::is_connectable_signal_v<Sig> && impl::is_slot_v<Slot>)>
    void Connect(Sig &signal, Slot &slot, tag_fixed_location_t, O &object, C &&callback)
    {
        impl::Connect(signal, slot, &object, std::forward<C>(callback), Meta::value_tag<impl::AddressMode::absolute>{});
    }

    // No additional objects will be passed to the callback.
    template <typename Sig, typename Slot, typename C, CHECK(impl::is_connectable_signal_v<Sig> && impl::is_slot_v<Slot>)>
    void Connect(Sig &signal, Slot &slot, C &&callback)
    {
        impl::Connect(signal, slot, (void *)nullptr, std::forward<C>(callback), Meta::value_tag<impl::AddressMode::unused>{});
    }
}
