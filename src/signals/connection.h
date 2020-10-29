#pragma once

#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>

#include "macros/finally.h"
#include "meta/basic.h"
#include "program/errors.h"

// This file implements connections.
// They are classes that can bind to each other and maintain the binding even if moved.

namespace Sig
{
    // A suitable owner for a connection.
    template <typename T>
    concept ConnectionOwner = std::is_class_v<T>;

    // A suitable additional state for a connection.
    // Move-assignability alone probably could be enough (since the connections always default-construct the state), but we also ask for move-constructibility for a good measure.
    // We could probably also tolerate cv-qualifiers, but I don't see how that could be useful.
    template <typename T>
    concept ConnectionState =
        std::is_object_v<T> &&
        !std::is_const_v<T> && !std::is_volatile_v<T> &&
        std::default_initializable<T> && std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>;

    // An empty connection state that's used by default.
    struct EmptyState {};
    static_assert(ConnectionState<EmptyState>);

    namespace impl
    {
        /* Class hierarchy:
         *
         *           impl::ConnectionAliases
         *                  /          \
         *                 /            \__________________________________
         *    impl::BasicConnection                                        \
         *       /  |       \                                               \
         *      /   |  (connection list element)  <--contains--  impl::BasicConnectionList
         *     /    |                                                       /  \
         * ___/_____|______________________________________________________/____\__________
         *    |     |               Exposed classes:                      /      \
         *    |     |                                                    /      FixedOwner::ConnectionList
         *    |    FixedOwner::Connection                               /
         *    |                                                       ConnectionList
         *  Connection
         *
         *
         * `FixedOwner::Connection[List]` clases don't change their owner unless they're told to.
         * `Connection[List]` clases change their owner automatically when moved,
         *   while assuming that the owner is at a fixed offset relative to themselves.
         */

        template <ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT, ConnectionState LocalStateT, ConnectionState RemoteStateT>
        class BasicConnection;

        // A helper class containing things related to connection callbacks.
        template <ConnectionOwner LocalOwner, ConnectionOwner RemoteOwner, ConnectionState LocalState, ConnectionState RemoteState>
        struct ConnectionCallbacks
        {
            ConnectionCallbacks() = delete;
            ~ConnectionCallbacks() = delete;

            using base_t         = BasicConnection<LocalOwner, RemoteOwner, LocalState, RemoteState>;
            using owner_t        = LocalOwner;
            using remote_owner_t = RemoteOwner;

            // A polymorphic class, containing connection callbacks in form of virtual functions.
            struct Base
            {
                using base_t         = ConnectionCallbacks::base_t;
                using owner_t        = ConnectionCallbacks::owner_t;
                using remote_owner_t = ConnectionCallbacks::remote_owner_t;

                // This is called when the connection is unbound from some other connection,
                //   EXCEPT when that happens because the connection died (the local one; if the remote dies you still get the callback).
                // If you want to handle the destruction too, call `Unbind()` before you destroy the connection.
                // The callback is defined this way to simplify the implementation of connection lists;
                //   list elements remove themselves from the list when they receive this callback.
                virtual void OnUnbindAlive(const base_t &local_con, LocalOwner &old_local_owner, RemoteOwner &old_remote_owner) noexcept
                {
                    (void)local_con;
                    (void)old_local_owner;
                    (void)old_remote_owner;
                }

                // This is called when the owner of a connection (or a connection list) is moved.
                // This can happen when the owner is changed manually, or when the connection or list itself is moved. In the latter case,
                //   the new owner address is calculated under the assumption that the connection is stored at a fixed offset relative to the owner.
                // This callback is not emitted when a connection list moves its elements around in the vector, because it doesn't change their owner.
                virtual void OnRemoteOwnerMoved(const base_t &local_con, LocalOwner &local_owner, RemoteOwner &old_remote_owner, RemoteOwner &new_remote_owner) noexcept
                {
                    (void)local_con;
                    (void)local_owner;
                    (void)old_remote_owner;
                    (void)new_remote_owner;
                }

                // Note that there's no `OnBind`, because if it existed, it would be useful to be able to throw from that, and callbacks must be `noexcept`.
                // If you want to run code on bind, wrap the connection in a class and wrap `Bind` in your own function.

