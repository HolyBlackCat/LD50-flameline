#pragma once

#include <limits>

#include "meta/type_info.h"
#include "reflection/full.h"
#include "utils/poly_storage.h"

namespace Refl
{
    // If present, the class will not be registered as polymorphic even if
    // it has an explicitly polymorphic base (i.e. with `REFL_POLYMORPHIC`).
    struct DontRegisterAsPolymorphic : BasicClassAttribute {};

    namespace impl
    {
        using polymorphic_index_binary_t = std::uint16_t; // Keep this unsigned, or adjust the validation logic below.
        inline constexpr auto polymorphic_index_byte_order = ByteOrder::little;
    }

    // This provides interface to the polymorphic classes.
    namespace Polymorphic
    {
        namespace impl
        {
            // This contains some stateful template metaprogramming logic to ensure that
            // classes that are not registered as polymorphic can't be used as ones.
            // This is the last line of protection. It's checked only if all other tests
            // give false positives. Usually it happens because `Derived` is a template
            // or is a member of one.
            namespace Registration
            {
                template <typename Base, typename Derived> struct tag
                {
                    friend constexpr bool zrefl_adl_CheckRegistration(tag);
                };

                // Instantiating this type registers a class.
                template <typename Base, typename Derived> struct touch_to_register
                {
                    friend constexpr bool zrefl_adl_CheckRegistration(tag<Base, Derived>) {return true;}
                };

                void zrefl_adl_CheckRegistration() = delete; // Dummy ADL target.

                // Instantiating this type triggers an assertion if `touch_to_register` wasn't instantiated first with the same parameters.
                template <typename Base, typename Derived, typename = void> struct verify
                {
                    static_assert(Meta::value<false, Base, Derived>, "This derived class is not registered as polymorphic for some reason, possibly because it's a template or a member of one. Those are never registered.");
                };
                template <typename Base, typename Derived> struct verify<Base, Derived, std::enable_if_t<zrefl_adl_CheckRegistration(tag<Base, Derived>{})>>
                {};
            }

            // This encapsulates most of the internal logic.
            class Data
            {
              private:
                // Explicit `private` allows nice code folding here.

                // This is set to `true` once the class lists are finalized.
                inline static bool finalized = false;

                // This list temporarily stores finalization functions for different bases.
                // Once the lists are finalized, it's cleared.
                static std::vector<void(*)()> &BaseFinalizationFuncs()
                {
                    static std::vector<void(*)()> ret; // Wrapped in a function to avoid the static init order fiasco.
                    return ret;
                }

