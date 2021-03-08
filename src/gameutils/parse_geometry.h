#pragma once

#include <concepts>
#include <functional>
#include <map>
#include <string>
#include <variant>

#include "meta/misc.h"
#include "reflection/full.h"
#include "stream/input.h"
#include "strings/format.h"
#include "utils/mat.h"
#include "utils/robust_math.h"

namespace GameUtils
{
    // Contains code that parses `.ply` models.
    class GeometryPly
    {
      public:
        struct Property
        {
            int index = 0; // Index of the property in the file. Not useful to user.
            bool is_uint8 = false; // Type of the property. `uint8_t` if true, `float` otherwise.
        };

      private:
        std::map<std::string, Property, std::less<>> properties;
        std::size_t vertex_count = 0;
        std::size_t face_count = 0;
        Stream::Input input;

      public:
        // Parses the file header. You can inspect it using the functions below, then finish the parsing by calling `ParseBody()`.
        // If you don't need to inspect the header, call `Parse` instead, which combines the effects of those two functions.
        [[nodiscard]] static GeometryPly ParseHeader(Stream::Input input)
        {
            GeometryPly ret;

            input.WantLocationStyle(Stream::text_byte_position);
            input.DiscardChars("ply\n");
            input.DiscardChars("format ascii 1.0\n");
            if (input.DiscardChars<Stream::if_present>("comment "))
            {
                input.Discard<Stream::any>(!Stream::Char::EqualTo('\n'));
                input.Discard('\n');
            }
            input.DiscardChars("element vertex ");
            Refl::InterfaceFor(ret.vertex_count).FromString(ret.vertex_count, input, {}, Refl::initial_state);
            input.Discard('\n');
            while (input.DiscardChars<Stream::if_present>("property "))
            {
                Property new_property;
                new_property.index = ret.properties.size();
                if (input.DiscardChars<Stream::if_present>("float "))
                    new_property.is_uint8 = false;
                else if (input.DiscardChars<Stream::if_present>("uchar "))
                    new_property.is_uint8 = true;
                else
                    Program::Error(input.GetExceptionPrefix() + "Unknown property type, expected `float` or `uchar`.");
                std::string property_name = input.Extract(Stream::Char::IsAlpha());
                input.Discard('\n');

                // Note that it's safe to move `property_name` even though we use it in the error message. If the element fails to insert, it's not moved from.
                if (!ret.properties.try_emplace(std::move(property_name), std::move(new_property)).second)
                    Program::Error(STR((input.GetExceptionPrefix()),"Duplicate property name: `",(property_name),"`."));
            }
            input.DiscardChars("element face ");
            Refl::InterfaceFor(ret.face_count).FromString(ret.face_count, input, {}, Refl::initial_state);
            input.Discard('\n');
            input.DiscardChars("property list uchar uint vertex_indices\n");
            input.DiscardChars("end_header\n");

            ret.input = std::move(input);
            return ret;
        }

        // Returns the number of vertices.
        // We don't expose the number of faces in a similar manner because faces can use more than 3 vertices each,
        // and we don't know the exact amount until we parse the remaining data.
        [[nodiscard]] std::size_t VertexCount() const {return vertex_count;}

        // Returns true if the file contains a property called `name`.
        [[nodiscard]] bool HasProperty(std::string_view name) const {return properties.contains(name);}

        // Returns some information about a property, throws if no such property.
        [[nodiscard]] const Property &GetPropertyInfo(std::string_view name) const
        {
            if (auto it = properties.find(name); it != properties.end())
                return it->second;
            else
                Program::Error(FMT("No vertex property named `{}`.", name));
        }


        // Those types represent all data types supported supported in code (not necessarily in the files).
        template <typename VertexType> using func_float_t = void(*)(VertexType &, float);
        template <typename VertexType> using func_uint8_t = void(*)(VertexType &, std::uint8_t);
        template <typename VertexType> using func_variant_t = std::variant<std::monostate, func_float_t<VertexType>, func_uint8_t<VertexType>>;