                // Note that there's no `OnLocalOwnerMoved`, because we're assuming that you can customize the move operations of the local owner.
            };

            // Is true if `T` is a valid callbacks class derived from the `Base`.
            // This should've been a concept, but they're not allowed at class scope.
            template <typename T>
            static constexpr bool is_valid_custom_callbacks_class_v = std::derived_from<T, Base> && !std::is_const_v<T> && !std::is_volatile_v<T> && std::default_initializable<T>;

            // A function pointer type that stores a "combined" callback.
            // If `new_remote_owner` is null, acts as `OnUnbindAlive`, otherwise acts as `OnRemoteOwnerMoved`. None of the other pointers can be null.
            using combined_callback_ptr_t = void(*)(const base_t *local_con, LocalOwner *local_owner, RemoteOwner *old_remote_owner, RemoteOwner *new_remote_owner) noexcept;

            // Constructs a single `combined_callback_ptr_t` from all callbacks present in `T`, which must be derived from the `Base`.
            template <typename T> requires is_valid_custom_callbacks_class_v<T>
            static constexpr combined_callback_ptr_t invoker = std::is_same_v<T, Base> ? nullptr :
                [](const base_t *local_con, LocalOwner *local_owner, RemoteOwner *old_remote_owner, RemoteOwner *new_remote_owner) noexcept
                {
                    Base &&base = T{}; // Using a reference to `Base` in case the functions are not public in `T`.
                    if (new_remote_owner)
                        base.OnRemoteOwnerMoved(*local_con, *local_owner, *old_remote_owner, *new_remote_owner);
                    else
                        base.OnUnbindAlive(*local_con, *local_owner, *old_remote_owner);
                };

        };

        // A helper class with some type aliases, common for connections and connection lists.
        template <ConnectionOwner LocalOwner, ConnectionOwner RemoteOwner, ConnectionState LocalState, ConnectionState RemoteState>
        struct ConnectionAliases
        {
            // The classes that connections and connection list elements inherit from.
            using base_t        = BasicConnection<LocalOwner , RemoteOwner, LocalState , RemoteState>;
            using remote_base_t = BasicConnection<RemoteOwner, LocalOwner , RemoteState, LocalState >;

            // The object owning a connection or a list.
            using owner_t        = LocalOwner;
            using remote_owner_t = RemoteOwner;

            // The user state that's stored in a connection or in an element of a connection list.
            using state_t        = LocalState;
            using remote_state_t = RemoteState;

            // Custom callback classes must inherit from this type.
            using basic_callbacks_t        = typename ConnectionCallbacks<LocalOwner, RemoteOwner, LocalState, RemoteState>::Base;
            using remote_basic_callbacks_t = typename ConnectionCallbacks<RemoteOwner, LocalOwner, RemoteState, LocalState>::Base;
        };

        // A helper class that invokes the `Bind` member function of the object that you give it (which can be a connection or a list).
        // A class is necessary because we need to be`friend` it, which is necessary because the functions it will be calling should be non-public.
        // We keep the `Bind` member functions non-public because they can be asymmetrical, e.g. a connection list knows how to bind to a connection,
        //   but not vice versa, and we don't want to expose this assymetry to the user.
        // See comment on `Sig::Bind` for why this is not `noexcept`.
        struct BindInvoker
        {
            template <auto LocalGetter, auto RemoteGetter, typename LocalCallbacks, typename RemoteCallbacks>
            static void Bind(auto &con_a, auto &owner_a, auto &con_b, auto &owner_b) requires
                requires{ { con_a.template Bind<LocalGetter, RemoteGetter, LocalCallbacks, RemoteCallbacks>(owner_a, con_b, owner_b) } -> std::same_as<void>; }
            {
                con_a.template Bind<LocalGetter, RemoteGetter, LocalCallbacks, RemoteCallbacks>(owner_a, con_b, owner_b);
            }
        };

        // An incomplete 'connection' class.
        // Is non-movable, but has some protected functions to help make derived classes movable.
        template <ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT, ConnectionState LocalStateT, ConnectionState RemoteStateT>
        class BasicConnection : public ConnectionAliases<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
        {
            using aliases = ConnectionAliases<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

          public:
            using typename aliases::base_t;
            using typename aliases::remote_base_t;

