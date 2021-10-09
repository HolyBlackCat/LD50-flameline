#pragma once

#include <string_view>
#include <string>
#include <type_traits>

#include "macros/maybe_const.h"
#include "reflection/full_with_poly.h"

namespace GameUtils::State
{
    class Selector;

    // You must inherit your custom base for states from this class.
    // It must be `REFL_POLYMORPHIC`.
    struct Base
    {
        Base() {}
        Base(const Base &) = default;
        Base &operator=(const Base &) = default;
        virtual ~Base() {}

        // This will be called once, right after the state is constructed and after its fields are deserialized.
        virtual void Init() {}

        // `next_state` is empty by default, assign to it to change state.
        // It will be deserialized as `Refl::PolyStorage<T>`, e.g. `MyState{}` or `MyState{foo=1}`.
        // Remember that classes with unnamed fields use `()` instead of `{}`.
        // Use `"0"` to set a null state.
        virtual void Tick(std::string &next_state) = 0;
    };

    // Manages a state.
    template <typename T>
    requires(std::is_base_of_v<Base, T> && !std::is_same_v<Base, T> && Refl::Class::explicitly_polymorphic<T>)
    class Manager
    {
        Refl::PolyStorage<T> state;
        std::string next_state;

      public:
        Manager() {}

        // Returns true if the state is not null.
        // If this returns false, you probably should stop the main loop.
        [[nodiscard]] explicit operator bool() const
        {
            return bool(state);
        }

        MAYBE_CONST(
            [[nodiscard]] auto Call(auto member, auto &&... params) CV
            {
                if (state)
                    return (state.base().*member)(decltype(params)(params)...);
                else
                    return decltype((state.base().*member)(decltype(params)(params)...))();
            }
        )

        void SetState(std::string_view state_str)
        {
            Refl::FromString(state, state_str);
            if (state)
                state->Init();
        }

        void Tick()
        {
            // Note that we change state right before `state->Tick()`.
            // This way, you can never have a state that wasn't `Tick`ed yet (e.g. you can't accidentally render it).
            if (!next_state.empty())
            {
                SetState(next_state);
                next_state.clear();
            }

            if (state)
                state->Tick(next_state);
        }
    };
}
