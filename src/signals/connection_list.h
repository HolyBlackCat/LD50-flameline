#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "macros/finally.h"
#include "macros/maybe_const.h"
#include "macros/member_downcast.h"
#include "meta/misc.h"
#include "signals/connection.h"
#include "signals/connection_state.h"

namespace Sig
{
    // Stores a list of `Sig::Connection`s.
    // The connections are automatically removed from the list if they lose their binding.
    // Each connection is wrapped in `ConnectionWithState<State, ...>`, so `GetConnectionState` can be applied to them (see `signals/connection_state.h`).
    // You probably don't want to make instances of this class `const`, since when accessed through a remote connection, the constness will be ignored.
    template <typename OriginalConnection, typename State = EmptyState>
    class ConnectionList
    {
        static_assert(Meta::is_specialization_of<OriginalConnection, Connection>, "The template parameter must be a specialization of `Sig::Connection`.");

      public:
        using original_connection_t = OriginalConnection;
        using basic_connection_t = typename OriginalConnection::basic_connection_t;

        using tag_t        = typename OriginalConnection::tag_t;
        using remote_tag_t = typename OriginalConnection::remote_tag_t;
        using remote_t     = typename OriginalConnection::remote_t;

        using state_t = State;

      private:
        static void OnUnbind_Callback(BasicConnection<tag_t, remote_tag_t> &object, remote_t *old_remote, Cause cause) noexcept
        {
            (void)old_remote;
            (void)cause;

            if (object.IsBound())
                return; // This can happen if the remote is moved, or if the binding of this connection was changed.

            using Entry = typename Data::Entry;
            Entry &entry = MEMBER_DOWNCAST(Entry, con_with_state.connection, static_cast<custom_connection_t &>(object));
            Data &data = *entry.data_ptr;

            if (data.CurrentlyResizingConnectionList())
                return; // This means we got this callback because the connection vector is in the process of being resized.

            data.RemoveConnection(data.GetConnectionList().begin() + (&entry - data.GetConnectionList().data()));
        }

        using custom_connection_t = typename OriginalConnection::template with_callbacks_after<nullptr, OnUnbind_Callback>;

        class Data
        {
          public:
            struct Entry
            {
                STORE_TYPE_HASH
                ConnectionWithState<State, custom_connection_t> con_with_state;
                Data *data_ptr = nullptr;

                Entry(remote_t &remote)
                {
                    con_with_state.connection.Bind(remote);
                }
            };

          private:
            std::vector<Entry> connections;
            bool currently_resizing_connections = false; // This MUST be set to true when `connections` is being resized.

          public:
            ConnectionList *main_class_ptr = nullptr;

            Data(ConnectionList *main_class) : main_class_ptr(main_class) {}

            // This class is non-copyable and non-movable.
            // It not being movable makes the code easier, since we don't have to update
            // pointers in each entry on move.
            // We wrap it in `std::unique_ptr` to make the `ConnectionList` movable.
            Data(const Data &) = delete;
            Data &operator=(const Data &) = delete;

            void InsertConnection(remote_t &remote)
            {
                currently_resizing_connections = true;
                FINALLY( currently_resizing_connections = false; )
                connections.emplace_back(remote).data_ptr = this;
            }

            void RemoveConnection(typename std::vector<Entry>::const_iterator iter)
            {
                currently_resizing_connections = true;
                FINALLY( currently_resizing_connections = false; )
                connections.erase(iter);
            }

            const std::vector<Entry> &GetConnectionList() const
            {
                return connections;
            }

            bool CurrentlyResizingConnectionList() const
            {
                return currently_resizing_connections;
            }
        };
        std::unique_ptr<Data> data;

        static typename Data::Entry &DowncastConnectionToEntry(basic_connection_t &connection)
        {
            auto &custom_con = static_cast<custom_connection_t &>(connection);
            using Entry = typename Data::Entry; // `MEMBER_DOWNCAST` doesn't like `typename`, so we need the alias.
            return MEMBER_DOWNCAST(Entry, con_with_state.connection, custom_con);
        }

      public:
        ConnectionList() {}

        ConnectionList(ConnectionList &&other) noexcept : data(std::move(other.data))
        {
            if (data)
                data->main_class_ptr = this;
        }

        ConnectionList &operator=(ConnectionList &&other) noexcept
        {
            data = std::move(other.data);
            if (data)
                data->main_class_ptr = this;
            return *this;
        }

        ~ConnectionList() {}

        // Given a reference to a connection stored in a list, returns a reference to the list.
        // If the connection is not stored in a list you either trigger an assertion (in debug builds) or get UB (in release builds).
        [[nodiscard]] static ConnectionList &DowncastConnectionToList(basic_connection_t &connection)
        {
            return *DowncastConnectionToEntry(connection).data_ptr->main_class_ptr;
        }

        // Adds a connection to the list.
        void InsertConnection(remote_t &remote)
        {
            if (!data)
                data = std::make_unique<Data>(this);

            data->InsertConnection(remote);
        }

        // Removes a connection already existing in the list.
        // Make sure the pointer is valid!
        void RemoveConnection(const basic_connection_t &con)
        {
            const auto &entry = DowncastConnectionToEntry(const_cast<basic_connection_t &>(con));
            data->RemoveConnection(data->GetConnectionList().begin() + (&entry - data->GetConnectionList().data()));
        }

        [[nodiscard]] std::size_t ConnectionCount() const
        {
            return data ? data->GetConnectionList().size() : 0;
        }

        [[nodiscard]] bool Empty() const
        {
            return ConnectionCount() == 0;
        }

        void Clear()
        {
            data = nullptr;
        }

        MAYBE_CONST(
            // `func` is `bool func(CV basic_connection_t &)`.
            // If it returns false, the loop stops and the function also returns `false`.
            template <typename F>
            bool ForEachConnection(F &&func) CV
            {
                if (data)
                {
                    for (const auto &entry : data->GetConnectionList())
                    {
                        // Note that we have to strip constness from `entry` here.
                        // `GetConnectionList()` returns a const reference (because the vector must never be resized manually),
                        // but stripping constness from specific vector elements is fine.
                        using cv_entry_t = CV typename Data::Entry;
                        auto &con = static_cast<CV basic_connection_t &>(const_cast<cv_entry_t &>(entry).con_with_state.connection);
                        if (bool(func(con)) == false)
                            return false;
                    }
                }

                return true;
            }
        )
    };

    template <typename C, typename S, typename A, typename B>
    void Bind(ConnectionList<C, S> &list, BasicConnection<A, B> &con)
    {
        list.InsertConnection(con);
    }
    template <typename C, typename S, typename A, typename B>
    void Bind(BasicConnection<A, B> &con, ConnectionList<C, S> &list)
    {
        list.InsertConnection(con);
    }
}