            using typename aliases::owner_t;
            using typename aliases::remote_owner_t;

            using typename aliases::state_t;
            using typename aliases::remote_state_t;

            using typename aliases::basic_callbacks_t;
            using typename aliases::remote_basic_callbacks_t;

          private:
            friend remote_base_t;
            friend BindInvoker;

            using callbacks_helper_t        = ConnectionCallbacks<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;
            using remote_callbacks_helper_t = ConnectionCallbacks<RemoteOwnerT, LocalOwnerT, RemoteStateT, LocalStateT>;

            struct Data
            {
                // Points to the remote `BasicConnection`, or null if not bound.
                // In theory, if we only wanted to support 1-1 connections, we could omit this field by putting
                //   the information on how to obtain connection from the owner into the type-erasure callback.
                // But since we do support connection lists, we need an extra field. We could use something
                //   like `std::size_t index_in_remote_list`, but a pointer makes things a lot easier.
                remote_base_t *remote_con = nullptr;

                // Points to the owner of `*remote_con`, or null if `remote_con` itself is null.
                remote_owner_t *remote_owner = nullptr;

                // A callback that's called when certain things happen to the remote.
                // See comment on `ConnectionCallbacks::invoker` for details.
                // Can be null even if the connection is bound, check before calling it.
                using remote_combined_callback_ptr_t = typename remote_callbacks_helper_t::combined_callback_ptr_t;
                remote_combined_callback_ptr_t remote_combined_callback = nullptr;

                // Opaque state of a user-specified type.
                [[no_unique_address]] state_t local_state;
            };
            Data data;

          protected:
            constexpr BasicConnection() noexcept {}

            // The class is not copyable/movable directly, but nothing stops derived classes from being movable.
            // Use `MoveFromWithoutCallback` when implementing move operations.
            BasicConnection(const BasicConnection &) = delete;
            BasicConnection &operator=(const BasicConnection &) = delete;

            ~BasicConnection()
            {
                // Unbind without calling the callback.
                LowUnbind(false);
            }

            // Runs some `ASSERT()`s on the state of the connection.
            void AssertConsistency() const noexcept
            {
                ASSERT(bool(data.remote_con) == bool(data.remote_owner)); // These must both be either null or non-null.
                ASSERT(bool(data.remote_combined_callback) <= bool(data.remote_con)); // If the callback is set, the remote pointers must also be set.
                ASSERT(!data.remote_con || data.remote_con->data.remote_con == this); // If the remote pointer is set, make sure the remote is also pointing back at us.
            }

          public:
            // Returns true if this connection is bound to an another connection.
            [[nodiscard]] bool IsBound() const noexcept
            {
                AssertConsistency();
                return bool(data.remote_con);
            }
            [[nodiscard]] explicit operator bool() const noexcept
            {
                return IsBound();
            }

            // Returns the owner of this object.
            // Doesn't work if not connected, in that case returns null.
            // Note that unlike the other getters, this is not const-correct.
            [[nodiscard]] owner_t *Owner() const noexcept
            {
                if (!*this) // This automatically does `AssertConsistency()`.
                    return nullptr;
                return data.remote_con->data.remote_owner;
            }
            // Returns the owner of the remote connection, or null if not connected.
            // Note that unlike the other getters, this is not const-correct.
            [[nodiscard]] remote_owner_t *RemoteOwner() const noexcept
            {
                AssertConsistency();
                return data.remote_owner;
            }
            [[nodiscard]] remote_owner_t &operator*() const noexcept
            {
                return *RemoteOwner();
            }
            [[nodiscard]] remote_owner_t *operator->() const noexcept
            {
                return RemoteOwner();
            }

            // Returns the remote connection, or null if not connected.
            // Note that we only provide a const reference, which prevents you from rebinding it, because that would end badly
            //   if the remote is an element of a connection list (because it would be destroyed as soon as the old binding is lost, before a new one could be created).
            [[nodiscard]] const remote_base_t *RemoteConnection() const noexcept
            {
                AssertConsistency();
                return data.remote_con;
            }

            // Returns the state of this connection.
            [[nodiscard]]       state_t &State()       noexcept {return data.local_state;}
            [[nodiscard]] const state_t &State() const noexcept {return data.local_state;}

