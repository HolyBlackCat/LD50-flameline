#pragma once

#include <algorithm>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#include "meta/basic.h"
#include "meta/lists.h"
#include "meta/type_info.h"
#include "program/errors.h"
#include "reflection/full.h"
#include "strings/format.h"
#include "utils/robust_math.h"
#include "utils/simple_iterator.h"
#include "utils/sparse_set.h"

namespace Ent
{
    // Some functions below use `__attribute__((const))`.
    // It's a hint to the optimizer. It means that the return value depends only on the parameters, and there are no side effects.


    namespace impl
    {
        template <typename Registry> struct TypeRegistrationHelper;

        // Assigns indices to types.
        // `StateT` is maintained as a singleton.
        // If `StateT state; state.RegisterType<T>();` is well-formed, it will be called for each registered type.
        template <typename StateT>
        class TypeRegistry
        {
            friend TypeRegistrationHelper<TypeRegistry>;

            static std::size_t &MutableCounter() {static std::size_t ret = 0; return ret;}

            template <typename T>
            static std::size_t &MutableIndex() {static std::size_t ret = -1; return ret;}

            static StateT &MutableState()
            {
                static StateT ret{};
                return ret;
            }

          public:
            TypeRegistry() = delete;
            ~TypeRegistry() = delete;

            // Returns the singleton of the user-provided type, which may or may not hold some useful state.
            [[nodiscard]] static const StateT &State() {return MutableState();}

            // The amount of registered types.
            [[nodiscard]] static std::size_t Count() {return MutableCounter();}

            // The index of a registered type, or `-1` if not registered.
            template <typename T>
            [[nodiscard]] static std::size_t Index() {return MutableIndex<T>();}
        };

        // Helps to register types in a `TypeRegistry`.
        template <typename Registry>
        struct TypeRegistrationHelper
        {
            // Touching this registers a type.
            template <typename T>
            inline static const std::nullptr_t register_type = []{
                Registry::template MutableIndex<T>() = Registry::MutableCounter()++;
                // Run the optional user-provided registration code.
                if constexpr (requires{Registry::MutableState().template RegisterType<T>();})
                    Registry::MutableState().template RegisterType<T>();
                return nullptr;
            }();
        };
    }

    // A type that could be a valid component, but isn't necessarily a component.
    // Can be checked even if the type is incomplete.
    template <typename T>
    concept PossibleComponentType = Meta::cv_unqualified<T> && std::is_class_v<T>;

    // Specifies which components, if any, a component implies. See `ComponentType` below.
    template <PossibleComponentType ...C>
    struct Component final
    {
        Component() = delete;
        ~Component() = delete;
        using implies = Meta::type_list<C...>;
    };

    // Specifies which components, if any, a component implies. See `ComponentType` below.
    // Similar to `Component` but additionally allows creating entities based on this component.
    template <PossibleComponentType ...C>
    struct EntityComponent final
    {
        EntityComponent() = delete;
        ~EntityComponent() = delete;
        using implies = Meta::type_list<C...>;
    };

    // A component that is not an entity.
    // See `ComponentType` below.
    template <typename T>
    concept ComponentNonEntityType = PossibleComponentType<T> && Meta::specialization_of<typename T::component, Component>;

    // A component that is an entity.
    // See `ComponentType` below.
    template <typename T>
    concept ComponentEntityType = PossibleComponentType<T> && Meta::specialization_of<typename T::component, EntityComponent>;

    // A type that is a component.
    // Component classes are required to have `using component = Ent::[Entity]Component<...>;`.
    // The ONLY difference between `Component` and `EntityComponent` is that the latter can be created as entities.
    template <typename T>
    concept ComponentType = (ComponentNonEntityType<T> || ComponentEntityType<T>) && !(ComponentNonEntityType<T> && ComponentEntityType<T>);


    namespace impl
    {
        // The allocation/deallocation functions receive this tag, to reduce the chance of accidentally calling them.
        struct MemoryManagementTag
        {
            explicit MemoryManagementTag() {}
        };


        // Returns `M0<Derived,M1<Derived,...Mn<Derived,Base>>>`.
        // `Base` is intended to be the `DefaultTag`.
        // `Derived` is the assumed most-derived class in the chain. It's gived to each mixin as a template parameter.
        template <typename Derived, typename Base, template <typename, typename> typename ...M>
        struct CombineMixins
        {
            using type = Base;
        };

        template <typename Derived, typename Base, template <typename, typename> typename M0, template <typename, typename> typename ...M>
        struct CombineMixins<Derived, Base, M0, M...>
        {
            using NextBase = typename CombineMixins<Derived, Base, M...>::type;
            using type = M0<Derived, NextBase>;
            static_assert(std::derived_from<type, NextBase>, "Mixins must inherit from their second template parameters.");
        };
    }