        template <typename VertexType>
        struct Format
        {
            // Maps property names to functions that write them to `VertexType`.
            // Normally you don't need to modify this directly, use the `Add...` functions below.
            std::map<std::string, func_variant_t<VertexType>> funcs;

            // If true, reject faces with more than three vertices.
            // Otherwise they will be converted to sequences of triangles.
            bool allow_triangle_faces_only = false;


            // For all the functions below, `MemPtr` must be a pointer-to-member for `VertexType` to a scalar or vector (number of components must match the parameter count).
            // The type of the scalar or vector components must correspond to one of the types `funcs` can store (currently `float` or `std::uint8_t`).

            // `MemPtr` must be a data member pointer for `VertexType`. Returns the type of the corresponding member type.
            template <auto MemPtr>
            using mem_ptr_target_t = std::remove_cvref_t<decltype(std::declval<VertexType &>().*MemPtr)>;

            // Checks if `MemPtr` is a valid data member pointer of `VertexType`
            // and that the field type (or for vectors the element type) is one of the supported types.
            template <auto MemPtr>
            static constexpr bool is_valid_member_ptr_v = requires(VertexType v, func_variant_t<VertexType> func_variant)
            {
                std::get<void(*)(VertexType &, Math::vec_base_t<mem_ptr_target_t<MemPtr>>)>(func_variant);
            };

            // Assigns property names to a scalar or a vector.
            // Previous meaning of the names is discarded.
            template <auto MemPtr>
            Format &Add(std::convertible_to<std::string> auto &&... names)
            requires is_valid_member_ptr_v<MemPtr> && (sizeof...(names) == Math::vec_size_v<mem_ptr_target_t<MemPtr>>)
            {
                std::string names_array[] = {std::string(names)...};
                Meta::cexpr_for<Math::vec_size_v<mem_ptr_target_t<MemPtr>>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    funcs.insert_or_assign(std::move(names_array[i]), [](VertexType &obj, Math::vec_base_t<mem_ptr_target_t<MemPtr>> value)
                    {
                        Math::get_vec_element<i>(obj.*MemPtr) = value;
                    });
                });
                return *this;
            }