            // Returns the state of the remote connection, or null if not connected.
            [[nodiscard]]       state_t *RemoteState()       noexcept {AssertConsistency(); return data.remote_con ? &data.remote_con->data.local_state : nullptr;}
            [[nodiscard]] const state_t *RemoteState() const noexcept {AssertConsistency(); return data.remote_con ? &data.remote_con->data.local_state : nullptr;}

            // If this object is connected, removes that connection. Otherwise does nothing.
            void Unbind() noexcept
            {
                LowUnbind();
            }

          private:
            // If this object is connected, removes that connection. Otherwise does nothing.
            // If the parameter is false, doesn't call the unbind callback on
            //   the local object (the destructor wants that), but always call it on the remote.
            void LowUnbind(bool invoke_local_callback = true) noexcept
            {
                if (!*this) // This automatically does `AssertConsistency()`.
                    return;

                data.remote_con->AssertConsistency();

                // Backup state before we unbind.
                Data old_data = data;
                auto old_remote_data = data.remote_con->data;

                // Actually unbind.
                data.remote_con->data = {};
                data = {};

                // Invoke callbacks.
                if (old_data.remote_combined_callback)
                    old_data.remote_combined_callback(old_data.remote_con, old_data.remote_owner, old_remote_data.remote_owner, nullptr);
                if (old_remote_data.remote_combined_callback && invoke_local_callback)
                    old_remote_data.remote_combined_callback(old_remote_data.remote_con, old_remote_data.remote_owner, old_data.remote_owner, nullptr);
            }

          protected:
            // Bind this connection (owned by `owner`) to a different one (`remote`, owned by `remote_owner`).
            // The specified callbacks are saved, and will be invoked later.
            // Old bindings on this connection and `remote` are lost, unbind callbacks are called for them.
            // This function is supposed to be used by derived classes in their implementations of `Bind`, see below.
            template <typename LocalCallbacks, typename RemoteCallbacks>
            void LowBind(owner_t &owner, remote_base_t &remote, remote_owner_t &remote_owner) noexcept
            {
                if (data.remote_con == &remote && data.remote_owner == &remote_owner)
                    return; // Already bound to that remote and owner, stop.

                if constexpr (std::is_same_v<base_t, remote_base_t>)
                {
                    if (this == &remote)
                        return; // Refuse to bind to self.
                }

                // Remove old binding, if any.
                Unbind();
                remote.Unbind();

                // Make the new binding.
                data.remote_owner = &remote_owner;
                remote.data.remote_owner = &owner;

                data.remote_con = &remote;
                remote.data.remote_con = this;

                data.remote_combined_callback = remote_callbacks_helper_t::template invoker<RemoteCallbacks>;
                remote.data.remote_combined_callback = callbacks_helper_t::template invoker<LocalCallbacks >;

                AssertConsistency();
            }

            // Wraps `LowBind` with an interface understood by `BindInvoker`.
            // This can bind to anything derived from `BasicConnection`.
            template <auto LocalGetter, auto RemoteGetter, typename LocalCallbacks, typename RemoteCallbacks>
            void Bind(LocalOwnerT &owner, remote_base_t &remote, RemoteOwnerT &remote_owner) noexcept
            {
                LowBind<LocalCallbacks, RemoteCallbacks>(owner, remote, remote_owner);
            }

            // Move `other` into this object.
            // If `this == &other`, does nothing.
            // The binding on the current object is lost, if any. The unbind callback is called in that case.
            // The "remote owner moved" callback is not invoked, and the owner pointer is not changed.
            void MoveFrom(BasicConnection &other) noexcept
            {
                if (this == &other)
                    return;

                // Disard the current binding, if any.
                Unbind();

                // If `other` is not bound, only move the state and stop.
                if (!other)
                {
                    // We don't use `std::exchange` to reduce the number of moves.
                    // We don't use `= {}` in case the state has some weird `operator=` overloads.
                    data.local_state = std::move(other.data.local_state);
                    other.data.local_state = state_t{};
                    return;
                }

                // Move all members.
                // We don't use `std::exchange` to reduce the number of moves.
                data = std::move(other.data);
                other.data = {};
                // Tell the remote our new address.
                data.remote_con->data.remote_con = this;

                AssertConsistency();
            }