                // This contains logic pertaining to a specific base class.
                template <typename Base> struct BaseData
                {
                    struct PolyStorageData;
                    using PolyStorage = Poly::Storage<Base, PolyStorageData>;

                    // This is the template parameter for `Poly::Storage`.
                    struct PolyStorageData
                    {
                        static_assert(Class::explicitly_polymorphic<Base>, "The base class must be explicitly polymorphic.");

                        // Various function pointers and other data we store for each reflected polymorphic class.
                        // You shouldn't use those before the class lists are finalized.
                        const std::size_t *zrefl_Index = nullptr;
                        const char *zrefl_Name = nullptr;
                        void (*zrefl_ToString)(const PolyStorage &object, Stream::Output &output, const ToStringOptions &options, Refl::impl::ToStringState state) = nullptr;
                        void (*zrefl_FromString)(PolyStorage &object, Stream::Input &output, const FromStringOptions &options, Refl::impl::FromStringState state) = nullptr;
                        void (*zrefl_ToBinary)(const PolyStorage &object, Stream::Output &output, const ToBinaryOptions &options, Refl::impl::ToBinaryState state) = nullptr;
                        void (*zrefl_FromBinary)(PolyStorage &object, Stream::Input &output, const FromBinaryOptions &options, Refl::impl::FromBinaryState state) = nullptr;

                        // Required by `Poly::Storage`. Assigns correct values to the fields above.
                        template <typename Derived> constexpr void _make()
                        {
                            static_assert(Class::members_known<Derived>, "Members of this derived class are not known.");
                            static_assert(Class::name_known<Derived>, "Name of this derived class is not known.");
                            static_assert(!Class::class_has_attrib<Derived, Refl::DontRegisterAsPolymorphic>, "This class was explicitly prevented from being registered as polymorphic.");

                            // Compile-time check that the class was registered.
                            [[maybe_unused]] Registration::verify<Base, Derived> registration_verifier;

                            // `zrefl_Index` has to be a pointer, since `derived_class_index` is assigned later, when the lists are finalized.
                            zrefl_Index = &derived_class_index<Derived>;

                            zrefl_Name = Class::name<Derived>;

                            zrefl_ToString = [](const PolyStorage &object, Stream::Output &output, const ToStringOptions &options, Refl::impl::ToStringState state)
                            {
                                Interface<Derived>().ToString(object.template derived<Derived>(), output, options, state.PartOfRepresentation(options));
                            };

                            zrefl_FromString = [](PolyStorage &object, Stream::Input &output, const FromStringOptions &options, Refl::impl::FromStringState state)
                            {
                                Interface<Derived>().FromString(object.template derived<Derived>(), output, options, state.PartOfRepresentation(options));
                            };

                            zrefl_ToBinary = [](const PolyStorage &object, Stream::Output &output, const ToBinaryOptions &options, Refl::impl::ToBinaryState state)
                            {
                                Interface<Derived>().ToBinary(object.template derived<Derived>(), output, options, state.PartOfRepresentation(options));
                            };

                            zrefl_FromBinary = [](PolyStorage &object, Stream::Input &output, const FromBinaryOptions &options, Refl::impl::FromBinaryState state)
                            {
                                Interface<Derived>().FromBinary(object.template derived<Derived>(), output, options, state.PartOfRepresentation(options));
                            };
                        }
                    };

                    // A function that constructs a Poly::Storage.
                    using func_t = PolyStorage();

                    // Combines a class name and a function pointer to construct it.
                    // Those are sorted by the name.
                    struct NameToFunc
                    {
                        const char *name = nullptr;
                        func_t *func = nullptr;
                        std::size_t *index_location = nullptr; // After sorting the list, index of this entry should be written here.

                        constexpr NameToFunc() {}
                        constexpr NameToFunc(const char *name) : name(name) {} // Allows comapring this class with strings.
                        constexpr NameToFunc(const char *name, func_t *func, std::size_t *index_location)
                            : name(name), func(func), index_location(index_location) {}

                        constexpr bool operator==(const NameToFunc &other) const
                        {
                            return Utils::cexpr_strcmp(name, other.name) == 0;
                        }
                        constexpr bool operator!=(const NameToFunc &other) const
                        {
                            return !(*this == other);
                        }

                        constexpr bool operator<(const NameToFunc &other) const
                        {
                            return Utils::cexpr_strcmp(name, other.name) < 0;
                        }
                    };


                    // Returns the internal list of classes. Once finalized, it's sorted by name.
                    static std::vector<NameToFunc> &NameToFuncList()
                    {
                        static std::vector<NameToFunc> ret; // Wrapped in a function to avoid the static init order fiasco.
                        return ret;
                    }

                    // Once the list is finalized, these contain indices of the derived classes.
                    template <typename Derived> inline static std::size_t derived_class_index = -1;


                    // Register this base class.
                    static void RegisterThisBaseIfNeeded()
                    {
                        [[maybe_unused]] static int dummy = []{
                            if (finalized)
                                Program::HardError("Attempt to register polymorphic base `", Meta::TypeName<Base>(), "` after class lists were finalized.");
                            BaseFinalizationFuncs().push_back(Finalize);
                            return 0;
                        }();
                    }

                    // Finalize this base class. This is called automatically, when all lists are finalized.
                    static void Finalize()
                    {
                        // Sadly this all of this has to happen at runtime.
                        auto &list = NameToFuncList();
                        std::sort(list.begin(), list.end());
                        auto dupe_it = std::adjacent_find(list.begin(), list.end());
                        if (dupe_it != list.end())
                            Program::HardError("Duplicate derived class `", dupe_it->name, "` registered for base `", Meta::TypeName<Base>(), "`.");
                        for (std::size_t i = 0; i < list.size(); i++)
                            *list[i].index_location = i;
                    }


                    // Register a specific derived class for this base.
                    template <typename Derived> static void RegisterClass()
                    {
                        RegisterThisBaseIfNeeded();

                        static_assert(!std::is_const_v<Base> && !std::is_const_v<Derived>);
                        static_assert(std::is_polymorphic_v<Base>, "The base class must be polymorphic.");
                        static_assert(std::is_base_of_v<Base, Derived>, "This class is not derived from the base.");

                        // This sets a compile-time registration flag.
                        [[maybe_unused]] Registration::touch_to_register<Base, Derived> registration_helper;

                        if (finalized)
                            Program::HardError("Attempt to register polymorphic class `", Meta::TypeName<Derived>(), "` (derived from `", Meta::TypeName<Base>(), "`) after class lists were finalized.");

                        auto &list = NameToFuncList();
                        // Make sure we don't have too many objects.
                        // One `-1` is because we need one more position for the new element.
                        // Another `-1` is because one position is used to represent null objects.
                        if (list.size() >= std::numeric_limits<Refl::impl::polymorphic_index_binary_t>::max() - 2)
                            Program::HardError("Attempt to register too many polymorphic classes for base `", Meta::TypeName<Base>(), "`.");

                        static_assert(Class::name_known<Derived>, "Name of the derived class is not known.");
                        list.push_back({Class::name<Derived>, []() -> PolyStorage {return Poly::derived<Derived>;}, &derived_class_index<Derived>});
                    }


                    // Converts a class name to its index.
                    // This assumes the list is already finalized.
                    // If the name is invalid, returns -1.
                    static std::size_t ClassNameToIndexIfValid(const char *name)
                    {
                        auto &list = NameToFuncList();
                        auto it = std::lower_bound(list.begin(), list.end(), name);
                        if (it == list.end() || *it != name)
                            return -1;
                        return it - list.begin();
                    }

                    // Converts a class name to its index.
                    // This assumes the list is already finalized.
                    // Throws if the name is invalid.
                    static std::size_t ClassNameToIndex(const char *name)
                    {
                        std::size_t ret = ClassNameToIndexIfValid(name);
                        if (ret == std::size_t(-1))
                            Program::Error("Unknown polymorphic class name: `", name, "`.");
                        return ret;
                    }

                    // Constructs a class given its index.
                    // This assumes the list is already finalized.
                    // Throws if the index is invalid.
                    static PolyStorage ConstructFromIndex(std::size_t index)
                    {
                        auto &list = NameToFuncList();
                        if (index >= list.size())
                            Program::Error("Polymorphic class index is invalid: ", index, ".");
                        return list[index].func();
                    }
                };