    // The default tag.
    // Tags are the primary customization point.
    // Entities and controllers are tagged, but components are not.
    // Things with different tags are completely isolated from each other.
    // You can either use `DefaultTag` directly, or inherit from it.
    // Inheriting can be done either directly or by inheriting from `Ent::TagWithMixins<YourTag, Ent::DefaultTag, Mixins...>`,
    // where `Mixins...` are some of the optional mixins provided in `namespace Ent::Mixins`.
    struct DefaultTag
    {
        // An index type for entities. Should probably be unsigned.
        // Limits the max number of entities.
        using entity_index_t = std::uint32_t;

        // An generation type for entities. Should probably be unsigned.
        // Using a small type increases the risks of entity ID collisions.
        using entity_generation_t = std::uint64_t;

        // Those components are added to all entities.
        using common_components_t = Meta::type_list<>;

        // The entity base class inherits from this.
        // Anything in this class will be accessible to the user.
        struct EntityBase {};

        // The specific entity types are wrapped in this template.
        // This struct must always inherit from its template parameter, which will be derived from `EntityBase`.
        // This struct must always inherit the constructors.
        // The base will also contain following members:
        //     using primary_component_t = C0;
        //     using component_types_t = Meta::type_list<C...>;
        //     std::tuple<C...> components;
        template <typename Base>
        struct EntityAdditions : Base
        {
            using Base::Base;
        };


        // A fraction describing the capacity growth factor.
        static constexpr std::size_t capacity_growth_num = 3, capacity_growth_den = 2;

        // This is added to the controller.
        // This struct must always inherit from its template parameter.
        // This struct must always inherit the constructors.
        template <typename Base>
        struct ControllerAdditions : Base
        {
            using Base::Base;
        };


        // A customizable allocation function.
        template <typename T, Meta::deduce..., typename ...P>
        [[nodiscard]] static T *Allocate(impl::MemoryManagementTag, P &&... params)
        {
            return new T(std::forward<P>(params)...);
        }

        // A customizable deallocation function.
        // The type this receives can be a base class of the actual allocated type.
        template <Meta::deduce..., typename T>
        static void Free(impl::MemoryManagementTag, T *memory) noexcept
        {
            delete memory;
        }
    };

    // A concept for tags, which must be derived from `DefaultTag`.
    // Must work on incomplete types, so we don't actually check inheritance.
    template <typename T>
    concept TagType = Meta::cv_unqualified<T> && std::is_class_v<T>;

    // Same as `TagType`, but applies only to complete types, thus can check more requirements.
    template <typename T>
    concept CompleteTagType = TagType<T> && std::is_base_of_v<DefaultTag, T>;


    // This is a CRTP base for adding mixins to tags.
    // `Derived` is the derived class.
    // `Base` is the tag we're inheriting from, normally `DefaultTag`.
    // `Mixins...` are mixins. Each mixin must inherit from its second template parameter. The first parameter receives `Derived`.
    template <TagType Derived, TagType Base, template <typename, typename> typename ...Mixins>
    using TagWithMixins = typename impl::CombineMixins<Derived, Base, Mixins...>::type;


    namespace impl
    {
        // A state for `TypeRegistry`.
        template <TagType Tag> struct RegistryState_Components {};

        // Entity controller core. See the definition below.
        template <TagType Tag>
        class ControllerBase;
    }

    // Assigns indices to components.
    template <TagType Tag>
    using ComponentRegistry = impl::TypeRegistry<impl::RegistryState_Components<Tag>>;


    // Entity controller. See the definition below.
    template <TagType Tag>
    class Controller;

    // A base class for entities.
    template <TagType Tag>
    class Entity : public Tag::EntityBase
    {
        __attribute__((const)) virtual int GetComponentCountLow(std::size_t index) const noexcept = 0;
        __attribute__((const)) virtual const void *GetComponentPtrLow(std::size_t index) const noexcept = 0;

        template <ComponentType C>
        __attribute__((const)) int GetComponentCount() const noexcept
        {
            return GetComponentCountLow(ComponentRegistry<Tag>::template Index<C>());
        }

        template <ComponentType C>
        __attribute__((const)) const C *GetComponentPtr() const noexcept
        {
            return reinterpret_cast<const C *>(GetComponentPtrLow(ComponentRegistry<Tag>::template Index<C>()));
        }

      protected:
        // The unique entity index. Indices of destroyed entities can be reused.
        typename Tag::entity_index_t entity_index = 0;

        // Destroy the entity. Automatically remove it from all lists, etc.
        virtual void Destroy(impl::ControllerBase<Tag> &controller) noexcept = 0;

      public:
        constexpr Entity() {}

        // Non-copyable for safety.
        Entity(const Entity &) = delete;
        Entity &operator=(const Entity &) = delete;

        virtual ~Entity() = default;

        // Check if this entity has a specific component.
        template <ComponentType C>
        [[nodiscard]] bool has(bool unique = true) const
        {
            if (unique)
                return bool(GetComponentPtr<C>()); // We do this instead of `GetComponentCount<C>() == 1` to hopefully help the optimizer.
            else
                return GetComponentCount<C>() > 0;
        }