            // Change the owner of this object to `new_owner`.
            // Since it requires modifying remote state, does nothing if this connection is not bound.
            // Does nothing if the already have this owner.
            // Invokes the "remote owner moved" callback on the remote.
            // If this is called together with `MoveFrom`, I think `MoveFrom` should be called first.
            void OwnerWasMovedTo(owner_t &new_owner) noexcept
            {
                if (!*this)
                    return; // Stop if not bound.

                if (data.remote_con->data.remote_owner == &new_owner)
                    return; // If the owner didn't change, stop early to avoid invoking the callback.

                // Remember the old owner.
                owner_t &old_owner = *data.remote_con->data.remote_owner;

                // Switch to the new owner.
                data.remote_con->data.remote_owner = &new_owner;

                // Then invoke the callback.
                if (data.remote_combined_callback)
                    data.remote_combined_callback(data.remote_con, data.remote_owner, &old_owner, &new_owner);
            }

            // Moves this object from `other`, and automatically updates the owner assuming it's at a fixed offset relative to this object.
            // Does nothing if `this == &other`.
            void MoveFromAndUpdateOwner(BasicConnection &other) noexcept
            {
                // None of this needs a self-assignment check.
                MoveFrom(other);
                if (*this)
                {
                    // Calculate new owner address, assuming it's at a fixed offset relative to the connection.
                    char *new_owner = reinterpret_cast<char *>(this) + (reinterpret_cast<char *>(Owner()) - reinterpret_cast<char *>(&other));
                    OwnerWasMovedTo(*reinterpret_cast<LocalOwnerT *>(new_owner));
                }
            }
        };

        // An incomplete 'connection list' class.
        // Is non-movable, but has some protected functions to help make derived classes movable.
        template <
            ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT,
            ConnectionState LocalStateT = EmptyState, ConnectionState RemoteStateT = EmptyState
        >
        class BasicConnectionList : public ConnectionAliases<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
        {
            friend BindInvoker;

            // A type of the symmetric list.
            using remote_list_t = BasicConnectionList<RemoteOwnerT, LocalOwnerT, RemoteStateT, LocalStateT>;
            friend remote_list_t;

            using aliases = ConnectionAliases<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

            // Wraps existing callbacks, adding code to `OnUnbindAlive` to erase the unbound connection from the list.
            template <typename Orig/*Original callbacks*/, auto GetLocalListFromOwner>
            struct ExtraCallbacks : Orig
            {
                void OnUnbindAlive(const typename Orig::base_t &local_con, typename Orig::owner_t &old_local_owner, typename Orig::remote_owner_t &old_remote_owner) noexcept override
                {
                    // Invoke the callback first, while `local_con` is still alive.
                    Orig::OnUnbindAlive(local_con, old_local_owner, old_remote_owner);

                    // Erase `local_con` from `.elements`.
                    auto &local_list_elems = std::invoke(GetLocalListFromOwner, old_local_owner).elements;
                    using elem_t = typename std::remove_cvref_t<decltype(local_list_elems)>::value_type;
                    std::size_t index = &static_cast<const elem_t &>(local_con) - local_list_elems.data();
                    ASSERT( index < local_list_elems.size() );
                    local_list_elems.erase(local_list_elems.begin() + index);
                }
            };

            // This is what is actually stored in the connection list.
            // Similar to `Connection`, but has custom bind funcitons and can't be bound with `Sig::Bind`.
            // Unlike `Connection`, doesn't recalculate the owner address if moved.
            struct Elem : public BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
            {
                friend BasicConnectionList;
                using base = BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

                constexpr Elem() noexcept {}

                Elem(Elem &&other) noexcept
                {
                    base::MoveFrom(other);
                }
                Elem &operator=(Elem &&other) noexcept
                {
                    base::MoveFrom(other);
                    return *this;
                }

