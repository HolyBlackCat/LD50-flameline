#pragma once

#include <exception>
#include <string>
#include <utility>

#include "interface/gui.h"
#include "interface/messagebox.h"
#include "program/errors.h"
#include "reflection/full.h"
#include "stream/readonly_data.h"
#include "stream/save_to_file.h"
#include "strings/format.h"
#include "utils/mat.h"

template <typename T> class Config
{
    T object = {};

    std::string file_name;

    bool was_modified_in_gui = 0;

    template <typename X> void DisplayGuiLow(X &object, std::string name)
    {
        if constexpr (std::is_same_v<X, int>)
        {
            if (ImGui::InputInt(name.c_str(), &object))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, ivec2>)
        {
            if (ImGui::InputInt2(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, ivec3>)
        {
            if (ImGui::InputInt3(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, ivec4>)
        {
            if (ImGui::InputInt4(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, float>)
        {
            if (ImGui::InputFloat(name.c_str(), &object))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, fvec2>)
        {
            if (ImGui::InputFloat2(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, fvec3>)
        {
            if (ImGui::InputFloat3(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, fvec4>)
        {
            if (ImGui::InputFloat4(name.c_str(), &object.x))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, double>)
        {
            if (ImGui::InputDouble(name.c_str(), &object))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, std::string>)
        {
            if (ImGui::InputTextMultiline(name.c_str(), &object))
                was_modified_in_gui = 1;
        }
        else if constexpr (std::is_same_v<X, bool>)
        {
            if (ImGui::Checkbox(name.c_str(), &object))
                was_modified_in_gui = 1;
        }
        else if constexpr (Refl::Class::members_known<X>)
        {
            if (ImGui::CollapsingHeader(name.c_str()))
            {
                ImGui::Indent();
                Meta::cexpr_for<Refl::Class::member_count<X>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    static_assert(Refl::Class::member_names_known<X>);
                    DisplayGuiLow(Refl::Class::Member<i>(object), Refl::Class::MemberName<X>(i));
                });
                ImGui::Unindent();
            }
        }
        else
        {
            ImGui::TextDisabled("%s", name.c_str());
        }
    }

  public:
    Config() {}

    explicit Config(const T &object) : object(object) {}
    explicit Config(T &&object) : object(std::move(object)) {}

    Config(const std::string &file_name) : file_name(file_name)
    {
        Stream::ReadOnlyData file;

        // Try loading the file;
        try
        {
            file = Stream::ReadOnlyData(file_name);
        }
        catch (...) {}

        if (file)
        {
            // The file was opened successfully, now we parse it.
            try
            {
                // Try parsing normally.
                Refl::FromString(object, file);

                // Success, we're done.
                return;
            }
            catch (std::exception &e)
            {
                // Normal parsing failed, disabling completeness check and trying again.
                // We probably don't need to reset `object` to the default state.

                bool partial_parsing_ok = 0;
                try
                {
                    Refl::FromStringOptions opt;
                    opt.ignore_missing_fields = true;
                    Refl::FromString(object, file, opt);
                    partial_parsing_ok = 1;
                }
                catch (...) {}

                if (!partial_parsing_ok)
                {
                    // Parsing without completeness check failed as well. Throwing the original exception.
                    Program::Error("Unable to parse config file `", file_name, "`.\nDelete the file if you want it to be regenerated.\n", e.what());
                }

                // The file is parsed successfully, but it's incomplete. Proceeding to making a new file.
            }
        }

        // We can't open the file, unable to parse the file, or file is incomplete.
        // Creating a new file.

        Save();
    }

    const T &operator*() const {return object;}
    const T *operator->() const {return &object;}

    void Reload()
    {
        try
        {
            *this = Config(file_name);
        }
        catch (std::exception &e)
        {
            Interface::MessageBox(Interface::MessageBoxType::warning, "Unable to load config", e.what());
        }
    }
    void Save() const
    {
        try
        {
            std::string obj_string = Refl::ToString(object);
            Stream::SaveFile(file_name, obj_string, Stream::text);
        }
        catch (...) {}
    }

    void DisplayGui()
    {
        if (ImGui::Begin(STR("Config: ", (file_name)).c_str()))
        {
            auto refl = Refl::Interface<T>(object);

            ImGui::PushItemWidth(iround(ImGui::GetWindowContentRegionWidth() * 0.4));
            refl.for_each_field([&](auto index)
            {
                constexpr int i = index.value;
                DisplayGuiLow(refl.template field_value<i>(), refl.field_name(i));
            });
            ImGui::PopItemWidth();
        }
        ImGui::End();
    }

    Config(const Config &) = default;
    Config(Config &&) noexcept = default;
    Config &operator=(const Config &) = default;
    Config &operator=(Config &&) noexcept = default;

    ~Config()
    {
        if (was_modified_in_gui)
            Save();
    }
};
