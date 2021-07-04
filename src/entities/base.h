#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "macros/finally.h"
#include "meta/lists.h"
#include "meta/misc.h"
#include "meta/type_info.h"
#include "program/errors.h"
#include "utils/alignment.h"
#include "utils/simple_iterator.h"

namespace Ent
{
    // A common base class for components.
    struct Component {};

    // The maximum alignment that components can get. Overaligned components trigger a static assertion.
    inline constexpr std::size_t component_alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

    // A concept for components.
    // A component is a class inheriting from `Component`.
    // It can't be overaligned, and can't have cv-qualifiers.
    // The `Component` base must be unambiguous (because if it is, then EBO probably doesn't apply, and you waste some storage).
    // Note that `std::derived` automatically makes sure the base is unambiguous.
    template <typename T>
    concept ValidComponent =
        std::derived_from<T, Component> && !std::is_same_v<T, Component> &&
        !std::is_const_v<T> && !std::is_volatile_v<T> && alignof(T) <= component_alignment;


    namespace impl
    {
        // Returns a pointer to a tuple element with specified `std::type_index`, null if no such element.
        // The tuple is supposed to contain unique types only.
        template <typename ...P>
        const void *GetTupleElemByTypeIndex(const std::tuple<P...> &tuple, std::type_index type_index) noexcept
        {
            using tuple_t = std::tuple<P...>;
            const void *ret = nullptr;
            Meta::cexpr_any<sizeof...(P)>([&](auto index)
            {
                constexpr auto i = index.value;
                using member_t = std::tuple_element_t<i, tuple_t>;
                if (typeid(member_t) != type_index)
                    return false;
                ret = &std::get<i>(tuple);
                return true;
            });
            return ret;
        }
    }

    // A common abstract base for entities.
    class Entity
    {
        /* Explanation on `__attribute__((const))`:
         * It's is a hint to the optimizer. It means that the return value depends only on the parameters,
         * and there are no side effects. Since the functions below merely return `this + offset` or booleans, it should be allowed.
         * GCC doesn't seem to be clever enough to utilize this attribute though, but Clang appears to use it.
         */

        // Returns a pointer to a component of this entity with the specified type index, or null if no such component.
        __attribute__((const))
        virtual const void *GetComponentPtr(std::type_index index) const noexcept = 0;

      public:
        Entity() {}

        // Prevent assignment of references to entities.
        Entity(const Entity &) = delete;
        Entity &operator=(const Entity &) = delete;

        virtual ~Entity() = default;

        // `has` and `get` should be decently optimized in release builds
        // (due to the custom attribute on `GetComponentPtr`),
        // so you shouldn't have to cache the results (at least in trivial cases).

        // Check if this entity has a specific component.
        template <ValidComponent T>
        [[nodiscard]] bool has() const
        {
            return bool(GetComponentPtr(typeid(T)));
        }

        // Get component by type. Throws if no such component.
        template <ValidComponent T>
        [[nodiscard]] T &get()
        {
            return const_cast<T &>(std::as_const(*this).get<T>());
        }
        // Get component by type. Throws if no such component.
        template <ValidComponent T>
        [[nodiscard]] const T &get() const
        {
            const void *ret = GetComponentPtr(typeid(T));
            if (!ret)
                Program::Error(FMT("No component `{}` in this entity.", Meta::TypeName<T>()));
            return *static_cast<const T *>(ret);
        }

        // Set component by type. Throws if no such component.
        // Returns a reference to this entity, to allow chaining.
        template <ValidComponent T>
        Entity &set(const T &value)
        {
            get<T>() = value;
            return *this;
        }
        // Set component by type. Throws if no such component.
        // Returns a reference to this entity, to allow chaining.
        template <ValidComponent T>
        Entity &set(T &&value)
        {
            get<T>() = std::move(value);
            return *this;
        }
    };


    namespace impl
    {
        // Base classes for `Requires` and `Implies`.
        // See comments on `DirectlyRequiredComponents` for why we need them.
        struct Requires_Base {};
        struct Implies_Base {};
        struct ConflictsWith_Base {};
    }

    // Inherit a component from this class to indicate which components it depends on.
    // Attempt to create an entity with this component but without some of its dependencies
    // will cause a compile-time error.
    template <ValidComponent ...C>
    struct Requires : impl::Requires_Base
    {
        using _component_requires = Meta::type_list<C...>;
    };

    // Inherit a component from this class to indicate which components it depends on.
    // Adding this component to an entity will automatically add the dependencies.
    template <ValidComponent ...C>
    struct Implies : impl::Implies_Base
    {
        using _component_implies = Meta::type_list<C...>;
    };