        // Get component by type. Throws if no such component.
        template <ComponentType C>
        [[nodiscard]] C &get()
        {
            return const_cast<C &>(std::as_const(*this).template get<C>());
        }
        // Get component by type. Throws if no such component.
        template <ComponentType C>
        [[nodiscard]] const C &get() const
        {
            const C *ret = GetComponentPtr<C>();
            if (!ret)
                Program::Error(FMT("{} component `{}` in this entity.", GetComponentCount<C>() > 1 ? "Ambiguous" : "No", Meta::TypeName<C>()));
            return *ret;
        }

        // Set component by type. Throws if no such component.
        // Returns a reference to this entity, to allow chaining.
        template <ComponentType C>
        Entity &set(const C &value)
        {
            get<C>() = value;
            return *this;
        }
        // Set component by type. Throws if no such component.
        // Returns a reference to this entity, to allow chaining.
        template <ComponentType C>
        Entity &set(C &&value)
        {
            // No perfect forwarding will happen here, because `ComponentType` rejects references and cv-qualifiers.
            get<C>() = std::move(value);
            return *this;
        }

        // Describes an entity type.
        struct Desc
        {
            [[nodiscard]] __attribute__((const))
            virtual bool HasComponent(std::size_t index, bool unique = true) const = 0;

            template <ComponentType C>
            [[nodiscard]] __attribute__((const))
            bool HasComponent(bool unique = true) const
            {
                return HasComponent(ComponentRegistry<Tag>::template Index<C>(), unique);
            }

          protected:
            ~Desc() {}
        };
        [[nodiscard]] virtual const Desc &Description() const = 0;
    };


    namespace impl
    {
        // Inherits `Entity` and exposes some hidden properties.
        template <TagType Tag>
        class EntityHidden : public Entity<Tag>
        {
          public:
            using Entity<Tag>::entity_index;
            using Entity<Tag>::Destroy;
        };
    }

    // An abstract base class for entity lists.
    // It seems we could make it not depend on the tag, but it would be less safe.
    template <TagType Tag>
    struct List
    {
        List() {}

        // Move-only, to prevent the user from copying those.
        List(List &&) = default;
        List &operator=(List &&) = default;

        virtual ~List() {}

        // This is called to increase capacity when needed.
        virtual void IncreaseCapacity(std::size_t new_capacity) {(void)new_capacity;}

        // Add an entity to the list. You don't need to support indices larger than the requested capacity.
        virtual void Insert(Entity<Tag> &entity) = 0;
        // Remove an entity from the list. You don't need to support indices larger than the requested capacity.
        // Unlike `Insert` this can't throw.
        virtual void Erase(Entity<Tag> &entity) noexcept = 0;
    };

    // A concept for list types derived from `List`.
    template <typename T, typename Tag>
    concept ListType = Meta::cv_unqualified<T> && std::derived_from<T, List<Tag>> && !std::is_abstract_v<T>;

    // An implemenetation of `ListBase` based on a sparse set.
    // NOTE: Deleting an element makes the pointers/iterators that were pointing to it point to the next element.
    template <TagType Tag, bool Ordered>
    class BasicSparseSet : public List<Tag>
    {
        using index_t = typename Tag::entity_index_t;

        std::vector<Entity<Tag> *> dense;
        std::vector<index_t> sparse;

        struct IterState
        {
            typename decltype(dense)::const_iterator vec_iter{};

            bool operator==(const IterState &) const = default;

            Entity<Tag> &operator()(std::false_type) const
            {
                return **vec_iter;
            }

            void operator()(std::true_type)
            {
                vec_iter++;
            }
        };

      public:
        void IncreaseCapacity(std::size_t new_capacity) override
        {
            sparse.resize(new_capacity, index_t(-1));
        }

        void Insert(Entity<Tag> &entity) override
        {
            auto entity_index = static_cast<impl::EntityHidden<Tag> &>(entity).entity_index;
            ASSERT(Robust::less(entity_index, sparse.size()), "Internal error: Entity sparse set is too small.");
            ASSERT(sparse[entity_index] == index_t(-1), "Internal error: Entity already exists in the sparse set.");
            sparse[entity_index] = dense.size();
            dense.push_back(&entity);
        }

        void Erase(Entity<Tag> &entity) noexcept override
        {
            auto entity_index = static_cast<impl::EntityHidden<Tag> &>(entity).entity_index;
            ASSERT(Robust::less(entity_index, sparse.size()), "Internal error: Entity sparse set is too small.");
            ASSERT(sparse[entity_index] != index_t(-1), "Internal error: Entity already exists in the sparse set.");
            std::size_t dense_index = sparse[entity_index];
            ASSERT(Robust::less(dense_index, dense.size()), "Internal error: Index in the sparse array in the entity sparse set is too small.");

            if constexpr (Ordered)
            {
                sparse[entity_index] = index_t(-1);
                dense.erase(dense.begin() + dense_index);
                for (std::size_t i = dense_index; i < dense.size(); i++)
                    sparse[static_cast<impl::EntityHidden<Tag> *>(dense[i])->entity_index] = i;
            }
            else
            {
                sparse[static_cast<impl::EntityHidden<Tag> *>(dense.back())->entity_index] = dense_index;
                sparse[entity_index] = index_t(-1);
                std::swap(dense[dense_index], dense.back());
                dense.pop_back();
            }

            ASSERT(
                sparse.size() - std::count(sparse.begin(), sparse.end(), index_t(-1)) == dense.size(),
                "Internal error: Entity sparse set consistency check failed: Sparse and dense array sizes mismatch."
            );
            ASSERT(
                std::all_of(dense.begin(), dense.end(), [&](Entity<Tag> *e)
                {
                    auto i = static_cast<impl::EntityHidden<Tag> *>(e)->entity_index;
                    return i == static_cast<impl::EntityHidden<Tag> *>(dense[sparse[i]])->entity_index;
                }),
                "Internal error: Entity sparse set consistency check failed: Sparse and dense roundtrip failed."
            );
        }

