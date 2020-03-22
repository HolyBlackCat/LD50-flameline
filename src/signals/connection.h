#pragma once

#include <type_traits>
#include <utility>

#include "macros/maybe_const.h"
#include "program/errors.h"

namespace Sig
{
    // Use this as a `Connection` template parameter if no state is needed.
    struct None {};

    // Connection update cause.
    enum class Cause
    {
        unspecified,
        destroyed,
        remote_destroyed,
        moved,
        remote_moved,

        // When a non-null connection object is destroyed, it receives `OnUnbind(destroyed)`.
        // The corresponding remote receives `OnUnbind(remote_destroyed)`.

        // When a non-null object is moved from, it receives `OnUnbind(moved)`.
        // The object that's being moved into receives `OnBind(moved)`. The old connection
        // of this object, if any, is discarded with `OnUnbind(unspecified)`.
        // The remote receives `OnUnbind(remote_moved)`, followed by `OnBind(remote_moved)`.
    };


    // Common non-template base class for all connections.
    struct GenericConnection {};


    // Contains a pointer to an object of symmetric type `Connection<B,A>` (called a "remote").
    // The pointers are always symmetric. If the pointer is not null, then the pointer of remote points back to that object.
    // If the current object is destroyed, bound to a different object, or moved, the remote is notified and its pointer is adjusted accordingly.
    // This class is meant to be used together with macros from `macros/member_downcast.h`.
    // This class is an abstract base, use references to it for function parameters.
    // `Sig::Connection` inherits from this class, you probably want to use it. Inheriting custom classes from this one is not recommended.
    // You probably don't want to make instances of this class `const`, since when accessed through a remote connection, the constness will be ignored.
    template <typename A, typename B>
    class BasicConnection : public GenericConnection
    {
      public:
        using tag_t = A;
        using remote_tag_t = B;
        using remote_t = BasicConnection<B, A>;

      private:
        friend BasicConnection<B, A>;

        remote_t *remote = nullptr;

        // Note that those callbacks are called AFTER changing the connections. Because of that you
        // can e.g. get `OnUnbind` for a connection that's no longer null.
        //
        // When those callbacks are called, the related connections are already in a valid state.
        // But still, it's a bad idea to connect/disconnect them from the callbacks. If you do so, you might
        // receive the new callbacks in a weird order.
        //
        // The callbacks must not throw, doing so will call `std::terminate`.
        virtual void OnBind(Cause cause) noexcept = 0;
        virtual void OnUnbind(remote_t *old_remote, Cause cause) noexcept = 0;

        void AssertBound() const
        {
            DebugAssert("Attempt to use a null Sig::Connect.", IsBound());
        }

        // Destroys current binding, if any. Doesn't invoke callbacks.
        // Returns a pointer to the old remote, or 0 if none.
        remote_t *LowUnbindSilent()
        {
            if (!remote)
                return nullptr;

            DebugAssert("`Sig::Connection` consistency check failed.", this == remote->remote);

            remote_t *old_remote = remote;
            remote->remote = nullptr;
            remote = nullptr;
            return old_remote;
        }

        // Destroys current binding, if any.
        void LowUnbind(Cause cause, Cause remote_cause)
        {
            if (remote_t *old_remote = LowUnbindSilent())
            {
                OnUnbind(old_remote, cause);
                old_remote->OnUnbind(this, remote_cause);
            }
        }

        // Creates a new binding. Destroys old ones if necessary.
        // If `target` is null, acts as `LowUnbind`. If the `target == remote`, does nothing.
        void LowBind(remote_t *target, Cause unbind_old_remote_cause, Cause unbind_cause, Cause unbind_target_cause, Cause unbind_target_old_remote_cause, Cause bind_cause, Cause bind_target_cause)
        {
            if (target == remote)
                return;

            if (!target)
            {
                LowUnbind(unbind_cause, unbind_old_remote_cause);
                return;
            }

            remote_t *old_remote = LowUnbindSilent();
            BasicConnection *target_old_remote = target->LowUnbindSilent();

            remote = target;
            target->remote = this;

            if (old_remote)
            {
                OnUnbind(old_remote, unbind_cause);
                old_remote->OnUnbind(this, unbind_old_remote_cause);
            }
            if (target_old_remote)
            {
                target->OnUnbind(target_old_remote, unbind_target_cause);
                target_old_remote->OnUnbind(target, unbind_target_old_remote_cause);
            }
            OnBind(bind_cause);
            target->OnBind(bind_target_cause);
        }

