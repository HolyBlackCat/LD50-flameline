#pragma once

namespace States
{
    struct State
    {
        virtual void Tick() = 0;
        virtual void Render() const = 0;

        virtual ~State() {}
    };
}