                template <typename Base, typename Derived> [[maybe_unused]] inline static const int touch_to_register_class = []{
                    BaseData<Base>::template RegisterClass<Derived>();
                    return 0;
                }();

              public:
                Data() = delete;
                ~Data() = delete;

                // Call this before using any of the functions below, and before using reflection-related custom functions of `Poly::Storage`.
                static void FinalizeIfNeeded() noexcept // If this throws, there is no point in trying to recover.
                {
                    static bool once = false;
                    if (once) return;
                    once = true;

                    finalized = true;

                    auto &base_fin_funcs = BaseFinalizationFuncs();

                    for (auto func : base_fin_funcs)
                        func();
                    base_fin_funcs = {};
                }

                // `Poly::Storage` with correct template parameters.
                template <typename Base> using PolyStorageData = typename BaseData<Base>::PolyStorageData;

                // Returns the amount of registered derived classes for a specific base.
                template <typename Base> static std::size_t ListSize()
                {
                    return BaseData<Base>::NameToFuncList().size();
                }

                // Constructs an object given its index. Throws on failure.
                template <typename Base> static Poly::Storage<Base, PolyStorageData<Base>> ConstructFromIndex(std::size_t index)
                {
                    return BaseData<Base>::ConstructFromIndex(index);
                }

                // Returns the index of the class named `name`, derived from `Base`.
                // Throws of failure.
                template <typename Base> static std::size_t NameToIndex(const char *name)
                {
                    return BaseData<Base>::ClassNameToIndex(name);
                }

