#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#include "reflection/full_with_poly.h"

namespace State
{
    class NextState;

    REFL_STRUCT( BasicState REFL_POLYMORPHIC )
    {
        virtual void Tick(const NextState &next_state) = 0;
        virtual void Render() const = 0;
    };
    using Storage = Refl::PolyStorage<BasicState>;

    class Tag
    {
        std::size_t index = -1;

      public:
        Tag() {}

        // Throws on failure.
        // WARNING: Do not invoke this before `main`. At that point states might not be registered yet.
        Tag(const char *name) : index(Refl::Polymorphic::NameToIndex<BasicState>(name)) {}
        Tag(const std::string &name) : index(Refl::Polymorphic::NameToIndex<BasicState>(name)) {}

        [[nodiscard]] explicit operator bool() const {return index != std::size_t(-1);}

        // Returns the state index, or `-1` if the tag is null.
        [[nodiscard]] std::size_t Index() const {return index;}
    };

    [[nodiscard]] inline Storage Construct(Tag tag)
    {
        return Refl::Polymorphic::ConstructFromIndex<BasicState>(tag.Index());
    }


    class NextState
    {
        mutable bool is_set = false;
        mutable Tag tag;

        using func_t = std::function<void(Storage &storage)>;
        mutable func_t func;

      public:
        NextState() {}

        // Sets the next state.
        // `new_func` is saved, and is later called on the new state after it's constructed.
        // It's `const` because we pass `NextState` to `Tick()` by a const reference.
        void Set(Tag new_tag, func_t new_func = nullptr) const
        {
            is_set = true;
            tag = new_tag;
            func = std::move(new_func);
        }

        // Non-const because we don't want `Tick()` to call it.
        [[nodiscard]] bool IsSet()
        {
            return is_set;
        }

        void Reset()
        {
            is_set = false;
            tag = Tag{};
            func = nullptr;
        }

        // Non-const because we don't want `Tick()` to call it.
        [[nodiscard]] Storage ConstructStateAndReset()
        {
            if (!is_set)
                return nullptr;

            Storage ret;

            if (tag)
            {
                ret = Construct(tag);
                if (func)
                    func(ret);
            }

            Reset();
            return ret;
        }
    };


    class StateManager
    {
        Storage state;
        NextState next_state;

      public:
        StateManager() {}

        [[nodiscard]] explicit operator bool() const
        {
            return bool(state);
        }

        template <typename T, typename ...P> T &SetState(P &&... params)
        {
            next_state.Reset();
            return state.make<T>(std::forward<P>(params)...);
        }

        void SetState(Tag tag)
        {
            next_state.Set(tag);
        }

        void Tick()
        {
            // Note that we call this before `state->Tick()`.
            // This way, we'll never reach `Render()` with a state that wasn't `Tick`ed yet.
            if (next_state.IsSet())
                state = next_state.ConstructStateAndReset();

            if (state)
                state->Tick(next_state);
        }

        void Render()
        {
            if (state)
                state->Render();
        }
    };
}