    // Inherit a component from this class to indicate which components it conflicts with.
    // Using both of those components in the same entity will trigger a static assertion.
    template <ValidComponent ...C>
    struct ConflictsWith : impl::ConflictsWith_Base
    {
        using _component_conflicts = Meta::type_list<C...>;
    };


    namespace impl
    {
        // Implementation of an entity class inheriting from `Entity` and providing storage for components.


        // The element-wise constructor of `SpecificEntity` uses this
        // to initialize components that didn't get an initializer.
        struct DefaultComponentInitializer
        {
            template <typename T>
            operator T() const
            {
                static_assert(std::default_initializable<T>, "This component is not default-constructible.");
                return T{};
            }
        };

        // Returns the index of `T` in `P...`, or -1 if not found, or -2 if found more than once.
        // Ignores cvref when comparing types.
        template <typename T, typename ...P>
        inline constexpr std::size_t find_type_index_ignoring_cvref = []{
            std::size_t index = 0, ret = -1;
            int matches = 0;
            ((std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<P>> ? void((ret = index, matches++)) : void(), index++), ...);
            if (matches > 1)
                ret = -2;
            return ret;
        }();

        // Inherits from `Entity` and stores components `C...`.
        // Don't use this class directly, because it doesn't remove duplicates from `C...` and doesn't enforce component dependencies.
        template <ValidComponent ...C>
        class SpecificEntity : public Entity
        {
            std::tuple<C...> components;

            __attribute__((const)) // See the base class for the explanation of this attribute.
            const void *GetComponentPtr(std::type_index index) const noexcept override final
            {
                return GetTupleElemByTypeIndex(components, index);
            }

          public:
            // Value-initializes all components. (The tuple constructor does that by default.)
            // Note the `requires`. It's not necessary per se, because the tuple won't have a default constructor
            // if some of the components are not default-constructible. However, it gives us a nicer error message
            // when user tries to default-construct a component, instead of a cryptic "a tuple doesn't have a default constructor".
            // The other (fancy) constructor is selected, and the error message for it should indireclty point the user
            // to a specific component that lacks the default constructor.
            SpecificEntity()
            requires(std::default_initializable<C> && ...)
            {}

            // Initializes each component separately.
            // If there's a parameter with the same type as the component (ignoring cv-qualifiers and value category),
            //   the corresponding component is initialized with it. Otherwise it's initialized
            //   with `DefaultComponentInitializer{}`, which should default-construct it.
            template <typename ...P>
            requires(ValidComponent<std::remove_cvref_t<P>> && ...)
            SpecificEntity(P &&... params) : components(
                // Make a list of initializers for the tuple.
                []<typename T>(Meta::tag<T>, P &&... params) -> decltype(auto)
                {
                    // Here `T` is one of `C...`.
                    constexpr std::size_t index = find_type_index_ignoring_cvref<C, P...>;

                    // Make sure we didn't get more than one initializer.
                    // Note that the condition is separated into a variable, to prevent it from cluttering the error message.
                    constexpr bool x = index != std::size_t(-2);
                    static_assert(("More than one initializer was provided for component", Meta::tag<C>{}, x));

                    if constexpr (index == std::size_t(-1))
                        return DefaultComponentInitializer{};
                    else
                        return std::get<index>(std::forward_as_tuple(std::forward<P>(params)...));
                }(Meta::tag<C>{}, std::forward<P>(params)...)...)
            {
                // Make sure we don't have any extra unused initializers.
                ([]<typename T>(Meta::tag<T>)
                {
                    // Here `T` is one of `P...`.
                    // Check if the entity actually has a component `std::remove_cvref<T>`.
                    // Note that the condition is separated into a variable, to prevent it from cluttering the error message.
                    constexpr bool x = Meta::list_contains_type<Meta::type_list<C...>, std::remove_cvref_t<T>>;
                    static_assert(("This entity doesn't contain component", Meta::tag<T>{}, "but an initializer for it was provided.", x));
                }(Meta::tag<P>{}), ...);
            }
        };


        // `::type` is a `Meta::type_list` of components directly required by `C`.
        template <typename C>
        struct DirectlyRequiredComponents
        {
            using type = Meta::type_list<>;
        };
        // Note that we use inheritance to check if a component has requirements, rather
        //   than directly checking for the member typedef. If user accidentally inherits from
        //   `Requires<...>` non-publicly or more than once, we'll be able to detect that, instead of
        //   silently ignoring the requirements due to the member typedef being ambiguous or inaccessible.
        // Note the use of `std::is_base_of` instead of `std::derived_from`, because the latter would return false for an ambiguous base.
        template <typename C> requires std::is_base_of_v<Requires_Base, C>
        struct DirectlyRequiredComponents<C>
        {
            // `std::derived_from` (unlike `std::is_base_of_v`) makes sure the base is non-ambiguous.
            // We also check that the member typedef is accessible.
            static_assert(std::derived_from<C, Requires_Base> && requires{typename C::_component_requires;},
                "A component can inherit from `Requires<...>` at most once, and the inheritance must be public.");
            using type = typename C::_component_requires;
        };