        // Return the current list size.
        [[nodiscard]] std::size_t size() const
        {
            return dense.size();
        }

        [[nodiscard]] auto begin() const {return SimpleIterator::Forward(IterState{dense.begin()});}
        [[nodiscard]] auto end  () const {return SimpleIterator::Forward(IterState{dense.end  ()});}
    };

    template <TagType Tag> using SparseSetOrdered = BasicSparseSet<Tag, true>;
    template <TagType Tag> using SparseSetUnordered = BasicSparseSet<Tag, false>;

    // An implemenetation of `ListBase` that can only hold a single entity.
    template <TagType Tag>
    class Single : public List<Tag>
    {
        Entity<Tag> *cur_entity = nullptr;

      public:
        // Add an entity to the list. You don't need to support indices larger than the requested capacity.
        void Insert(Entity<Tag> &entity) override
        {
            if (cur_entity)
                Program::Error("Expected at most one entity for this entity list.");
            cur_entity = &entity;
        }

        // Remove an entity from the list. You don't need to support indices larger than the requested capacity.
        void Erase(Entity<Tag> &entity) noexcept override
        {
            (void)entity;
            ASSERT(cur_entity == &entity, "Internal error: Attempt to erase a wrong entity from a single-entity list.");
            cur_entity = nullptr;
        }


        // Returns true if the entity exists.
        [[nodiscard]] explicit operator bool() const
        {
            return bool(cur_entity);
        }

        // Returns the target entity, or throws if none.
        [[nodiscard]] Entity<Tag> &operator*() const
        {
            if (!cur_entity)
                Program::Error("A single-entity list contains no entitiy.");
            return *cur_entity;
        }

        // Returns the target entity, or throws if none.
        [[nodiscard]] Entity<Tag> *operator->() const
        {
            return &**this;
        }

        // Returns the target entity or null if none.
        [[nodiscard]] Entity<Tag> *get() const
        {
            return cur_entity;
        }
    };


    namespace impl
    {
        // The base class for entity categories.
        template <TagType Tag>
        struct CategoryBase
        {
            // Should return true if the list should contain this entity.
            virtual bool ShouldContain(const typename Entity<Tag>::Desc &desc) const = 0;
        };
    }

    // An abstract base for entity categories. Categories must be stateless.
    // If `C` is not empty, a specialization is used, which is not abstract.
    template <TagType Tag, template <typename> typename List, PossibleComponentType ...C>
    struct Category : impl::CategoryBase<Tag>
    {};

    // A concept for entity categories.
    template <typename T, typename Tag>
    concept CategoryType = Meta::cv_unqualified<T> && std::derived_from<T, impl::CategoryBase<Tag>> && !std::is_abstract_v<T>;


    // A non-abstract class derived from the `Category` primary template.
    // Represents a category of entities that
    template <TagType Tag, template <typename> typename List, PossibleComponentType C0, PossibleComponentType ...C>
    struct Category<Tag, List, C0, C...> : Category<Tag, List>
    {
        bool ShouldContain(const typename Entity<Tag>::Desc &desc) const
        {
            return (desc.template HasComponent<C0>() && ... && desc.template HasComponent<C>());
        }
    };


    namespace impl
    {
        // Touch this to register a component.
        template <TagType Tag, ComponentType C>
        constexpr void RegisterComponent()
        {
            // Register the component itself.
            (void)TypeRegistrationHelper<ComponentRegistry<Tag>>::template register_type<C>;

            // Recursivelt register its direct bases (virtual and regular).
            []<typename ...L>(Meta::type_list<L...>){
                ([]{
                    if constexpr (ComponentType<L>)
                        RegisterComponent<Tag, L>();
                }(), ...);
            }(Meta::list_cat_types<Refl::Class::bases<C>, Refl::Class::direct_virtual_bases<C>>{});
        }


        // A class convertible to any component type.
        // The resulting object is default-constructed, or a static assertion is triggered if it's not default-constructible.
        struct ComponentDefaultInitializer
        {
            ComponentDefaultInitializer() {}
            ComponentDefaultInitializer(const ComponentDefaultInitializer &) = delete;
            ComponentDefaultInitializer &operator=(const ComponentDefaultInitializer &) = delete;

