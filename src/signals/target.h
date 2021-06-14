#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include "signals/interface.h"

namespace Sig
{
    template <typename> class Pointer;

    namespace impl
    {
        // This is a CRTP base.
        template <typename D>
        class Target
        {
            static_assert(std::is_class_v<D> && !std::is_const_v<D> && !std::is_volatile_v<D>, "The template parameter must be a cv-unqualified class.");

            friend Pointer<D>;

            struct State
            {
                D *pointer = nullptr;
            };

            // If this is not null, then `state->pointer` is guaranteed to not be null.
            std::weak_ptr<State> state;

          public:
            constexpr Target() noexcept {}

            Target(Target &&other) noexcept
                : state(std::move(other.state))
            {
                if (auto ptr = state.lock())
                    ptr->pointer = this;
            }

            Target &operator=(Target &&other) noexcept
            {
                state = std::move(other.state);
                if (auto ptr = state.lock())
                    ptr->pointer = this;
                return *this;
            }

            ~Target()
            {
                UnbindAll();
            }

            // Returns true if at least one pointer is bound to this target.
            [[nodiscard]] explicit operator bool() const
            {
                return !state.expired();
            }

            // Returns the number of pointers bound to this target.
            [[nodiscard]] std::size_t BoundPointerCount() const
            {
                return state.use_count();
            }

            // Unbinds all pointers bound to this target.
            void UnbindAll() noexcept
            {
                auto ptr = state.lock();
                if (!ptr)
                    return;
                ptr->pointer = nullptr;
                state.reset();
            }
        };
    }

    // This is a CRTP base.
    // The actual implementation is in the private base.
    // Apply `Interface()` to classes derived from this to access the interface.
    template <typename D>
    class Target : impl::Target<D>
    {
        friend impl::InterfaceHelper;
        using interface_helper = impl::InterfaceHelper;
        using interface_type = impl::Target<D>;
    };

    // Observers a class derived from `Target<D>`.
    template <typename D>
    class Pointer
    {
        // No assertions for `D` here, because `impl::Target<D>` already checks it.
        using State = typename InterfaceType<Target<D>>::State;

        // Unlike in `class Target`, this CAN point to a state storing a null pointer.
        // This is `mutable` because some const functions can reset it to null if they detect this state.
        mutable std::shared_ptr<State> state;

      public:
        constexpr Pointer(std::nullptr_t = nullptr) noexcept {}

        // Attaches the pointer to a target.
        // This is not `noexcept` solely because it can perform heap allocation.
        void Bind(Target<D> &target)
        {
            impl::Target<D> &target_interface = Interface(target);
            state = target_interface.state.lock();
            if (state)
                return;
            state = std::make_shared<State>();
            state->pointer = static_cast<D *>(&target);
            target_interface.state = state;
        }

        // Detaches the pointer from the target.
        void Unbind() noexcept
        {
            state = nullptr;
        }

        // Returns true if a remote object is bound to this one.
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return bool(state);
        }

        // Returns the bound remote object, or null if not bound.
        [[nodiscard]] D *GetTarget() noexcept
        {
            return const_cast<D *>(std::as_const(*this).GetTarget());
        }

        [[nodiscard]] const D *GetTarget() const noexcept
        {
            if (!state)
                return nullptr;
            if (!state->pointer)
            {
                state = nullptr;
                return nullptr;
            }
            return state->pointer;
        }

        // Returns the bound remote object, might crash if none is bound.
        [[nodiscard]]       D &operator*()       {return *GetTarget();}
        [[nodiscard]] const D &operator*() const {return *GetTarget();}

        [[nodiscard]]       D *operator->()       {return GetTarget();}
        [[nodiscard]] const D *operator->() const {return GetTarget();}

        // Cleans up any memory lingering after the remote object broke the connection.
        // `GetTarget` and operators `*`,`->` do this automatically, even on const objects.
        // For the memory to actually be freed, all pointers to the detached remote must do this action.
        void Prune() const
        {
            if (state && !state->pointer)
                state = nullptr;
        }
    };
}
