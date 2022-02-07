#pragma once

#include <cstddef>
#include <functional>
#include <map>

#include "entities/base.h"
#include "meta/common.h"
#include "meta/type_info.h"
#include "reflection/full.h"
#include "strings/format.h"

namespace Ent::Mixins
{
    namespace impl::CreateEntitiesByName
    {
        // A function that contructs an unknown entity.
        template <TagType Tag>
        using factory_func_t = Entity<Tag> &(*)(Ent::impl::ControllerBase<Tag> &con);

        // A map of `factory_func_t` functions.
        template <TagType Tag>
        using factory_func_map_t = std::map<std::string, factory_func_t<Tag>, std::less<>>;

        // Returns a singleton for the factory function map.
        template <TagType Tag>
        [[nodiscard]] factory_func_map_t<Tag> &FactoryFuncs()
        {
            static factory_func_map_t<Tag> ret;
            return ret;
        }
    }

    // Allows entities to be constructed by their names at runtime.
    // The names are provided by reflection.
    template <typename FinalTag, typename BaseMixin>
    struct CreateEntitiesByName : BaseMixin
    {
      private:
        // Touching this class registers the entity `E`.
        // See below for requirements.
        template <ComponentEntityType E>
        class RegisterEntityToCreateByName
        {
            static_assert(Refl::Class::name_known<E>, "The name of this entity type is not reflected.");

            inline static std::nullptr_t dummy = []{
                auto factory_func = +[](Ent::impl::ControllerBase<FinalTag> &con) -> Entity<FinalTag> &
                {
                    return con.template Create<E>();
                };
                bool ok = impl::CreateEntitiesByName::FactoryFuncs<FinalTag>().try_emplace(Refl::Class::name<E>, factory_func).second;
                if (!ok)
                    Program::Error(FMT("Attempt to register a duplicate entity type `{}` for tag `{}`.", Refl::Class::name<E>, Meta::TypeName<FinalTag>()));
                return nullptr;
            }();

            [[maybe_unused]] static constexpr Meta::value_tag<&dummy> dummy_helper;
        };

      public:
        struct EntityBase : BaseMixin::EntityBase
        {
            // Returns the name by which the entity can be constructed.
            // Returns null either if the name is not reflected or the entity is not default-constructible.
            [[nodiscard]] virtual const char *GetConstructibleName() const = 0;
        };

        template <typename Base>
        struct EntityAdditions : BaseMixin::template EntityAdditions<Base>
        {
          private:
            using typename Base::primary_component_t;

            static constexpr bool constructible_by_name = Refl::Class::name_known<primary_component_t> && std::is_default_constructible_v<decltype(EntityAdditions::components)>;

          public:
            using BaseMixin::template EntityAdditions<Base>::EntityAdditions;

            const char *GetConstructibleName() const override
            {
                if constexpr (constructible_by_name)
                {
                    (void)RegisterEntityToCreateByName<primary_component_t>{}; // Register at compile-time.
                    return Refl::Class::name<primary_component_t>;
                }
                else
                {
                    return nullptr;
                }
            }
        };

        template <typename Base>
        struct ControllerAdditions : BaseMixin::template ControllerAdditions<Base>
        {
            Entity<FinalTag> &CreateByName(std::string_view name)
            {
                const auto &map = impl::CreateEntitiesByName::FactoryFuncs<FinalTag>();
                auto it = map.find(name);
                if (it == map.end())
                    Program::Error(FMT("Unknown entity type `{}` in tag `{}`.", name, Meta::TypeName<FinalTag>()));
                return it->second(*this);
            }

            // Touching this registers entity `E`.
            // Its name must be reflected, and all its components must be default-constructible.
            // Not meeting the conditions causes a static assertion.
            // If the conditions are met, creating the entity in the controller automatically registers it.
            template <ComponentEntityType E>
            using RegisterEntityToCreateByName = RegisterEntityToCreateByName<E>;
        };
    };
}