                // Returns the index of the class named `name`, derived from `Base`.
                // Returns -1 on failure.
                template <typename Base> static std::size_t NameToIndexIfValid(const char *name)
                {
                    return BaseData<Base>::ClassNameToIndexIfValid(name);
                }


                // If `Enable` is true, instantiating this type registers class `Derived` for the base `Base`.
                // Otherwise does nothing.
                template <bool Enable, typename Base, typename Derived> struct DirectlyRegisterDerivedClass : Meta::value_tag<&touch_to_register_class<Base, Derived>> {};
                template <typename Base, typename Derived> struct DirectlyRegisterDerivedClass<false, Base, Derived> {};
            };

            template <typename T> inline static constexpr bool is_suitable_base = Class::explicitly_polymorphic<T>;
            template <typename T> inline static constexpr bool is_suitable_derived =
                std::is_polymorphic_v<T> && !std::is_abstract_v<T> && std::is_default_constructible_v<T> && !Class::class_has_attrib<T, DontRegisterAsPolymorphic>;

            // If `Derived` is suitable (see `is_suitable_derived`), instantiating this type registers it for each its explicitly polymorphic base.
            // Otherwise does nothing.
            template <
                typename Derived, typename Base,
                typename NextBases = Meta::list_cat<Class::bases<Base>, Class::direct_virtual_bases<Base>>,
                typename = void
            >
            struct RegisterForEachBaseIfNeeded {};

            template <typename Derived, typename Base, typename ...NextBases>
            struct RegisterForEachBaseIfNeeded<Derived, Base, Meta::type_list<NextBases...>, std::enable_if_t<is_suitable_derived<Derived>>>
                : Data::DirectlyRegisterDerivedClass<is_suitable_base<Base>, Base, Derived>, // Register with this base.
                  RegisterForEachBaseIfNeeded<Derived, NextBases>... // Check bases of this base.
            {};

            template <typename Derived, typename = void> struct RegisterTypeIfNeeded {};
            template <typename Derived> struct RegisterTypeIfNeeded<Derived, std::enable_if_t<is_suitable_derived<Derived>>> : RegisterForEachBaseIfNeeded<Derived, Derived> {};

            // This attaches class registration function to the interface provided in `reflection/structs_basic.h`.
            template <typename T> struct RegisterType<T, nullptr> : RegisterTypeIfNeeded<T> {};
        }


        // This is the public interface for polymorphic classes.

        // The template parameter for `Poly::Storage` that adds reflection-related stuff.
        template <typename T> using PolyStorageData = impl::Data::PolyStorageData<T>;
        // Alias for `Poly::Storage` with correct template parameters.
        template <typename T> using PolyStorage = Poly::Storage<T, PolyStorageData<T>>;

        // The amount of registered classes derived from a specific base.
        template <typename T> [[nodiscard]] std::size_t DerivedClassCount()
        {
            impl::Data::FinalizeIfNeeded();
            return impl::Data::ListSize<T>();
        }

        // Returns the index of the stored class.
        // If the object is null, `-1` is returned.
        template <typename T> [[nodiscard]] std::size_t Index(const PolyStorage<T> &object)
        {
            impl::Data::FinalizeIfNeeded();
            if (!object)
                return -1;
            return *object.dynamic().zrefl_Index;
        }

        // Returns the name of the stored class.
        // If the object is null, a null pointer is returned.
        template <typename T> [[nodiscard]] const char *Name(const PolyStorage<T> &object)
        {
            impl::Data::FinalizeIfNeeded();
            if (!object)
                return nullptr;
            return object.dynamic().zrefl_Name;
        }

        // Returns the index of the class named `name`, derived from `T`.
        // Throws on failure.
        template <typename T> [[nodiscard]] std::size_t NameToIndex(const char *name)
        {
            impl::Data::FinalizeIfNeeded();
            return impl::Data::NameToIndex<T>(name);
        }
        template <typename T> [[nodiscard]] std::size_t NameToIndex(const std::string &name)
        {
            return NameToIndex<T>(name.c_str());
        }