                // Binds this object to something derived from `BasicConnection`, adding `ExtraCallbacks` to the local callbacks.
                template <auto GetLocalListFromOwner, typename LocalCallbacks, typename RemoteCallbacks>
                void BindThisElem(LocalOwnerT &owner, typename base::remote_base_t &remote, RemoteOwnerT &remote_owner) noexcept
                {
                    using ExtraLocalCallbacks = ExtraCallbacks<LocalCallbacks, GetLocalListFromOwner>;
                    base::template LowBind<ExtraLocalCallbacks, RemoteCallbacks>(owner, remote, remote_owner);
                }
                // Binds this object to a `BasicConnectionList`, adding `ExtraCallbacks` to both callbacks.
                template <auto GetLocalListFromOwner, auto GetRemoteListFromOwner, typename LocalCallbacks, typename RemoteCallbacks>
                void BindThisElemToOtherListElem(LocalOwnerT &owner, typename base::remote_base_t &remote, RemoteOwnerT &remote_owner) noexcept
                {
                    using ExtraLocalCallbacks = ExtraCallbacks<LocalCallbacks, GetLocalListFromOwner>;
                    using ExtraRemoteCallbacks = ExtraCallbacks<RemoteCallbacks, GetRemoteListFromOwner>;
                    base::template LowBind<ExtraLocalCallbacks, ExtraRemoteCallbacks>(owner, remote, remote_owner);
                }
            };
            // The list of connections.
            std::vector<Elem> elements;

            // This can bind to anything derived from `BasicConnection`.
            template <auto LocalGetter, auto RemoteGetter, typename LocalCallbacks, typename RemoteCallbacks>
            void Bind(LocalOwnerT &owner, typename Elem::remote_base_t &remote, RemoteOwnerT &remote_owner) noexcept
            {
                // First, explicitly unbind the remote!
                // We have to do this in case it's already bound to our list, because in that case the next line would probably malfunction.
                remote.Unbind();
                elements.emplace_back().template BindThisElem<LocalGetter, LocalCallbacks, RemoteCallbacks>(owner, remote, remote_owner);
            }

            // This can bind to a `BasicConnectionList`.
            // This is not noexcept on purpose, because the element allocation can throw.
            template <auto LocalGetter, auto RemoteGetter, typename LocalCallbacks, typename RemoteCallbacks>
            void Bind(LocalOwnerT &owner, remote_list_t &remote, RemoteOwnerT &remote_owner)
            {
                // We don't check if the second list is already bound to ours, add unconditionally add a new connection.
                // This could've been avoided only in linear time, which is not good enough.
                Elem &local_elem = elements.emplace_back();
                FINALLY_ON_THROW( elements.pop_back(); ) // The next `emplace_back` is the only thing that can throw here.
                typename remote_list_t::Elem &remote_elem = remote.elements.emplace_back(); // Nothing throws below this point, so no scope guard.
                local_elem.template BindThisElemToOtherListElem<LocalGetter, RemoteGetter, LocalCallbacks, RemoteCallbacks>(owner, remote_elem, remote_owner);
            }

          protected:
            constexpr BasicConnectionList() noexcept {}

            // Updates the owner for all elements.
            // Does nothing if already using that owner.
            void OwnerWasMovedTo(LocalOwnerT &new_owner) noexcept
            {
                // We don't have to manually check if the owner didn't change.
                for (Elem &elem : elements)
                    elem.OwnerWasMovedTo(new_owner);
            }

            // Moves this object from `other`.
            // Does nothing if `this == &other`.
            void MoveFrom(BasicConnectionList &other) noexcept
            {
                if (this == &other)
                    return;

                // Clear the list first.
                // Note that we can't just overwrite it, because it wouldn't call the user callbacks.
                UnbindAll();
                // Move the elements.
                elements = std::move(other.elements);
            }

            // Moves this object from `other`, and automatically updates the owner assuming it's at a fixed offset relative to this object.
            // Does nothing if `this == &other`.
            void MoveFromAndUpdateElemOwners(BasicConnectionList &other) noexcept
            {
                // None of this needs a self-assignment check.

                // Move the state.
                MoveFrom(other);

                if (elements.empty())
                    return; // Stop if no elements, because we need at least one to do calculations.

                // Calculate new owner address, assuming it's at a fixed offset relative to the connection.
                // This assumes that all elements have the same owner.
                char *new_owner = reinterpret_cast<char *>(this) + (reinterpret_cast<char *>(elements.front().Owner()) - reinterpret_cast<char *>(&other));
                OwnerWasMovedTo(*reinterpret_cast<LocalOwnerT *>(new_owner));
            }