        // `::type` is a `Meta::type_list` of components directly implied by `C`.
        // See comments on `DirectlyRequiredComponents` for the explanation of the implementation.
        template <typename C>
        struct DirectlyImpliedComponents
        {
            using type = Meta::type_list<>;
        };
        template <typename C> requires std::is_base_of_v<Implies_Base, C>
        struct DirectlyImpliedComponents<C>
        {
            static_assert(std::derived_from<C, Implies_Base> && requires{typename C::_component_implies;},
                "A component can inherit from `Implies<...>` at most once, and the inheritance must be public.");
            using type = typename C::_component_implies;
        };

        // `::type` is a `Meta::type_list` of components that `C` conflicts with.
        // See comments on `DirectlyRequiredComponents` for the explanation of the implementation.
        template <typename C>
        struct ConflictingComponents
        {
            using type = Meta::type_list<>;
        };
        template <typename C> requires std::is_base_of_v<ConflictsWith_Base, C>
        struct ConflictingComponents<C>
        {
            static_assert(std::derived_from<C, ConflictsWith_Base> && requires{typename C::_component_conflicts;},
                "A component can inherit from `ConflictsWith<...>` at most once, and the inheritance must be public.");
            using type = typename C::_component_conflicts;
        };


        // Returns a list of all components recursively implied by `P...`, including `P...` themselves.
        template <typename ...P>
        struct AddImpliedComponents
        {
            using type = Meta::type_list<>;
        };
        template <typename T, typename ...P>
        struct AddImpliedComponents<T, P...>
        {
            using type = Meta::list_copy_uniq<typename AddImpliedComponents<P...>::type, // 3. Remaining types.
                         Meta::list_copy_uniq<typename Meta::list_apply_types<AddImpliedComponents, typename DirectlyImpliedComponents<T>::type>::type, // 2. Dependencies of this type.
                                              Meta::type_list<T>>>; // 1. This type.
        };


        // Check if component list `L` satisfies requirements of its elements.
        // Always returns true, but triggers a static assertion on failure.
        // Note that the function is written in a way that should provide decent
        //   error messages (i.e. point to specific components that caused the problem).
        // Make sure it stays this way if you decide to rewrite it.
        template <typename L>
        constexpr bool CheckComponentRequirements()
        {
            // Convert `L` to a parameter pack `ListElements...`.
            []<typename ...ListElements>(Meta::type_list<ListElements...>)
            {
                // For `ListElement` = each type in `ListElements...`.
                ([]<typename ListElement>(Meta::tag<ListElement>)
                {
                    // Check requirements.
                    // Convert `DirectlyRequiredComponents<ListElement>` to parameter pack `RequiredComponents...`.
                    []<typename ...RequiredComponents>(Meta::type_list<RequiredComponents...>)
                    {
                        // For `RequiredComponent` = each type in `RequiredComponents...`.
                        ([]<typename RequiredComponent>(Meta::tag<RequiredComponent>)
                        {
                            // Here `RequiredComponent` is one of the components required by `ListElement`.

                            // Note that we don't need to recursively check the requirements of `RequiredComponent`. If some of the directly required
                            // components are missing, you'll get an error regardless, and if all of them are present, they'll check
                            // their own requirements.
                            // The condition is separated into a variable to prevent it form being included in the assertion message.
                            constexpr bool x = Meta::list_contains_type<L, RequiredComponent>;
                            // `Meta::tag<...>{}` is used to print types in the error message.
                            static_assert(("Entity lacks component:", Meta::tag<RequiredComponent>{}, "required by:", Meta::tag<ListElement>{}, x));
                        }(Meta::tag<RequiredComponents>{}), ...);
                    }(typename DirectlyRequiredComponents<ListElement>::type{});

                    // Check conflicts.
                    // Convert `ConflictingComponents<ListElement>` to parameter pack `Conflicts...`.
                    []<typename ...Conflicts>(Meta::type_list<Conflicts...>)
                    {
                        // For `Conflict` = each type in `Conflicts...`.
                        ([]<typename Conflict>(Meta::tag<Conflict>)
                        {
                            // Here `Conflict` is one of the components that `ListElement` conflicts with.

                            // See comments in the lambda-loop above on why we write the assertion in this specific way.
                            constexpr bool x = !Meta::list_contains_type<L, Conflict>;
                            static_assert(("Component", Meta::tag<ListElement>{}, "conflicts with", Meta::tag<Conflict>{}, x));
                        }(Meta::tag<Conflicts>{}), ...);
                    }(typename ConflictingComponents<ListElement>::type{});
                }(Meta::tag<ListElements>{}), ...);
            }(L{});
            return true;
        }

