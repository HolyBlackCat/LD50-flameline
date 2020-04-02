#pragma once

#include <exception>
#include <functional>
#include <map>
#include <string_view>
#include <string>
#include <type_traits>

#include "program/errors.h"
#include "reflection/full.h"
#include "stream/input.h"
#include "strings/format.h"

class InterfaceStrings
{
    // Stores localization strings.
    // The strings support libfmt formatting syntax.

    std::map<std::string, std::string, std::less<>> strings;

    // Construct a dummy object of type `T`, for the purposes of verifying a format string.
    template <typename T>
    static T ConstructDummyParameter()
    {
        // We assume `T` is non-const, and not a reference.

        if constexpr (std::is_same_v<T, const char *>) // `char *` is not disallowed by a `static_assert`, so we don't need to handle it.
        {
            return "";
        }
        else
        {
            return T{};
        }
    }

  public:
    InterfaceStrings() {}

    // Loads strings from a file.
    // File format: `[("key1","value1"),...]`.
    InterfaceStrings(Stream::Input input)
    {
        Refl::FromString(strings, input);
    }


    // A class that stores a reference to a string.
    // `P...` are formatting parameters.
    template <typename ...P>
    class Str
    {
        static_assert((!std::is_reference_v<P> && ...), "The template parameters must not be references.");
        static_assert((!std::is_const_v<P> && ...), "The template parameters must not be const.");
        static_assert((!std::is_same_v<P, char *> && ...), "`char *` parameter makes no sense, use `const char *`.");

        // This is necessary to be able to check formatting strings. (By default-constructing arguments and substituting them into a string.)
        static_assert((std::is_default_constructible_v<P> && ...), "The template parameters must be default-constructible.");

        friend InterfaceStrings;

        static constexpr bool have_params = sizeof...(P) > 0;

        const std::string *str = nullptr;

      public:
        Str() {}

        // Returns the string, with arguments substituted into it.
        // Shouldn't throw, since string format is checked earlier.
        // If there are no parameters, returns a const reference to the string, otherwise returns by value.
        [[nodiscard]] std::conditional_t<have_params, std::string, const std::string &> operator()(const P &... params)
        {
            if constexpr (have_params)
                return fmt::format(*str, params...);
            else
                return *str;
        }
    };

    // Returns a reference to the string with the specified name.
    // Throws on failure.
    [[nodiscard]] const std::string &GetRaw(std::string_view name) const
    {
        auto it = strings.find(name);
        if (it == strings.end())
            Program::Error("Interface string `", name, "` is missing.");
        return it->second;
    }

    // Returns a string with the specified name, wrapped in a `Str` object.
    // The `InterfaceStrings` must be kept alive as long as this string is needed.
    template <typename ...P>
    [[nodiscard]] Str<P...> Get(std::string_view name) const
    {
        Str<P...> ret;
        ret.str = &GetRaw(name);

        try
        {
            // If the parameters don't match the string, this should throw.
            (void)fmt::formatted_size(*ret.str, ConstructDummyParameter<P>()...);
        }
        catch (std::exception &e)
        {
            Program::Error("Interface string `", name, "` is invalid:\n", e.what());
        }

        return ret;
    }

    // Fills `object` with strings, by repeatedly calling `Get()`.
    // `object` has to have its members reflected.
    // The only allowed member types are specializations of `Str<...>`.
    template <typename T> void InitStrings(T &object, std::string name_prefix = "") const
    {
        static_assert(Refl::Class::members_known<T> && Refl::Class::member_names_known<T>, "Members of this class are not reflected.");

        auto GetString = [&]<typename ...P>(Str<P...> &str, std::string_view name)
        {
            str = Get<P...>(name);
        };

        Meta::cexpr_for<Refl::Class::member_count<T>>([&](auto index)
        {
            constexpr auto i = index.value;
            GetString(Refl::Class::Member<i>(object), name_prefix + Refl::Class::MemberName<T>(i));
        });
    }
};