            template <ComponentType T>
            operator T() const
            {
                constexpr bool x = std::default_initializable<T>;
                static_assert(("Component", Meta::tag<T>{}, "is not default-constructible and requires an explicit initializer.", x));
                return T{};
            }
        };

        // Inherits from `Entity` and stores components `C...`.
        // Don't use this class directly, since it doesn't preprocess the list of components.
        // `PrimaryComponent` is merely a tag, useful for serialization/deserialization.
        template <ComponentEntityType PrimaryComponent, TagType Tag, typename L>
        class EntityWithComponents;

        template <ComponentEntityType PrimaryComponent, TagType Tag, ComponentType ...C>
        class EntityWithComponents<PrimaryComponent, Tag, Meta::type_list<C...>> : public EntityHidden<Tag>
        {
          protected:
            using primary_component_t = PrimaryComponent;
            using component_types_t = Meta::type_list<C...>;
            std::tuple<C...> components;

          private:
            template <ComponentType T>
            static constexpr bool contains_component = Meta::list_contains_type<component_types_t, T>;

            // Calls the `func` for each component (either direct or a base of a component). It will be called more than once for ambiguous components.
            // `func` is `void func(Component *)`. If `e` is null, the argument will also be null, but still with the correct type.
            static void ForEachComponentRecursively(const EntityWithComponents *e, auto &&func)
            {
                auto lambda = [&]<typename T>(auto &lambda, const T *c)
                {
                    if constexpr (ComponentType<T>)
                        func(c);

                    // Call the lambda for direct non-virtual bases (recursively).
                    [&]<typename ...L>(Meta::type_list<L...>){
                        (lambda(lambda, c ? static_cast<const L *>(c) : nullptr), ...);
                    }(Refl::Class::bases<T>{});
                };

                // For each component...
                ([&]{
                    using component_t = C;
                    // Call the lambda for the component itself, and its direct non-virtual bases (recursively).
                    lambda(lambda, e ? &std::get<component_t>(e->components) : nullptr);
                    // Call the lambda for the virtual bases, including indirect ones.
                    [&]<typename ...L>(Meta::type_list<L...>){
                        (lambda(lambda, e ? static_cast<const L *>(&std::get<component_t>(e->components)) : nullptr), ...);
                    }(Refl::Class::virtual_bases<component_t>{});
                }(), ...);
            }

            // Returns 0 if no such component, 1 if it's present, or >1 if it's ambiguous.
            __attribute__((const))
            static int GetComponentCountLowStatic(std::size_t index) noexcept
            {
                static const std::vector<int> table = []{
                    std::vector<int> table(ComponentRegistry<Tag>::Count());

                    ForEachComponentRecursively(nullptr, [&]<ComponentType T>(const T *)
                    {
                        table[ComponentRegistry<Tag>::template Index<T>()]++;
                    });

                    return table;
                }();
                if (index == std::size_t(-1))
                    return 0; // This component is not registered.
                return table[index];
            }

            __attribute__((const))
            int GetComponentCountLow(std::size_t index) const noexcept override final
            {
                return GetComponentCountLowStatic(index);
            }

            __attribute__((const))
            const void *GetComponentPtrLow(std::size_t index) const noexcept override final
            {
                // Register the components (at compile-time).
                // Because this function is virtual, this conveniently runs even if it's unused.
                (RegisterComponent<Tag, C>(), ...);

                // Pre-compute the offsets (relative to the tuple) for all known components for this tag.
                // `-1` means the component is missing.
                // `-2` means the component is ambiguous.
                static const std::vector<std::ptrdiff_t> offsets = [&]{
                    std::vector<std::ptrdiff_t> offsets(ComponentRegistry<Tag>::Count(), -1);

                    // A shame that we have to use an actual instance to compute the offsets.
                    ForEachComponentRecursively(this, [&]<ComponentType T>(const T *component)
                    {
                        std::ptrdiff_t &offset = offsets[ComponentRegistry<Tag>::template Index<T>()];
                        if (offset != -1)
                        {
                            // The base is ambiguous, but don't stop because we need to process all bases.
                            offset = -2;
                        }
                        else
                        {
                            // Save the offset for this base.
                            offset = reinterpret_cast<const char *>(component) - reinterpret_cast<const char *>(&components);
                        }
                    });

                    return offsets;
                }();
                if (index == std::size_t(-1))
                    return nullptr; // This component is not registered.
                std::ptrdiff_t offset = offsets[index];
                if (offset < 0)
                    return nullptr;
                return reinterpret_cast<const char *>(&components) + offset;
            }

          public:
            // This function is written in a very specific manner to show nice messages on a static assertion.
            template <typename ...P>
            requires (contains_component<std::remove_cvref_t<P>> && ...)
            EntityWithComponents(P &&... params)
                : components(
                    [&]() -> decltype(auto) {
                        using component_t = C;
                        using search = Meta::list_find_type<Meta::type_list<std::remove_cvref_t<P>...>, component_t>;
                        if constexpr (!search::found)
                        {
                            return ComponentDefaultInitializer{};
                        }
                        else if constexpr (Meta::list_contains_type<typename search::remaining, component_t>)
                        {
                            constexpr bool x = Meta::value<false, P...>;
                            static_assert(("Duplicate component type in the initializer:", Meta::tag<component_t>{}, x));
                        }
                        else
                        {
                            return std::get<search::value>(std::forward_as_tuple(std::forward<P>(params)...));
                        }
                    }()...
                )
            {}