        // Constructs a `Meta::type_list` of components that includes `C...` and recursively all components implied by them.
        // Verifies that all component dependencies are met.
        // The resulting list is accessible as `::components`.
        template <ValidComponent ...C>
        struct MakeComponentList
        {
            static_assert(sizeof...(C) > 0, "Refuse to create an entity with zero components.");
            using components = typename AddImpliedComponents<C...>::type;

            // No error message needed, because `CheckComponentRequirements` always triggers
            // a static assertion on failure.
            static_assert(CheckComponentRequirements<components>());
        };



        // Implementation of intrusive linked lists for entities.


        // A node class for the intrusive linked lists of entities.
        class ListNode
        {
            ListNode *prev = nullptr;
            ListNode *next = nullptr;
            Entity *target = nullptr;

          public:
            // Non-copyable, non-movable. Primarily for simplicity.
            ListNode(const ListNode &) = delete;
            ListNode &operator=(const ListNode &) = delete;

            ~ListNode()
            {
                ASSERT(next->prev == this && prev->next == this, "Entity linked list consistency check failed.");

                // If `next = prev = this`, this is a no-op.
                prev->next = next;
                next->prev = prev;

                // This should help catch errors in debug builds.
                next = nullptr;
                prev = nullptr;
            }

            struct insert_before {};
            // Inserts the node before an other node.
            ListNode(insert_before, ListNode &other, Entity *entity) noexcept
            {
                target = entity;
                next = &other;
                prev = other.prev;
                other.prev->next = this;
                other.prev = this;
            }

            struct list_head {};
            // Constructs a list head node. Both `next` and `prev` are initially equal to `this`.
            ListNode(list_head) noexcept
            {
                prev = this;
                next = this;
            }

            // The entity this node points to, or null if this node is a list head.
            [[nodiscard]] Entity *Target() const noexcept
            {
                return target;
            }

            // Next and prev pointers are never null, at least not until the node is destroyed.
            // Note that the linked lists are circular, but there's always a single dummy node with `Target() == null`.
            [[nodiscard]] ListNode *Next() const noexcept
            {
                ASSERT(next->prev == this, "Entity linked list consistency check failed.");
                return next;
            }
            [[nodiscard]] ListNode *Prev() const noexcept
            {
                ASSERT(prev->next == this, "Entity linked list consistency check failed.");
                return prev;
            }
        };

        // Inherits from `SpecificEntity` and adds stuff for the intrusive linked lists of entities.
        // Uses something similar to a flexible-array-member, so be careful when allocating memory for those.
        template <ValidComponent ...C>
        class SpecificEntityWithNodes final : public SpecificEntity<C...>
        {
            // The amount of lists that this object is a part of,
            // equal to the amount of `ListNode`s following this class in memory.
            int node_count = 0;

          public:
            // Note that this class is non-copyable and non-movable,
            // because the same is true for `Entity` it inherits from.

            struct have_enough_storage {};
            // YOU MUST INVOKE THIS CONSTRUCTOR USING PLACEMENT NEW, AND HAVE ENOUGH STORAGE ALLOCATED.
            // The required size of storage can be obtained from `RequiredStorageSize()`.
            // Constructs all the nodes using placement-new, in memory immediately following this instance.
            // `func` is `ListNode &func(int i)`, where `i = 0..node_count-1. It must return a reference to
            //   the head node of i-th list to which this entity should be appended.
            // The `params...` are forwarded to the constructor of `SpecificEntity`, see it for details.
            template <typename F, typename ...P>
            explicit SpecificEntityWithNodes(have_enough_storage, int node_count, F &&func, P &&... params)
                requires(noexcept(func(int{})))
                : SpecificEntityWithNodes::SpecificEntity(std::forward<P>(params)...), node_count(node_count)
            {
                for (int i = 0; i < node_count; i++)
                    ::new(GetNodeStoragePtr(i)) ListNode(ListNode::insert_before{}, func(i), this);
            }

            // Destroys the owned nodes.
            ~SpecificEntityWithNodes()
            {
                for (int i = 0; i < node_count; i++)
                    GetNode(i).~ListNode();
            }