          public:
            // Returns a const reference to one of the sub-connections (`const BasicConnection<...> &`).
            // The reference is const because rebinding a sub-connection would break things (the connection
            //   would be destroyed as soon as the old binding is lost, in the middle of the rebinding process).
            [[nodiscard]] const typename aliases::base_t &operator[](std::size_t index) const noexcept
            {
                ASSERT(index < elements.size());
                const typename aliases::base_t &ret = elements[index];
                ASSERT(ret);
                return ret;
            }

            // Returns the amount of the sub-connections.
            [[nodiscard]] std::size_t Size() const noexcept
            {
                return elements.size();
            }

            // Removes a specific connection from the list.
            void Unbind(std::size_t index) noexcept
            {
                ASSERT(index < elements.size());
                // The unbound connection is removed from the list automatically by its callback.
                // Note that erasing the element and relying on its destructor would be wrong, since it wouldn't call the user callback.
                elements[index].Unbind();
            }

            // Removes all connections from the list.
            void UnbindAll() noexcept
            {
                // Note that we can't just do `elements.clear()`, since it wouldn't call the user callback.
                std::size_t i = elements.size();
                // Loop from the back to avoid pointlessly moving the elements around.
                while (i-- > 0)
                    Unbind(i);
            }
        };
    }


    // Binds two connections to each other.
    // Example usage: `Sig::Bind<&A::con_a, &B::con_b>(a, b);`,
    //   where `a`,`b` are instances of classes `A`,`B`, having member variables `con_a`,`con_b`, which are `Sig::Connection`s or `Sig::ConnectionList`s.
    // Connections returned by the getters must be symmetrical, i.e. `Sig::Connection[List]<A,B,X,Y>` can only bind to `Sig::Connection[List]<B,A,Y,X>` (lists can bind to non-lists).
    // The getters don't have to be member pointers; they are used as `std::invoke(getter, connection)`,
    //   but NOTE that the connections returned by the getters have to be located AT A FIXED OFFSET relative to their owners. If moved, they'll look for a new owner at that offset.
    // You can optionally provide callbacks a the 3rd and 4th parameters, for the first and the second connections respectively. Those callbacks
    //   must inherit from `::basic_callbacks_t` (a member type of the respective connection), which is an alias for `Sig::impl::ConnectionCallbacks<...>::Base`.
    // Note that this is not `noexcept`, it can throw only if a connection list runs out of memory.
    template <
        auto GetterA,
        auto GetterB,
        typename CallbacksOrVoidA = void,
        typename CallbacksOrVoidB = void,
        Meta::deduce..., typename OwnerA, typename OwnerB
    >
    requires std::is_nothrow_invocable_v<decltype(GetterA), OwnerA &> && std::is_nothrow_invocable_v<decltype(GetterB), OwnerB &>
    void Bind(OwnerA &owner_a, OwnerB &owner_b)
    {
        // Figure out connection types.
        using con_a = std::remove_cvref_t<std::invoke_result_t<decltype(GetterA), OwnerA &>>;
        using con_b = std::remove_cvref_t<std::invoke_result_t<decltype(GetterB), OwnerB &>>;
        // If no callbacks were specified, figure out the types for empty callbacks.
        using callbacks_a = std::conditional_t<!std::is_void_v<CallbacksOrVoidA>, CallbacksOrVoidA, typename con_a::basic_callbacks_t>;
        using callbacks_b = std::conditional_t<!std::is_void_v<CallbacksOrVoidB>, CallbacksOrVoidB, typename con_b::basic_callbacks_t>;

        // Try `a.Bind<...>(..., b, ...)`. If it fails, try the reverse. If that also fails, raise a static assertion.
        // See comment on `BindInvoker` for why the member functions are not symmetrical.
        if constexpr (requires{
            impl::BindInvoker::Bind<GetterA, GetterB, callbacks_a, callbacks_b>(std::invoke(GetterA, owner_a), owner_a, std::invoke(GetterB, owner_b), owner_b);})
            impl::BindInvoker::Bind<GetterA, GetterB, callbacks_a, callbacks_b>(std::invoke(GetterA, owner_a), owner_a, std::invoke(GetterB, owner_b), owner_b);
        else if constexpr (requires{
            impl::BindInvoker::Bind<GetterB, GetterA, callbacks_b, callbacks_a>(std::invoke(GetterB, owner_b), owner_b, std::invoke(GetterA, owner_a), owner_a);})
            impl::BindInvoker::Bind<GetterB, GetterA, callbacks_b, callbacks_a>(std::invoke(GetterB, owner_b), owner_b, std::invoke(GetterA, owner_a), owner_a);
        else
        {
            // Note that the following `static_assert` is written in a very specific way to make it output a nice message.
            constexpr bool x = Meta::value<false, Meta::value_tag<GetterA>, Meta::value_tag<GetterB>, CallbacksOrVoidA, CallbacksOrVoidB>;
            using A = std::invoke_result_t<decltype(GetterA), OwnerA>;
            using B = std::invoke_result_t<decltype(GetterB), OwnerB>;
            static_assert(("Don't know how to `Sig::Bind` types", Meta::tag<A>{}, "and", Meta::tag<B>{}, x));
        }
    }