        // Returns the index of the class named `name`, derived from `T`.
        // Returns `-1` if the name is invalid.
        template <typename T> [[nodiscard]] std::size_t NameToIndexIfValid(const char *name)
        {
            impl::Data::FinalizeIfNeeded();
            return impl::Data::NameToIndexIfValid<T>(name);
        }
        template <typename T> [[nodiscard]] std::size_t NameToIndexIfValid(const std::string &name)
        {
            return NameToIndexIfValid<T>(name.c_str());
        }

        // Checks if there's a reflected class named `name`, derived from `T`.
        template <typename T> [[nodiscard]] bool NameIsValid(const char *name)
        {
            return NameToIndexIfValid<T>(name) != std::size_t(-1);
        }
        template <typename T> [[nodiscard]] bool NameIsValid(const std::string &name)
        {
            return NameIsValid<T>(name);
        }

        // Constructs an object given its index.
        // Throws if the index is invalid or if the constructor throws.
        // Passing `-1` as an index causes a null object to be returned.
        template <typename T> [[nodiscard]] PolyStorage<T> ConstructFromIndex(std::size_t index)
        {
            impl::Data::FinalizeIfNeeded();
            if (index == std::size_t(-1))
                return nullptr;
            return impl::Data::ConstructFromIndex<T>(index);
        }

        // Constructs an object given its name.
        // Throws if the name is invalid or if the constructor throws.
        // Passing a null pointer as the name causes a null object to be returned.
        template <typename T> [[nodiscard]] PolyStorage<T> ConstructFromName(const char *name)
        {
            return ConstructFromIndex<T>(NameToIndex<T>(name));
        }
        template <typename T> [[nodiscard]] PolyStorage<T> ConstructFromName(const std::string &name)
        {
            return ConstructFromName<T>(name.c_str());
        }
    }

    using Polymorphic::PolyStorage;
    using Polymorphic::PolyStorageData;


    template <typename T>
    class Interface_Polymorphic : public InterfaceBasic<T>
    {
        using elem_t = typename T::base_type;
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            if (!object)
            {
                // We use `0` because it's not a valid identitier. `nullptr` would also work.
                output.WriteString("0");
            }
            else
            {
                output.WriteString(Polymorphic::Name(object));
                if (options.pretty)
                    output.WriteChar(' ');
                object.dynamic().zrefl_ToString(object, output, options, state.PartOfRepresentation(options));
            }
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            std::string name = input.Extract(Stream::Char::Is("class name", [](char ch)
            {
                // We would use `Stream::Char::SeqIdentifier{}`, but it rejects `0`.
                return Stream::Char::IsAlphaOrDigit{}(ch) || ch == '_';
            }));

            if (name == "0")
            {
                object = nullptr;
                return;
            }

            try
            {
                // This throws if the name is invalid.
                object = Polymorphic::ConstructFromName<elem_t>(name);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }

            Utils::SkipWhitespaceAndComments(input);
            object.dynamic().zrefl_FromString(object, input, options, state.PartOfRepresentation(options));
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            Refl::impl::polymorphic_index_binary_t index = Polymorphic::Index(object); // No range validation is necessary, since we check the amount of classes when registering them.
            output.WriteWithByteOrder<Refl::impl::polymorphic_index_binary_t>(Refl::impl::polymorphic_index_byte_order, index);

            if (index != Refl::impl::polymorphic_index_binary_t(-1))
                object.dynamic().zrefl_ToBinary(object, output, options, state.PartOfRepresentation(options));
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            auto index = input.ReadWithByteOrder<Refl::impl::polymorphic_index_binary_t>(Refl::impl::polymorphic_index_byte_order);

            if (index == Refl::impl::polymorphic_index_binary_t(-1))
            {
                // We can't pass the index directly to `ConstructFromIndex`, because
                // `polymorphic_index_binary_t(-1)` might be different from `size_t(-1)`.
                object = nullptr;
                return;
            }

            try
            {
                // This throws if the index is invalid.
                object = Polymorphic::ConstructFromIndex<elem_t>(index);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }

            object.dynamic().zrefl_FromBinary(object, input, options, state.PartOfRepresentation(options));
        }
    };

    template <typename U>
    struct impl::SelectInterface<PolyStorage<U>>
    {
        using type = Interface_Polymorphic<PolyStorage<U>>;
    };
}