            // This class uses a trick similar to a flexible-array-member.
            // This function returns the amount of memory needed to store an instance of this class,
            // followed by `node_count` nodes (which is the amount of entity lists you want the instance to be a part of).
            [[nodiscard]] static std::size_t RequiredStorageSize(int node_count)
            {
                // Note how the size is increased to be a multiple of the alignment of `ListNode`.
                // We don't want the nodes to end up with a wrong alignment.
                return Storage::Align<alignof(ListNode)>(sizeof(SpecificEntityWithNodes)) + sizeof(ListNode) * node_count;
            }

            // The amount of lists that this instance is a part of, equal to
            // the amount of `ListNode`s that follow it in the memory.
            [[nodiscard]] int NodeCount() const
            {
                return node_count;
            }
            // A pointer to the storage of ith node. You need to `reinterpret_cast` and then `std::launder` it
            //   to access the actual node, which is what `GetNode` does. This function is exposed only to allow
            //   calling placement-new on the nodes.
            // I don't think we really need a `const` overload.
            [[nodiscard]] char *GetNodeStoragePtr(int index)
            {
                ASSERT(index >= 0 && index < node_count, "Node index is out of range.");
                return reinterpret_cast<char *>(this) + RequiredStorageSize(index);
            }
            // Returns a reference to the ith node.
            // I don't think we really need a `const` overload.
            [[nodiscard]] ListNode &GetNode(int index)
            {
                return *std::launder(reinterpret_cast<ListNode *>(GetNodeStoragePtr(index)));
            }
        };
    }

    // Describes a linked list of entities.
    // Shouldn't be created directly, the `Controller` will create those when requested.
    // Doesn't actually own the entities. They're owned by `Controller` that owns this list.
    class List
    {
        // A dummy "head" node.
        impl::ListNode head = impl::ListNode::list_head{};

      public:
        List() {}

        // Non-copyable and non-movable, because the same is true for `ListNode`.
        List(const List &) = delete;
        List &operator=(const List &) = delete;

        // For internal use. Returns the head node of the linked list that this instance owns.
        // Non-explicit for convenience. Since non-const lists shouldn't be exposed to user anyway, it shouldn't hurt.
        [[nodiscard]] operator impl::ListNode &()
        {
            return head;
        }

        // Iterators.
        // In short, use `.begin()` and `.end()` (which return `iterator_t`) to iterate forward.
        // You can also use `.reverse().begin()` and `.reverse().end()` (which return `reverse_iterator_t`) to iterate in the opposite direction.
        // Obviously, you can use the instance of this class and `.reverse()` in ranged-for loops.
        // All provided iterators are forward iterators.
        // The iterators are invalidated only when the entity they point to is destroyed.
      private:
        // An iterator state, for `SimpleIterator`-based iterators.
        template <bool Reverse>
        struct IteratorState
        {
            // State.
            const impl::ListNode *node = nullptr;

            // Increment.
            void operator()(std::true_type)
            {
                if constexpr (Reverse)
                    node = node->Prev();
                else
                    node = node->Next();
            }

            // Dereference.
            Entity &operator()(std::false_type)
            {
                ASSERT(node->Target());
                return *node->Target();
            }

            // Check for equality.
            bool operator==(const IteratorState &other) const
            {
                return node == other.node;
            }
        };

        // A wrapper for a pair of iterators, to conveniently pass them to ranged-for loops, among other things.
        template <bool Reverse>
        class Range
        {
            const List &list;
            using state_t = IteratorState<Reverse>;

          public:
            Range(const List &list) : list(list) {}

            using iterator_t = SimpleIterator::Forward<state_t>;

            [[nodiscard]] iterator_t begin() const {return state_t{list.head.Next()};}
            [[nodiscard]] iterator_t end  () const {return state_t{&list.head};}
        };

        using range_t = Range<false>; // Note that this is private, unlike `reverse_range_t`.

      public:
        using iterator_t = range_t::iterator_t;
        [[nodiscard]] iterator_t begin() const {return range_t(*this).begin();}
        [[nodiscard]] iterator_t end  () const {return range_t(*this).end  ();}

        using reverse_range_t = Range<true>;
        using reverse_iterator_t = reverse_range_t::iterator_t;
        [[nodiscard]] reverse_range_t reverse() const {return reverse_range_t(*this);}
    };

    // Use this to get an `List` from an `Controller`.
    class ListHandle
    {
        std::size_t index = -1;

      public:
        // Makes a null (invalid) handle.
        ListHandle() {}

        // Checks if the handle is null or not.
        [[nodiscard]] explicit operator bool() const
        {
            return index != std::size_t(-1);
        }

        // Mostly for internal use. Constructs a new handle given a list index.
        [[nodiscard]] static ListHandle ConstructFromIndex(std::size_t index)
        {
            ListHandle ret;
            ret.index = index;
            return ret;
        }

