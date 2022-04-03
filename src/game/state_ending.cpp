#include "main.h"

#include "game/sounds.h"

namespace States
{
    STRUCT( Ending EXTENDS StateBase )
    {
        MEMBERS(
            DECL(fvec3) bg_color
            DECL(float) vignette_alpha
            DECL(int) cur_secrets
            DECL(int) max_secrets
            DECL(int) time
            DECL(int) time_sub
        )

        int timer = 0;

        float exit_fade = 0;
        std::optional<std::string> queued_state;

        bool FoundAllSecrets() const
        {
            return cur_secrets >= max_secrets;
        }

        std::string TicksToTime(int t) const
        {
            constexpr int second = 60, minute = second * 60, hour = minute * 60;

            int hours = t / hour;
            t %= hour;
            int minutes = t / minute;
            t %= minute;
            int seconds = t / second;
            t %= second;

            int millis = clamp(t / 60.f * 1000, 0, 999);

            if (hours == 0)
                return FMT("{}:{:02d}.{:03d}", minutes, seconds, millis);
            else
                return FMT("{}:{:02d}:{:02d}.{:03d}", hours, minutes, seconds, millis);
        }

        void Tick(std::string &next_state) override
        {
            if (timer > 450 && !queued_state && Input::Button().AssignKey())
            {
                if (FoundAllSecrets())
                    queued_state = "0";
                else
                    queued_state = "World{}";
                Sounds::jump();
            }

            if (queued_state)
            {
                clamp_var_max(exit_fade += 0.01f);
                if (exit_fade > 0.999f)
                    next_state = *queued_state;
            }

            timer++;
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            constexpr fvec3
                text_color = fvec3(3, 16, 109) / 255,
                alt_text_color = fvec3(73, 161, 243) / 255;

            r.iquad(ivec2(), screen_size).center().color(bg_color);

            std::string_view main_text = "Thanks for playing!";
            r.itext(ivec2(-Graphics::Text(Fonts::main, main_text).ComputeStats().size.x / 2, -32), Graphics::Text(Fonts::main, main_text.substr(0, clamp_min((timer - 60) / 10)))).color(text_color).align_x(-1);

            std::string secrets_text =
                cur_secrets == 0 ? FMT("You didn't find any secrets though...") :
                cur_secrets < max_secrets ? FMT("You found {}/{} secrets, keep looking...", cur_secrets, max_secrets) : "You found all the secrets. Great job!";

            r.itext(ivec2(0, 8), Graphics::Text(Fonts::main, secrets_text)).color(alt_text_color).alpha(smoothstep(clamp((timer - 300) / 60.f)));

            r.itext(ivec2(0, -screen_size.y/2 + 8), Graphics::Text(Fonts::main, FMT("Real time: {}      Game world time: {}", TicksToTime(time), TicksToTime(time_sub)))).color(alt_text_color).alpha(smoothstep(clamp((timer - 360) / 60.f)));

            r.itext(ivec2(0, screen_size.y/2 - 18), Graphics::Text(Fonts::main, FoundAllSecrets() ? "Press any key to quit!" : "Press any key to restart!")).color(alt_text_color).alpha(smoothstep(clamp((timer - 420) / 60.f)));

            { // Exit fade.
                if (exit_fade > 0)
                    r.iquad(ivec2(), screen_size).center().color(fvec3(0)).alpha(smoothstep(exit_fade));
            }

            { // Vignette.
                static const auto &region = texture_atlas.Get("vignette.png");
                    r.iquad(ivec2(), region).alpha(vignette_alpha).center();
            }

            r.Finish();
        }
    };
}
