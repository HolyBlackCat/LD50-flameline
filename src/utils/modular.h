#pragma once

#include <cstddef>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <utility>

#include "program/errors.h"
#include "utils/meta.h"
#include "utils/poly_storage.h"

namespace Modular
{
    enum EnumCopyable {copyable, non_copyable};


    template <typename ...P> struct make_t {};
    template <typename ...P> inline constexpr make_t<P...> make;


    class ComponentBase;

    namespace impl
    {
        class ComponentBaseLow : public Meta::polymorphic<ComponentBaseLow> {};

        class ObjectData
        {
            struct DynamicComponentData
            {
                using _use_fake_copying_if_needed = void;

                template <typename> constexpr void _make() {}
            };

            struct Data
            {
                std::vector<Poly::Storage<ComponentBaseLow, DynamicComponentData>> components;
                std::size_t (*get_component_index)(std::type_index) = 0; // Returns `-1` if no such component.
            };
            Data data;

            template <typename ...P>
            static std::size_t GetTypeIndexInList(std::type_index index)
            {
                std::size_t ret = 0;
                ((std::type_index(typeid(P)) == index || (ret++ && false)) || ...);
                if (ret == sizeof...(P))
                    return -1;
                return ret;
            };

          public:
            ObjectData() {}

            ObjectData(const ObjectData &) = default;

            ObjectData(ObjectData &&other) noexcept : data(std::exchange(other.data, {})) {}
            ObjectData &operator=(ObjectData other) noexcept
            {
                std::swap(data, other.data);
                return *this;
            }

            template <typename ...P>
            ObjectData(make_t<P...>) : data{{Poly::make_derived<P>...}, GetTypeIndexInList<P...>}
            {
                static_assert(((std::is_base_of_v<ComponentBase, P> && !std::is_same_v<ComponentBase, P>) && ...), "All components have to inherit from `Modular::ComponentBase`.");
            }

            template <typename F> void ForEachComponent(F &&func)       {for (auto &ptr : data.components) func(*ptr);}
            template <typename F> void ForEachComponent(F &&func) const {for (auto &ptr : data.components) func(*ptr);}

            explicit operator bool() const
            {
                return bool(data.get_component_index);
            }

            template <typename T> T *GetOpt()
            {
                return const_cast<T *>(std::as_const(*this).GetOpt<T>());
            }
            template <typename T> const T *GetOpt() const
            {
                if (!*this)
                    Program::Error("Attempt to access component of a null modular object.");

                std::size_t index = data.get_component_index(typeid(T));
                if (index == std::size_t(-1))
                    return 0;

                return reinterpret_cast<const T *>(data.components[index].bytes());
            }

            template <typename T> T &Get()
            {
                return const_cast<T &>(std::as_const(*this).Get<T>());
            }
            template <typename T> const T &Get() const
            {
                const T *ptr = GetOpt<T>();
                if (!ptr)
                    Program::Error("No component of type `", typeid(T).name(), "` in a modular object.");
                return *ptr;
            }

            template <typename T> bool Has() const
            {
                return GetOpt<T>() != 0;
            }
        };
    }


    class ComponentBase : public impl::ComponentBaseLow
    {
        impl::ObjectData *object = 0;

        void ThrowIfNotAttached() const
        {
            if (!object)
                Program::Error("This component isn't bound to any modular object.");
        }

      public:
        void _attach_to_modular_object(impl::ObjectData &target)
        {
            object = &target;
        }

        template <typename T>       T *ComponentOpt()       {if (!object) return 0; return object->GetOpt<T>();}
        template <typename T> const T *ComponentOpt() const {if (!object) return 0; return object->GetOpt<T>();}

        template <typename T>       T &Component()       {ThrowIfNotAttached(); return object->Get<T>();}
        template <typename T> const T &Component() const {ThrowIfNotAttached(); return object->Get<T>();}

        template <typename T> bool ComponentExists() const {if (!object) return 0; return object->Has<T>();}
    };

    template <EnumCopyable IsCopyable = copyable>
    class Object : Meta::copyable_if<IsCopyable == copyable>
    {
        struct Low
        {
            impl::ObjectData data;

            Low() {}

            template <typename ...P>
            Low(make_t<P...>) : data(make<P...>) {}

            Low(const Low &other) : data(other.data)
            {
                AttachComponents();
            }

            Low(Low &&other) noexcept : data(std::exchange(other.data, {}))
            {
                AttachComponents();
            }

            Low &operator=(Low other) noexcept
            {
                std::swap(data, other.data);
                AttachComponents();
                return *this;
            }

            void AttachComponents()
            {
                data.ForEachComponent([&](impl::ComponentBaseLow &obj)
                {
                    static_cast<ComponentBase &>(obj)._attach_to_modular_object(data);
                });
            }
        };
        Low low;

      public:
        static constexpr bool is_copyable = IsCopyable == copyable;

        Object() {}

        template <typename ...P>
        Object(make_t<P...>) : low{make<P...>}
        {
            static_assert(((is_copyable <= std::is_copy_constructible_v<P>) && ...), "This modular object is copyable, thus all components have to be copyable as well.");
            low.AttachComponents();
        }

        explicit operator bool() const
        {
            return bool(low.data);
        }

        template <typename T>       T *GetOpt()       {return low.data.template GetOpt<T>();}
        template <typename T> const T *GetOpt() const {return low.data.template GetOpt<T>();}

        template <typename T>       T &Get()       {return low.data.template Get<T>();}
        template <typename T> const T &Get() const {return low.data.template Get<T>();}

        template <typename T> bool Has() const {return low.data.template Has<T>();}
    };
}