    // Those connections and connection lists are immovable for safety, but can be moved manually with a special member function.
    // The owner can be updated manually with a member function.
    namespace FixedOwner
    {
        // Makes some low-level functions of `BasicConnection` public, and has a public construtor.
        // See `Sig::Connection` for more details on connections in general.
        template <
            ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT,
            ConnectionState LocalStateT = EmptyState, ConnectionState RemoteStateT = EmptyState
        >
        class Connection : public impl::BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
        {
            using base = impl::BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

          public:
            constexpr Connection() noexcept {}

            using base::MoveFrom;
            using base::OwnerWasMovedTo;
        };

        // Makes some low-level functions of `impl::BasicConnection` public, and has a public construtor.
        // See `Sig::ConnectionList` for more details on connections in general.
        template <
            ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT,
            ConnectionState LocalStateT = EmptyState, ConnectionState RemoteStateT = EmptyState
        >
        class ConnectionList : public impl::BasicConnectionList<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
        {
            using base = impl::BasicConnectionList<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

          public:
            constexpr ConnectionList() noexcept {}

            using base::MoveFrom;
            using base::OwnerWasMovedTo;
        };
    }

    // A connection.
    // Can be bound to a single `Connection` or `ConnectionList` (the type of which has to be symmetric to this one, see the comment on `Bind`).
    // When bound, stores a pointer to the remote connection and to its owner (the enclosing class, possibly indirectly enclosing),
    //   and some type-erased callbacks that are called automatically in certain situations.
    // If `LocalStateT` is specified, also stores a value of that type, which can be freely accessed.
    // When const, it can't be bound/unbound, the user state becomes const, and the remote connection will be exposed only as const. Everything else shouldn't change.
    // NOTE: This class can be moved, but if it's connected, it will look for a new owner at the same offset relative to itself, be careful.
    template <
        ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT,
        ConnectionState LocalStateT = EmptyState, ConnectionState RemoteStateT = EmptyState
    >
    class Connection : public impl::BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
    {
        using base = impl::BasicConnection<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

      public:
        constexpr Connection() noexcept {}

        Connection(Connection &&other) noexcept
        {
            base::MoveFromAndUpdateOwner(other);
        }

        Connection &operator=(Connection &&other) noexcept
        {
            base::MoveFromAndUpdateOwner(other); // This automatically does a self-assignment check.
            return *this;
        }
    };

    // Same as `Connection`, but can bind to more than one connection or connection list.
    template <
        ConnectionOwner LocalOwnerT, ConnectionOwner RemoteOwnerT,
        ConnectionState LocalStateT = EmptyState, ConnectionState RemoteStateT = EmptyState
    >
    class ConnectionList : public impl::BasicConnectionList<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>
    {
        using base = impl::BasicConnectionList<LocalOwnerT, RemoteOwnerT, LocalStateT, RemoteStateT>;

      public:
        constexpr ConnectionList() noexcept {}

        ConnectionList(ConnectionList &&other) noexcept
        {
            base::MoveFromAndUpdateElemOwners(other);
        }

        ConnectionList &operator=(ConnectionList &&other) noexcept
        {
            base::MoveFromAndUpdateElemOwners(other); // This automatically does a self-assignment check.
            return *this;
        }
    };
}