        // For internal use. Returns the list index stored in the object.
        [[nodiscard]] std::size_t GetIndex() const
        {
            return index;
        }
    };


    // For internal use, unless you're writing a custom entity template cache.
    // A non-template base for `EntityTemplate`.
    class UntypedEntityTemplate
    {
        std::vector<ListHandle> list_handles;

      public:
        explicit UntypedEntityTemplate(std::vector<ListHandle> list_handles) : list_handles(std::move(list_handles)) {}

        // For internal use.
        [[nodiscard]] const std::vector<ListHandle> &GetListHandles() const
        {
            return list_handles;
        }
    };

    // Stores all information necessary to create an entity, for a specific `Controller` configuration.
    // You're supposed to cache those, either manually or using `EntityTemplateCache` (see below).
    // Entity templates can be reused for different controllers with the same configuration and same
    //   default components.
    template <ValidComponent ...C>
    class EntityTemplate : public UntypedEntityTemplate
    {
      public:
        // For internal use.
        EntityTemplate(UntypedEntityTemplate base) : UntypedEntityTemplate(std::move(base)) {}
    };


    // A common base class for entity template caches.
    class BasicEntityTemplateCache {};

    // A concept for entity template caches.
    // Simply checks if it inherits from the base and is default-constructible, because writing a proper concept seems to be too tricky.
    template <typename T>
    concept ValidEntityTemplateCache =
        std::derived_from<T, BasicEntityTemplateCache> && !std::is_same_v<T, BasicEntityTemplateCache> &&
        std::default_initializable<T> &&
        !std::is_const_v<T> && !std::is_volatile_v<T>;

    // A default implementation of an entity template cache.
    // Uses a hash map under the hood.
    class EntityTemplateCache : public BasicEntityTemplateCache
    {
        std::unordered_map<std::type_index, UntypedEntityTemplate> map;

      public:
        // If the template for `C...` is cached, a reference to it is returned.
        // Otherwise it's constructed by calling `create_template()`, which should return an `UntypedEntityTemplate` by value.
        template <Meta::specialization_of<Meta::type_list> L, typename F>
        [[nodiscard]] const UntypedEntityTemplate &GetTemplate(F &&create_template)
        {
            struct S
            {
                F &&func;
                operator UntypedEntityTemplate() {return std::forward<F>(func)();}
            };

            return map.try_emplace(typeid(Meta::type_list<L>), S{std::forward<F>(create_template)}).first->second;
        }
    };


    // The default allocator.
    // Dealing with `std::allocator_traits`-based allocators seems to be way too compilcated,
    // so this class serves as a more simple abstraction.
    class DefaultAllocator
    {
      public:
        // Throws on failure.
        // The allocated memory should be aligned at least to `component_alignment`.
        [[nodiscard]] char *Allocate(std::size_t size)
        {
            return static_cast<char *>(operator new(size));
        }
        // Deleting null is a no-op.
        void Deallocate(char *pointer) noexcept
        {
            return operator delete(pointer);
        }
    };

    // An allocator concept.
    template <typename T>
    concept ValidAllocator = std::movable<T> && requires(T t)
    {
        {t.Allocate(std::size_t())} -> std::same_as<char *>;
        {t.Deallocate((char *)nullptr)} noexcept -> std::same_as<void>;
    };


    // Returns true if the entity type being described contains a component with the specified index.
    using component_predicate_t = bool(std::type_index);
    // Returns true if the list being described should include entities described by the parameter predicate.
    // The return value must depend only on the parameter.
    using list_predicate_t = bool(component_predicate_t *);

    // A function implementing `list_predicate_t`. Returns true if an entity has all the specified components.
    // Use a lambda instead if you need a more complex condition.
    template <ValidComponent ...C>
    [[nodiscard]] bool HasComponents(component_predicate_t *pred)
    {
        return (pred(typeid(C)) && ...);
    }


    // The entity controller.
    // Owns several entity lists, and the entities included in them.
    // To be usable, needs to be configured once after construction using `ControllerConfig`.
    template <
        // A `Meta::type_list` of default components that are added to all entities.
        Meta::specialization_of<Meta::type_list> DefaultComponents = Meta::type_list<>,
        // An allocator for the entities.
        ValidAllocator Allocator = DefaultAllocator
    >
    class Controller
    {
        friend class ControllerConfig;
        [[no_unique_address]] Allocator allocator;

        struct ListWithPred
        {
            List list;
            std::function<list_predicate_t> predicate;
        };
        std::vector<ListWithPred> entity_lists;

        // Entity count, for debugging purposes. Can be removed without affecting anything.
        // A temporary measure, until we figure out a decent customization point to plug this in.
        Meta::ResetIfMovedFrom<std::size_t> entity_count;