      protected:
        // Derived classes must call this in move constructors.
        void MoveFrom(BasicConnection &other)
        {
            LowBind(other.remote, Cause::unspecified, Cause::unspecified, Cause::remote_moved, Cause::moved, Cause::moved, Cause::remote_moved);
        }

        // Derived classes must call this in destructors.
        void Destroy()
        {
            LowUnbind(Cause::destroyed, Cause::remote_destroyed);
        }

      public:
        BasicConnection() {}

        // We can't implement this, because it would have to call `MoveFrom(other);`,
        // which invokes callbacks. Since callbacks are virtual functions, they
        // don't work properly in the constructors of this class.
        // Derived classes should implement move constructor using `MoveFrom(...)`.
        BasicConnection(BasicConnection &&) = delete;

        // Not implemented for consistency with the move constructor.
        // Derived classes should implement this using `MoveFrom(...)`.
        // A self-assignment check is not necessary.
        BasicConnection &operator=(BasicConnection &&) = delete;

        ~BasicConnection()
        {
            DebugAssert("The destructor of the most derived class didn't call `this->Destroy()`.", !IsBound());
        }

        // If `target` is null, acts as `Unbind`. Otherwise acts as `Bind`.
        BasicConnection &BindPointer(remote_t *target)
        {
            LowBind(target, Cause::unspecified, Cause::unspecified, Cause::unspecified, Cause::unspecified, Cause::unspecified, Cause::unspecified);
            return *this;
        }

        // Binds this object to a remote one.
        // `a.Bind(b)` is equivalent to `b.Bind(a)`.
        // If any of the two objects are currently bound to something else, those bindings are discarded.
        BasicConnection &Bind(remote_t &target)
        {
            return BindPointer(&target);
        }

        // If the current object is bound to something, this binding is discarded.
        BasicConnection &Unbind()
        {
            LowUnbind(Cause::unspecified, Cause::unspecified);
            return *this;
        }


        [[nodiscard]] explicit operator bool() const {return IsBound();}
        [[nodiscard]] bool IsBound() const {return bool(remote);}
        [[nodiscard]] bool IsBoundTo(const remote_t &target) {return remote == &target;}

        MAYBE_CONST(
            // Returns a pointer to the bound object, or null if nothing is bound.
            [[nodiscard]] CV remote_t *RemotePointerOrNull() CV {return remote;}

            // Returns a reference to the bound object.
            // If no object is bound, an assertion is triggered.
            [[nodiscard]] CV remote_t &Remote() CV {AssertBound(); return *remote;}

            // Downcasts (using `static_cast`) this object to `T`.
            template <typename T>
            [[nodiscard]] CV T &UnsafeDowncast() CV
            {
                static_assert(!std::is_const_v<T> && std::is_base_of_v<BasicConnection, T>);
                return *static_cast<CV T *>(this);
            }
        )
    };


    // Provides connection callback types.
    // Also serves as a base class of `ConnectionCallbacks`.
    template <typename A, typename B>
    struct BasicConnectionCallbacks
    {
        // `noexcept` is not strictly necessary here, because virtual callback functions
        // from `BasicConnection` are noexcept anyway. But it forces user to declare their
        // callbacks as `noexcept`, which helps to remind them that they must not throw.
        using bind_t = void(BasicConnection<A, B> &object, Cause cause) noexcept;
        using unbind_t = void(BasicConnection<A, B> &object, BasicConnection<B, A> *old_remote, Cause cause) noexcept;
    };

    // Encapsulates bind/unbind callbacks for connections.
    // `Bind` and/or `Unbind` can be null, then they're ignored.
    // Callbacks must be noexcept.
    template <typename A, typename B, typename BasicConnectionCallbacks<A,B>::bind_t *Bind, typename BasicConnectionCallbacks<A,B>::unbind_t *Unbind>
    struct ConnectionCallbacks : BasicConnectionCallbacks<A, B>
    {
        static constexpr auto OnBind = Bind;
        static constexpr auto OnUnbind = Unbind;
    };