            // Shorthands for `Add()` with predefined names.
            template <auto MemPtr> Format &AddPos() {return Add<MemPtr>("x", "y", "z");}
            template <auto MemPtr> Format &AddNormal() {return Add<MemPtr>("nx", "ny", "nz");}
            template <auto MemPtr> Format &AddTexCoord() {return Add<MemPtr>("s", "t");}
            template <auto MemPtr> Format &AddColor() requires(Math::vec_size_v<mem_ptr_target_t<MemPtr>> == 3) {return Add<MemPtr>("red", "green", "blue");}
            template <auto MemPtr> Format &AddColor() requires(Math::vec_size_v<mem_ptr_target_t<MemPtr>> == 4) {return Add<MemPtr>("red", "green", "blue", "alpha");}
        };

        // Vertices and indices obtained from parsing.
        template <typename IndexType, typename VertexType>
        struct Data
        {
            std::vector<VertexType> vertices;
            std::vector<IndexType> indices;
        };

        // Finishes the file parsing started by `ParseHeader`.
        template <typename IndexType, typename VertexType>
        [[nodiscard]] Data<IndexType, VertexType> ParseBody(const Format<VertexType> &format)
        {
            static_assert(std::is_same_v<IndexType, std::uint8_t> || std::is_same_v<IndexType, std::uint16_t> || std::is_same_v<IndexType, std::uint32_t>);

            // Make a table mapping property indices to various info about them.
            struct Entry
            {
                bool property_is_uint8 = false; // Type of the property. `uint8_t` if true, `float` otherwise.
                func_variant_t<VertexType> func;
            };
            std::vector<Entry> table(properties.size());
            for (const auto &[name, func] : format.funcs)
            {
                const Property &property = GetPropertyInfo(name);
                Entry &entry = table[property.index];
                entry.property_is_uint8 = property.is_uint8;
                entry.func = func;
            }

            Data<IndexType, VertexType> ret;

            // Read vertices.
            ret.vertices.resize(vertex_count);
            for (std::size_t i = 0; i < vertex_count; i++)
            {
                VertexType &vertex = ret.vertices[i];
                // Read each property of this vertex.
                for (std::size_t j = 0; j < table.size(); j++)
                {
                    bool is_last = j == table.size() - 1;
                    char terminating_char = is_last ? '\n' : ' ';

                    const Entry &entry = table[j];

                    // Parse the property.
                    if (std::holds_alternative<std::monostate>(entry.func))
                    {
                        // Unused property, skip it.
                        input.Discard<Stream::at_least_one>(!Stream::Char::EqualTo(terminating_char));
                    }
                    else if (entry.property_is_uint8)
                    {
                        // `uint8_t` property.
                        std::uint8_t number = 0;
                        Refl::InterfaceFor(number).FromString(number, input, {}, Refl::initial_state);
                        std::visit(Meta::overload
                        {
                            [&](std::monostate) {}, // Shouldn't happen.
                            [&](func_uint8_t<VertexType> func) {func(vertex, number);}, // Exact type match.
                            [&](func_float_t<VertexType> func) {func(vertex, number / 255.f);}, // Convert to float.
                        }, entry.func);
                    }
                    else
                    {
                        // `float` property.
                        float number = 0;
                        Refl::InterfaceFor(number).FromString(number, input, {}, Refl::initial_state);
                        std::visit(Meta::overload
                        {
                            [&](std::monostate) {}, // Shouldn't happen.
                            [&](func_float_t<VertexType> func) {func(vertex, number);}, // Exact type match.
                            [&](func_uint8_t<VertexType> func) {func(vertex, clamp_max(int(clamp(number) * 255.f + 0.5f), 255));}, // Convert to `uint8_t`, with saturation.
                        }, entry.func);
                    }

                    // Skip the separator.
                    input.Discard(terminating_char);
                }
            }

            // Read indices.
            if (format.allow_triangle_faces_only)
                ret.indices.reserve(face_count * 3);
            for (std::size_t i = 0; i < face_count; i++)
            {
                std::uint8_t list_size; // This type is selected intentionally, to match the `property list ...` line in the files.
                Refl::InterfaceFor(list_size).FromString(list_size, input, {}, Refl::initial_state);
                if (format.allow_triangle_faces_only && list_size != 3)
                    Program::Error(input.GetExceptionPrefix() + STR("Expected exactly three elements in the list."));
                else if (!format.allow_triangle_faces_only && list_size < 2)
                    Program::Error(input.GetExceptionPrefix() + STR("Expected at least three elements in the list."));

                using tmp_index_t = std::uint32_t; // Should be the largest supported index type.
                tmp_index_t prev_indices[2];
                for (decltype(list_size) j = 0; j < list_size; j++)
                {
                    input.Discard(' ');
                    tmp_index_t this_index;
                    Refl::InterfaceFor(this_index).FromString(this_index, input, {}, Refl::initial_state);
                    if (!Robust::representable_as<IndexType>(this_index))
                        Program::Error(input.GetExceptionPrefix() + "The index is not representable in the target type.");

                    if (j > 1)
                    {
                        for (tmp_index_t index : prev_indices)
                            ret.indices.push_back(index);
                        ret.indices.push_back(this_index);
                    }

                    prev_indices[int(j != 0)] = this_index;
                }

                if (i != face_count - 1)
                    input.Discard('\n');
            }
            input.Discard<Stream::if_present>('\n');
            input.ExpectEnd();

            return ret;
        }


        // A shorthand for `ParseHeader` followed by `ParseBody`.
        // Prefer to use this function if you don't have any conditional logic depending on the contents of the file header.
        template <typename IndexType, typename VertexType>
        [[nodiscard]] static Data<IndexType, VertexType> Parse(Stream::Input input, const Format<VertexType> &format)
        {
            return ParseHeader(std::move(input)).ParseBody<IndexType>(format);
        }
    };
}