      public:
        // Default-constructible (assuming the allocator is default-constructible),
        // but needs to be configured with `ControllerConfig` before use.
        Controller() {}

        // Construct from an allocator.
        Controller(const Allocator &allocator) : allocator(allocator) {}
        Controller(Allocator &&allocator) : allocator(std::move(allocator)) {}

        // Movable.
        Controller(Controller &&) = default;
        Controller &operator=(Controller &&) = default;

        // Destroys all entities owned by this controller.
        ~Controller()
        {
            DestroyAllEntities();
        }


        // Expands a list of components by adding implied dependencies (specified with `Implies<...>`).
        // Checks components requirements (specified with `Requires<...>`) and conflicts (specified with `ConflictsWith<...>`),
        //   and triggers a static assertion on failure.
        // Before any of that happens, `DefaultComponents` (the template parameter of the controller) is automatically added to the input list.
        // Returns a `Meta::type_list` of components.
        template <ValidComponent ...C>
        using full_component_list = typename Meta::list_apply_types<impl::MakeComponentList, Meta::list_cat<DefaultComponents, Meta::type_list<C...>>>::components;

        // Check if the controller is null or not.
        [[nodiscard]] explicit operator bool() const
        {
            return !entity_lists.empty();
        }

        // Returns the allocator.
        [[nodiscard]]       Allocator &GetAllocator()       {return allocator;}
        [[nodiscard]] const Allocator &GetAllocator() const {return allocator;}

        // Returns the current entity count.
        [[nodiscard]] std::size_t GetEntityCount() const {return entity_count.value;}

        // Returns the entity list with the specified handle. Throws if the handle is invalid.
        [[nodiscard]] const List &operator()(ListHandle list_handle) const
        {
            if (!list_handle)
                Program::Error("Attempt to use a null entity list handle.");
            return entity_lists.at(list_handle.GetIndex()).list;
        }
        // Same as `operator()`, but throws if the list has more than one entity.
        [[nodiscard]] const List &GetAtMostOne(ListHandle list_handle) const
        {
            const List &list = operator()(list_handle);
            List::iterator_t it = list.begin();
            if (it != list.end() && ++it != list.end())
                Program::Error("Expected at most one entity in the specified list.");
            return list;
        }
        // Same as `operator()`, but throws if the list has less than one entity.
        [[nodiscard]] const List &GetAtLeastOne(ListHandle list_handle) const
        {
            const List &list = operator()(list_handle);
            if (list.begin() == list.end())
                Program::Error("Expected at least one entity in the specified list.");
            return list;
        }
        // If the list contains a single entity, returns it, otherwise throws.
        [[nodiscard]] Entity &GetOne(ListHandle list_handle) const
        {
            const List &list = operator()(list_handle);
            List::iterator_t it = list.begin();
            if (it == list.end())
                Program::Error("Expected exactly one entity in the specified list, but got none.");
            Entity &ret = *it;
            if (++it != list.end())
                Program::Error("Expected exactly one entity in the specified list, but got more.");
            return ret;
        }

        // Destroys a single entity.
        // WARNING: This invalidates any list iterators pointing to that entity.
        void Destroy(Entity &entity)
        {
            entity.~Entity();
            allocator.Deallocate(reinterpret_cast<char *>(&entity));
            entity_count.value--;
        }
        // Destroys all entities in the specified list.
        void DestroyListed(ListHandle list_handle)
        {
            const auto &list = operator()(list_handle);
            for (auto it = list.begin(); it != list.end();)
            {
                Entity &entity = *it;
                it++; // Destroying the entity invalidates its iterators, so we must increment before doing that.
                Destroy(entity);
            }
        }
        // Destroys all entities owned by this controller.
        void DestroyAllEntities()
        {
            // Each entity must belong to at least one list. Because of that,
            // we can simply iterate over all lists and destroy every entity in them.
            for (std::size_t i = 0; i < entity_lists.size(); i++)
                DestroyListed(ListHandle::ConstructFromIndex(i));
        }

        // Create an entity using a template.
        // The `params` is a list of components, a subset of `C...`.
        // The matching components from `C...` are initialized from those, and other components are value-initialized.
        template <ValidComponent ...C, typename ...P>
        Entity &Create(const EntityTemplate<C...> &entity_template, P &&... params)
        {
            return CreateFromUntypedTemplate<C...>(entity_template, std::forward<P>(params)...);
        }
        // Create an entity by grabbing a template for it from a cache.
        // If there's no matching template in the cache, it will be added automatically.
        // The `params` is a list of components, a subset of `C...`.
        // The matching components from `C...` are initialized from those, and other components are value-initialized.
        template <ValidComponent ...C, typename ...P>
        Entity &Create(ValidEntityTemplateCache auto &template_cache, P &&... params)
        {
            auto lambda = [this]
            {
                return MakeEntityTemplate<C...>();
            };

            return CreateFromUntypedTemplate<C...>(template_cache.template GetTemplate<Meta::type_list<C...>>(lambda), std::forward<P>(params)...);
        }