            struct EntityDesc : Entity<Tag>::Desc
            {
                __attribute__((const))
                bool HasComponent(std::size_t index, bool unique) const override
                {
                    if (unique)
                        return GetComponentCountLowStatic(index) == 1;
                    else
                        return GetComponentCountLowStatic(index) > 0;
                }
            };

            const typename Entity<Tag>::Desc &Description() const override
            {

                static const EntityDesc ret;
                return ret;
            }
        };


        // `R` should start as an empty `Meta::type_list<>`.
        // Returns the list `L`, recursively amended with implied components.
        template <typename L, typename R>
        struct ImpliedComponents {};
        template <typename R>
        struct ImpliedComponents<Meta::type_list<>, R>
        {
            using type = R;
        };
        template <ComponentType C0, ComponentType ...C, typename R> requires Meta::list_contains_type<R, C0>
        struct ImpliedComponents<Meta::type_list<C0, C...>, R>
        {
            using type = typename ImpliedComponents<Meta::type_list<C...>, R>::type;
        };
        template <ComponentType C0, ComponentType ...C, typename R> requires(!Meta::list_contains_type<R, C0>)
        struct ImpliedComponents<Meta::type_list<C0, C...>, R>
        {
            using type =
                typename ImpliedComponents<
                    Meta::list_cat_types<
                        typename C0::component::implies, // Add directly implied components.
                        Refl::Class::bases<C0>, // Add direct non-virtual bases.
                        Refl::Class::direct_virtual_bases<C0>, // Add direct virtual bases.
                        Meta::type_list<C...> // Add remaining components.
                    >,
                    Meta::list_cat_types<R, Meta::type_list<C0>> // Add `C0` to the returned list.
                >::type;
        };


        template <TagType Tag, template <typename> typename List>
        constexpr Meta::tag<List<Tag>> ListFromCategoryHelper(Category<Tag, List> *) {return {};}

        // Returns the list type used by a category `C`.
        template <typename C>
        using ListFromCategory = typename decltype(ListFromCategoryHelper((C *)nullptr))::type;


        // A state for `TypeRegistry` for categries.
        template <TagType Tag>
        struct RegistryState_Categories
        {
            // Functions that can be used to construct the lists corresponding to the categories.
            std::vector<std::unique_ptr<List<Tag>> (*)()> list_factory_funcs;

            // The singleton instances of the category types.
            std::vector<const CategoryBase<Tag> *> instances;

            template <CategoryType<Tag> C>
            void RegisterType()
            {
                list_factory_funcs.push_back([]() -> std::unique_ptr<List<Tag>> {return std::make_unique<ListFromCategory<C>>();});
                static C instance{};
                instances.push_back(&instance);
            }
        };

        // A state for `TypeRegistry` for entity types.
        template <TagType Tag>
        struct RegistryState_EntityTypes
        {
            // Entity type descriptions.
            std::vector<const typename Entity<Tag>::Desc *> descriptions;

            template <typename T>
            void RegisterType()
            {
                static typename T::EntityDesc desc;
                descriptions.push_back(&desc);
            }
        };
    }

    // Assigns indices to categories.
    template <TagType Tag>
    using CategoryRegistry = impl::TypeRegistry<impl::RegistryState_Categories<Tag>>;

    // Assigns indices to entity types.
    template <TagType Tag>
    using EntityTypeRegistry = impl::TypeRegistry<impl::RegistryState_EntityTypes<Tag>>;


    // The entity pointer.
    // Use `controller(entity)` to form one and `controller(pointer)` to access the entity.
    template <TagType Tag, bool IsConst>
    class BasicPointer
    {
        friend BasicPointer<Tag, !IsConst>;
        friend impl::ControllerBase<Tag>;
      public:
        using index_t = typename Tag::entity_index_t;
        using generation_t = typename Tag::entity_generation_t;

      private:
        index_t index = -1;
        generation_t generation = 0;

      public:
        constexpr BasicPointer() {}
        constexpr BasicPointer(std::nullptr_t) {}

        // Returns true if the pointer is not null.
        // To additionally check if it's not expired, you need to access it with `controller(pointer)`.
        [[nodiscard]] bool IsSet() const
        {
            return index != index_t(-1);
        }

        // Implicit conversion from non-const to const pointer.
        [[nodiscard]] operator BasicPointer<Tag, true>() const requires(!IsConst)
        {
            BasicPointer<Tag, true> ret;
            ret.index = index;
            ret.generation = generation;
            return ret;
        }

        // Returns the entity index, mostly for debug purposes.
        [[nodiscard]] index_t GetIndex() const
        {
            return index;
        }
        // Returns the entity generation, mostly for debug purposes.
        [[nodiscard]] generation_t GetGeneration() const
        {
            return generation;
        }

