#pragma once

#include <cstddef>
#include <type_traits>

#include "meta/check_type_hash.h"
#include "meta/misc.h"
#include "signals/connection.h"

namespace Sig
{
    namespace impl
    {
        template <typename T>
        struct ConnectionStateWrapper
        {
            ConnectionStateWrapper() {}

            STORE_TYPE_HASH
            T value;
        };
    }

    // Use this as a template parameter of `ConnectionWithState` if you need no state.
    struct EmptyState {};

    // Stores a `Sig::Connection` (`C`), and an arbitrary state (`S`).
    // Can be used instead of `MEMBER_DOWNCAST` in simple cases.
    // Also useful to implement consistent handling of remote `Sig::Connection` and `Sig::ConnectionList` objects.
    template <typename C, typename S>
    struct ConnectionWithState
    {
        static_assert(std::is_base_of_v<GenericConnection, C>, "`C` must be a specialization of `Sig::Connection`.");

        using basic_connection_t = typename C::basic_connection_t;
        static_assert(sizeof(C) == sizeof(basic_connection_t) && alignof(C) == alignof(basic_connection_t), "`C` must not have any additional state."); // This shouldn't happen if `C` is `Sig::Connection`.

        C connection;
        [[no_unique_address]] impl::ConnectionStateWrapper<S> state;
    };

    // Given a reference to a connection stored in a `Sig::ConnectionWithState<..., S>`,
    // returns a reference to the state that's stored alongside it.
    // If the connection is NOT stored in a `ConnectionWithState`, or if the specified state type is incorrect,
    // you either trigger an assertion (in debug builds, using `Meta::AssertTypeHash`) or get UB (in release builds).
    template <typename S, Meta::deduce..., typename A, typename B>
    [[nodiscard]] S &GetConnectionState(BasicConnection<A, B> &connection)
    {
        char *connection_ptr = reinterpret_cast<char *>(&connection);

        // We expect `ConnectionWithState<Connection<A,B>, S>` and `ConnectionWithState<???, S>` to have `state` at the same offset.
        using offset_calc_helper = ConnectionWithState<Connection<A, B>, S>;
        constexpr auto offset = std::ptrdiff_t(offsetof(offset_calc_helper, state)) - std::ptrdiff_t(offsetof(offset_calc_helper, connection));

        auto &wrapper = *reinterpret_cast<impl::ConnectionStateWrapper<S> *>(connection_ptr + offset);
        Meta::AssertTypeHash(wrapper);

        return wrapper.value;
    }
    template <typename S, Meta::deduce..., typename A, typename B>
    [[nodiscard]] const S &GetConnectionState(const BasicConnection<A, B> &connection)
    {
        return GetConnectionState<S>(const_cast<BasicConnection<A, B> &>(connection));
    }
}