        // Makes an "entity template".
        // A template is necessary to create an entity with a specific set of components.
        // Templates are exposed solely to allow them to be cached by the user.
        // Constructing one requires examining every registered entity list, so they should be cached.
        template <ValidComponent ...C>
        [[nodiscard]] EntityTemplate<C...> MakeEntityTemplate()
        {
            // A full list of components in this entity, including the default components.
            using components = full_component_list<C...>;

            // A predicate describing this entity. Returns true if a component with a given index is included.
            auto predicate = [](std::type_index index)
            {
                return [&]<typename ...P>(Meta::type_list<P...>){
                    return ((typeid(P) == index) || ...);
                }(components{});
            };

            // Iterate over all lists, and make a list of handles for the matching lists.
            std::vector<ListHandle> handles;
            for (std::size_t i = 0; i < entity_lists.size(); i++)
            {
                if (entity_lists[i].predicate(predicate))
                    handles.push_back(ListHandle::ConstructFromIndex(i));
            }

            // Construct a template.
            return UntypedEntityTemplate(std::move(handles));
        }

      private:
        template <ValidComponent ...C, typename ...P>
        Entity &CreateFromUntypedTemplate(const UntypedEntityTemplate &entity_template, P &&... params)
        {
            const auto &list_handles = entity_template.GetListHandles();

            // Stop if the entity wouldn't have been inserted into any lists.
            // Note that this is tied to the logic of `DestroyAllEntities()` and the destructor.
            // The entity must be a part of at least one list to be cleaned up properly.
            if (list_handles.empty())
                Program::Error("Refuse to create an entity that doesn't belong to any lists.");

            // Determine a full list of the components, and the actual entity type.
            using entity_type = Meta::list_apply_types<impl::SpecificEntityWithNodes, full_component_list<C...>>;
            static_assert(alignof(entity_type) <= component_alignment);

            // Allocate storage.
            std::size_t storage_size = entity_type::RequiredStorageSize(list_handles.size());
            char *storage = allocator.Allocate(storage_size);
            FINALLY_ON_THROW( allocator.Deallocate(storage); )

            // Construct the entity using placement-new.
            auto ith_list_head = [&](int i) noexcept -> impl::ListNode &
            {
                return const_cast<List &>(operator()(list_handles[i]));
            };
            entity_type *entity = ::new(storage) entity_type(typename entity_type::have_enough_storage{}, list_handles.size(), ith_list_head, std::forward<P>(params)...);

            entity_count.value++;

            return *entity;
        }
    };

    // A configurator for `Controller`s.
    // An entity controller must be configured using one of those before it can be used.
    class ControllerConfig
    {
        struct ListEntry
        {
            std::function<list_predicate_t> predicate;
        };
        std::vector<ListEntry> entity_lists;

        bool finalized = false;

        void ThrowIfFinalized()
        {
            if (finalized)
                Program::Error("Can't change configuration of a `ControllerConfig` after it was used to configure an entity controller.");
        }

      public:
        // Registers a list for the specified predicate.
        // Gives you a handle that can be used to access the list in
        // any `Controller` configured using this instance.
        [[nodiscard]] ListHandle AddList(std::function<list_predicate_t> predicate)
        {
            ThrowIfFinalized();
            auto ret = ListHandle::ConstructFromIndex(entity_lists.size());
            entity_lists.push_back({std::move(predicate)});
            return ret;
        }

        // Configures the `controller` using the stored data.
        // After you call this function at least once, attempting to modify this configuration will throw an exception.
        template <Meta::specialization_of<Controller> C>
        void ConfigureController(C &controller)
        {
            FINALLY_ON_SUCCESS( finalized = true; )

            // Stop if the `controller` was already configured.
            if (bool(controller))
                Program::Error("Attempt to reconfigure an entity controller.");

            // Stop if the controller would have 0 lists.
            if (entity_lists.empty())
                Program::Error("Refuse to configure an entity controller without any entity lists.");

            // Create the lists in the controller.
            controller.entity_lists = decltype(controller.entity_lists)(entity_lists.size());
            for (std::size_t i = 0; i < entity_lists.size(); i++)
            {
                // Note that we don't move the predicate, because the factory
                // can be used to configure several separate controllers.
                controller.entity_lists[i].predicate = entity_lists[i].predicate;
            }
        }
    };
}