        // The regular comparisons.
        auto operator<=>(const BasicPointer &) const = default;

        // Comparisons with different constness.
        bool operator==(const BasicPointer<Tag, !IsConst> &p) const
        {
            return *this <=> p == 0;
        }
        auto operator<=>(const BasicPointer<Tag, !IsConst> &p) const
        {
            return BasicPointer<Tag, true>(*this) <=> BasicPointer<Tag, true>(p);
        }
    };
    template <TagType Tag> using Pointer = BasicPointer<Tag, false>;
    template <TagType Tag> using ConstPointer = BasicPointer<Tag, true>;


    namespace impl
    {
        // The core part of the entity controller.
        // `Derived` is the most-derived controller class.
        template <TagType Tag>
        class ControllerBase
        {
            static_assert(CompleteTagType<Tag>);

            // The entity lists.
            std::vector<std::unique_ptr<List<Tag>>> lists;

            struct EntityDeleter
            {
                void operator()(Entity<Tag> *e) const
                {
                    Tag::Free(impl::MemoryManagementTag{}, e);
                }
            };
            using entity_unique_ptr_t = std::unique_ptr<Entity<Tag>, EntityDeleter>;

            struct EntityData
            {
                // Entity pointer.
                entity_unique_ptr_t ptr;

                // Entity generation.
                typename Tag::entity_generation_t generation = 0;
            };

            // Entities. The size of this vector is the current controller capacity.
            std::vector<EntityData> entities;

            // Manages entity indices. Should have the same capacity as the size of `entities`.
            SparseSet<typename Tag::entity_index_t> entity_indices;

            // Returns the class from which the final entity type should be inherited, based on a specific component.
            template <ComponentEntityType C>
            using incomplete_entity_t = typename Tag::template EntityAdditions<
                impl::EntityWithComponents<
                    C,
                    Tag,
                    typename impl::ImpliedComponents<Meta::list_cat_types<typename Tag::common_components_t, Meta::type_list<C>>, Meta::type_list<>>::type
                >
            >;

            // List indices to which an entity based on component `C` should be added.
            template <ComponentEntityType C>
            [[nodiscard]] static const std::vector<std::size_t> &GetEntityListIndices()
            {
                static std::vector<std::size_t> ret = []{
                    std::vector<std::size_t> ret;
                    typename incomplete_entity_t<C>::EntityDesc desc{};
                    for (std::size_t i = 0; i < CategoryRegistry<Tag>::Count(); i++)
                    {
                        if (CategoryRegistry<Tag>::State().instances[i]->ShouldContain(desc))
                            ret.push_back(i);
                    }
                    if (ret.empty())
                        Program::Error(FMT("Refuse to create an entity based on component `{}` that doesn't belong to any lists.", Meta::TypeName<C>()));
                    return ret;
                }();
                return ret;
            }

            // The final entity type.
            template <ComponentEntityType C>
            struct FinalEntity final : incomplete_entity_t<C>
            {
                using incomplete_entity_t<C>::incomplete_entity_t;

                void Destroy(ControllerBase &con) noexcept override
                {
                    // Remove the entity from lists.
                    for (std::size_t i : GetEntityListIndices<C>())
                        con.lists[i]->Erase(*this);

                    auto index = static_cast<impl::EntityHidden<Tag> *>(this)->entity_index;

                    // Increment the generation.
                    con.entities[index].generation++;

                    // Release the index.
                    con.entity_indices.EraseUnordered(index);

                    // Destroy self.
                    con.entities[index].ptr = nullptr;
                }
            };

          protected:
            // Protected to prevent prevent accidental misuse. Use the factory functions below.
            constexpr ControllerBase() {}

          public:
            ControllerBase(ControllerBase &&) = default;
            ControllerBase &operator=(ControllerBase &&) = default;
            ~ControllerBase() = default;

            // Creates an null controller.
            [[nodiscard]] static constexpr Controller<Tag> MakeEmptyController()
            {
                return {};
            }

            // Creates a valid controller.
            [[nodiscard]] static Controller<Tag> MakeController()
            {
                Controller<Tag> ret;
                for (std::size_t i = 0; i < CategoryRegistry<Tag>::Count(); i++)
                    ret.lists.push_back(CategoryRegistry<Tag>::State().list_factory_funcs[i]());
                return ret;
            }

            // Reports the current entity count.
            [[nodiscard]] std::size_t EntityCount() const
            {
                return entity_indices.ElemCount();
            }

            // Returns the current controller capacity, i.e. the number of entities it can hold without allocating more memory.
            [[nodiscard]] std::size_t Capacity() const
            {
                return entities.size(); // Sic.
            }

            // Returns the max possible capacity, which depends on `Tag::entity_index_t`.
            [[nodiscard]] static std::size_t MaxPossibleCapacity()
            {
                static std::size_t ret = []{
                    std::size_t ret = 0;
                    // Note the lack of `+ 1` here.
                    // This ensures that we can always use `-1` as an invalid entity index.
                    if (Robust::value(std::numeric_limits<typename Tag::entity_index_t>::max()) >>= ret)
                        ret = -1;
                    return ret;
                }();
                return ret;
            }