    // A non-abstract class derived from `Sig::BasicConnection`. See `Sig::BasicConnection` for usage.
    // `Callbacks` is 0 or more `ConnectionCallbacks<A, B, Bind, Unbind>`.
    template <typename A, typename B, typename ...Callbacks>
    class Connection final : public BasicConnection<A, B>
    {
        static_assert(((std::is_base_of_v<BasicConnectionCallbacks<A, B>, Callbacks> && !std::is_same_v<BasicConnectionCallbacks<A, B>, Callbacks>) && ...),
                      "Each of `Callbacks` must be `ConnectionCallbacks<A, B, ...>`.");

        virtual void OnBind(Cause cause) noexcept override
        {
            ((Callbacks::OnBind ? Callbacks::OnBind(*this, cause) : void()), ...);
        }
        virtual void OnUnbind(BasicConnection<B, A> *old_remote, Cause cause) noexcept override
        {
            ((Callbacks::OnUnbind ? Callbacks::OnUnbind(*this, old_remote, cause) : void()), ...);
        }

      public:
        using BasicConnection<A, B>::BasicConnection;

        Connection(Connection &&other)
        {
            // Every non-abstract class that inherits from `BasicConnection` has to do this.
            this->MoveFrom(other);
        }
        Connection &operator=(Connection &&other)
        {
            // Every non-abstract class that inherits from `BasicConnection` has to do this.
            this->MoveFrom(other);
            return *this;
        }

        ~Connection()
        {
            // This line is important!
            // Every non-abstract class that inherits from `BasicConnection` has to have
            // it in its destructor, even if its parent already has it.
            this->Destroy();
        }

        // The base class.
        using basic_connection_t = BasicConnection<A, B>;


        // Adds a pair of callbacks after the existing ones, returns the modified type.
        template <typename BasicConnectionCallbacks<A,B>::bind_t *Bind, typename BasicConnectionCallbacks<A,B>::unbind_t *Unbind>
        using with_callbacks_after = Connection<A, B, Callbacks..., ConnectionCallbacks<A, B, Bind, Unbind>>;

        // Adds a pair of callbacks before the existing ones, returns the modified type.
        template <typename BasicConnectionCallbacks<A,B>::bind_t *Bind, typename BasicConnectionCallbacks<A,B>::unbind_t *Unbind>
        using with_callbacks_before = Connection<A, B, ConnectionCallbacks<A, B, Bind, Unbind>, Callbacks...>;
    };

    // Alternative to `x.Bind(y)`. Returns a reference to `x`.
    template <typename A, typename B>
    BasicConnection<A, B> &Bind(BasicConnection<A, B> &x, BasicConnection<B, A> &y)
    {
        x.Bind(y);
        return x;
    }
}

// Set to `true` to enable automatic tests.
#if false

#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

namespace Sig
{
    namespace Test
    {
        inline std::optional<std::stringstream> ss;
        inline std::map<const void *, std::string> var_names;

        std::string GetVarName(const void *ptr)
        {
            if (!ptr)
                return "0";

            auto it = var_names.find(ptr);
            if (it == var_names.end())
                return "??";
            else
                return it->second;
        }

        std::string CauseToString(Cause cause)
        {
            switch (cause)
            {
                case Cause::unspecified: return "-";
                case Cause::destroyed: return "d";
                case Cause::remote_destroyed: return "rd";
                case Cause::moved: return "m";
                case Cause::remote_moved: return "rm";
            }
            return "??";
        }


        template <typename A, typename B> void OnBind(BasicConnection<A, B> &object, Cause cause) noexcept
        {
            *ss << GetVarName(&object) << ".OnBind(" << CauseToString(cause) << ")\n";
        }
        template <typename A, typename B> void OnUnbind(BasicConnection<A, B> &object, BasicConnection<B, A> *old_remote, Cause cause) noexcept
        {
            *ss << GetVarName(&object) << ".OnUnbind(" << GetVarName(old_remote) << "," << CauseToString(cause) << ")\n";
        }


        struct TagA {};
        struct TagB {};

        using ConA = Connection<TagA, TagB>::with_callbacks_after<OnBind, OnUnbind>;
        using ConB = Connection<TagB, TagA>::with_callbacks_after<OnBind, OnUnbind>;

