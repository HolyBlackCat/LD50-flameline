#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#include "reflection/full_with_poly.h"

namespace GameUtils::State
{
    class NextStateSelector;

    REFL_STRUCT( BasicState REFL_POLYMORPHIC )
    {
        BasicState() = default;
        BasicState(BasicState &&) = default;
        BasicState &operator=(BasicState &&) = default;

        // This should called once when the state is constructed.
        // `params` is a user-provided string, or an empty one if it wasn't provided.
        virtual void Init(const std::string &params) {(void)params;}

        virtual void Tick(const NextStateSelector &next_state) = 0;
        virtual void Render() const = 0;
    };
    using Storage = Refl::PolyStorage<BasicState>;

    class Tag
    {
        std::size_t index = -1;

      public:
        Tag() {}

        // Throws on failure.
        // WARNING: Do not invoke this before `main`. At that point states might not have been registered yet.
        Tag(const char *name) : index(Refl::Polymorphic::NameToIndex<BasicState>(name)) {}
        Tag(const std::string &name) : index(Refl::Polymorphic::NameToIndex<BasicState>(name)) {}

        [[nodiscard]] explicit operator bool() const {return index != std::size_t(-1);}

        // Returns the state index, or `-1` if the tag is null.
        [[nodiscard]] std::size_t Index() const {return index;}
    };

    [[nodiscard]] inline Storage Construct(Tag tag, const std::string &params = {})
    {
        Storage ret = Refl::Polymorphic::ConstructFromIndex<BasicState>(tag.Index());
        ret->Init(params);
        return ret;
    }


    class NextStateSelector
    {
        mutable bool is_set = false;
        mutable Tag tag;
        mutable std::string params;

      public:
        NextStateSelector() {}

        // Sets the next state.
        // If you pass a null tag, the next state will be null.
        // Note that this function is `const` because we pass `NextStateSelector` to `Tick()` by a const reference.
        void Set(Tag new_tag, std::string new_params = std::string{}) const
        {
            is_set = true;
            tag = new_tag;
            params = std::move(new_params);
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
            params = std::string{};
        }

        // Non-const because we don't want `Tick()` to call it.
        [[nodiscard]] Storage ConstructStateAndReset()
        {
            if (!is_set)
                return nullptr;

            Storage ret;

            // If `tag` is null, `ret` remains null.
            if (tag)
                ret = Construct(tag, params);

            Reset();
            return ret;
        }
    };


    class StateManager
    {
        Storage state;
        NextStateSelector next_state;

      public:
        StateManager() {}

        [[nodiscard]] explicit operator bool() const
        {
            return bool(state);
        }

        [[nodiscard]] const NextStateSelector &NextState()
        {
            return next_state;
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