            // Increases capacity. Can't increase it past `MaxPossibleCapacity()`.
            void IncreaseCapacity(std::size_t new_capacity)
            {
                new_capacity = std::min(new_capacity, MaxPossibleCapacity());

                if (new_capacity <= Capacity())
                    return;

                // Note `resize` instead of `reserve`.
                entities.resize(new_capacity);
                entity_indices.Reserve(new_capacity);

                for (const auto &list : lists)
                    list->IncreaseCapacity(new_capacity);
            }

            // Creates a new entity.
            template <ComponentEntityType C, Meta::deduce..., typename ...P>
            Entity<Tag> &Create(P &&... params)
            {
                if (EntityCount() >= Capacity()) [[unlikely]]
                    IncreaseCapacity(Capacity() * Tag::capacity_growth_num / Tag::capacity_growth_den + 1); // Note `+ 1`. We need to be able to handle zero capacity.

                // Allocate the index.
                // We do it before allocating the entity, because the index allocation is more likely to fail.
                auto new_index = entity_indices.InsertAny();
                FINALLY_ON_THROW( entity_indices.EraseUnordered(new_index); )

                // Allocate the entity.
                EntityData &new_entity = entities[new_index];
                new_entity.ptr = entity_unique_ptr_t(Tag::template Allocate<FinalEntity<C>>(impl::MemoryManagementTag{}, std::forward<P>(params)...));
                FINALLY_ON_THROW( new_entity.ptr = nullptr; )
                static_cast<impl::EntityHidden<Tag> &>(*new_entity.ptr).entity_index = new_index;

                // Indices of lists to which the entity should be added.
                const auto& list_indices = GetEntityListIndices<C>();

                // Add the entity to the lists.
                std::size_t list_pos = 0;
                FINALLY_ON_THROW(
                    // Removing from the lists not in the reverse order. It shouldn't matter.
                    for (std::size_t i = 0; i < list_pos; i++)
                        lists[list_indices[i]]->Erase(*new_entity.ptr);
                )
                for (; list_pos < list_indices.size(); list_pos++)
                    lists[list_indices[list_pos]]->Insert(*new_entity.ptr);

                return *new_entity.ptr;
            }

            // Destroys an entity.
            void Destroy(Entity<Tag> &e) noexcept
            {
                static_cast<impl::EntityHidden<Tag> &>(e).Destroy(*this);
            }
            // Destroys an entity. Does nothing if the pointer is null.
            void Destroy(Entity<Tag> *p) noexcept
            {
                if (p)
                    Destroy(*p);
            }
            // Destroys an entity. Does nothing if the pointer is null or expired.
            void Destroy(const Pointer<Tag> &p) noexcept
            {
                if (auto e = operator()(p))
                    Destroy(*e);
            }

            // Provides access to an entity list.
            // The parameter is non-const to discourage ad-hoc (rvalue) categories, since creating too many categories can get expensive.
            template <CategoryType<Tag> C>
            [[nodiscard]] const impl::ListFromCategory<C> &operator()(C &) const
            {
                (void)impl::TypeRegistrationHelper<CategoryRegistry<Tag>>::template register_type<C>;
                return static_cast<impl::ListFromCategory<C> &>(*lists[CategoryRegistry<Tag>::template Index<C>()]);
            }

            // Forms a pointer to an entity.
            [[nodiscard]] Pointer<Tag> operator()(Entity<Tag> &e) const
            {
                Pointer<Tag> ret;
                ret.index = static_cast<impl::EntityHidden<Tag> &>(e).entity_index;
                ret.generation = entities[ret.index].generation;
            }
            // Forms a const pointer to an entity.
            [[nodiscard]] ConstPointer<Tag> operator()(const Entity<Tag> &e) const
            {
                return operator()(const_cast<Entity<Tag> &>(e));
            }

            // Reads a pointer to an entity. Returns null if the pointer is null or expired.
            [[nodiscard]] Entity<Tag> *operator()(const Pointer<Tag> &p) const
            {
                return const_cast<Entity<Tag> *>(operator()(ConstPointer<Tag>(p)));
            }
            // Reads a const pointer to an entity. Returns null if the pointer is null or expired.
            [[nodiscard]] const Entity<Tag> *operator()(const ConstPointer<Tag> &p) const
            {
                if (!p.IsSet())
                    return nullptr;
                ASSERT(p.index < entities.size(), "Entity index is out of range."); // This should be impossible, unless you used a pointer from a different controller.
                if (p.index >= entities.size())
                    return nullptr;
                const EntityData &data = entities[p.index];
                if (data.generation != p.generation)
                    return nullptr; // Expired.
                return data.ptr.get();
            }
        };
    }

    // The entity controller. See `impl::ControllerBase` for the actual implementation.
    template <TagType Tag>
    class Controller : public Tag::template ControllerAdditions<impl::ControllerBase<Tag>>
    {
        using Tag::template ControllerAdditions<impl::ControllerBase<Tag>>::ControllerAdditions;
    };
}