        void RunTestCase(std::string name, bool (*func)(), std::string result)
        {
            ss.emplace();
            var_names.clear();
            std::cout << ":: Running Sig::Connection test '" << name << "'... " << std::flush;
            bool ok = func();
            if (!ok)
            {
                std::cout << "NOT OK\n";
                Program::Exit(1);
            }
            if (ss->str() == result)
            {
                std::cout << "OK\n";
                return;
            }

            std::cout << "NOT OK\nOutput:\n" << ss->str();
            Program::Exit(1);
        }

        void TestConnections()
        {
            RunTestCase("bind_unbind", []{
                ConA a;
                ConB b;
                var_names.emplace(&a, "a");
                var_names.emplace(&b, "b");

                a.Bind(b);

                if (!a.IsBoundTo(b)) return false;
                *ss << "---\n";

                a.Unbind();
                return true;

            },
                "a.OnBind(-)\n"
                "b.OnBind(-)\n"
                "---\n"
                "a.OnUnbind(b,-)\n"
                "b.OnUnbind(a,-)\n"
            );

            RunTestCase("bind_destroy", []{
                ConA a;
                ConB b;
                var_names.emplace(&a, "a");
                var_names.emplace(&b, "b");

                a.Bind(b);
                if (!a.IsBoundTo(b)) return false;
                *ss << "---\n";

                return true;

            },
                "a.OnBind(-)\n"
                "b.OnBind(-)\n"
                "---\n"
                "b.OnUnbind(a,d)\n"
                "a.OnUnbind(b,rd)\n"
            );

            RunTestCase("move_a", []{
                ConA a, aa;
                ConB bb;
                var_names.emplace(&a, "a");
                var_names.emplace(&aa, "aa");
                var_names.emplace(&bb, "bb");

                aa.Bind(bb);
                *ss << "---\n";
                a = std::move(aa);
                if (!a.IsBoundTo(bb)) return false;
                *ss << "---\n";

                return true;

            },
                "aa.OnBind(-)\n"
                "bb.OnBind(-)\n"
                "---\n"
                "bb.OnUnbind(aa,rm)\n"
                "aa.OnUnbind(bb,m)\n"
                "a.OnBind(m)\n"
                "bb.OnBind(rm)\n"
                "---\n"
                "bb.OnUnbind(a,d)\n"
                "a.OnUnbind(bb,rd)\n"
            );

            RunTestCase("move_b", []{
                ConA a, aa;
                ConB b, bb;
                var_names.emplace(&a, "a");
                var_names.emplace(&aa, "aa");
                var_names.emplace(&b, "b");
                var_names.emplace(&bb, "bb");

                a.Bind(b);
                aa.Bind(bb);
                *ss << "---\n";
                a = std::move(aa);
                if (!a.IsBoundTo(bb)) return false;
                *ss << "---\n";

                return true;

            },
                "a.OnBind(-)\n"
                "b.OnBind(-)\n"
                "aa.OnBind(-)\n"
                "bb.OnBind(-)\n"
                "---\n"
                "a.OnUnbind(b,-)\n"
                "b.OnUnbind(a,-)\n"
                "bb.OnUnbind(aa,rm)\n"
                "aa.OnUnbind(bb,m)\n"
                "a.OnBind(m)\n"
                "bb.OnBind(rm)\n"
                "---\n"
                "bb.OnUnbind(a,d)\n"
                "a.OnUnbind(bb,rd)\n"
            );

            RunTestCase("complex_bind", []{
                ConA a, aa;
                ConB b, bb;
                var_names.emplace(&a, "a");
                var_names.emplace(&aa, "aa");
                var_names.emplace(&b, "b");
                var_names.emplace(&bb, "bb");

                a.Bind(b);
                aa.Bind(bb);
                *ss << "---\n";
                a.Bind(bb);
                if (!a.IsBoundTo(bb)) return false;
                *ss << "---\n";

                return true;

            },
                "a.OnBind(-)\n"
                "b.OnBind(-)\n"
                "aa.OnBind(-)\n"
                "bb.OnBind(-)\n"
                "---\n"
                "a.OnUnbind(b,-)\n"
                "b.OnUnbind(a,-)\n"
                "bb.OnUnbind(aa,-)\n"
                "aa.OnUnbind(bb,-)\n"
                "a.OnBind(-)\n"
                "bb.OnBind(-)\n"
                "---\n"
                "bb.OnUnbind(a,d)\n"
                "a.OnUnbind(bb,rd)\n"
            );
        }

        [[maybe_unused]] static int unused = (TestConnections(), 0);
    }
}
#endif
