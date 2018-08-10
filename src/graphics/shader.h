#ifndef GRAPHICS_SHADER_H_INCLUDED
#define GRAPHICS_SHADER_H_INCLUDED

#include <string>
#include <type_traits>
#include <utility>

#include <GLFL/glfl.h>

#include "texture.h"

#include "reflection/complete.h"
#include "program/errors.h"
#include "utils/finally.h"
#include "utils/meta.h"
#include "utils/strings.h"

namespace Graphics::Shader
{
    template <typename T> std::string GlslTypeName()
    {
        if constexpr (Math::is_vector_v<T> || Math::is_matrix_v<T>)
        {
            constexpr int is_vec = Math::is_vector_v<T>;
            using base = typename T::type;

            std::string ret;

                 if constexpr (std::is_same_v<base, bool        >) ret = "b";
            else if constexpr (std::is_same_v<base, double      >) ret = "d";
            else if constexpr (std::is_same_v<base, int         >) ret = "i";
            else if constexpr (std::is_same_v<base, unsigned int>) ret = "u";
            else static_assert(std::is_same_v<base, float>, "No name for this type.");

            if constexpr (is_vec)
            {
                ret += "vec";
                ret += std::to_string(T::size);
            }
            else
            {
                ret += "mat";
                ret += std::to_string(T::width);
                if constexpr(T::width != T::height)
                {
                    ret += "x";
                    ret += std::to_string(T::height);
                }
            }

            return ret;
        }
        else if constexpr (std::is_same_v<T, Texture     >) return "sampler2D";
        else if constexpr (std::is_same_v<T, bool        >) return "bool";
        else if constexpr (std::is_same_v<T, float       >) return "float";
        else if constexpr (std::is_same_v<T, double      >) return "double";
        else if constexpr (std::is_same_v<T, int         >) return "int";
        else if constexpr (std::is_same_v<T, unsigned int>) return "uint";
        else static_assert(!sizeof(T), "No name for this type.");
    }

    struct Config
    {
        std::string common_header = "#version 330 compatibility";
        std::string vertex_header = "/* vertex */";
        std::string fragment_header = "/* fragment */";

        std::string attribute = "attribute";
        std::string uniform = "uniform";
    };

    struct Preferences
    {
        std::string attribute_prefix = "a_";
        std::string uniform_prefix = "u_";
    };

    struct None_t {} None; // Means no attributes or no uniforms.

    template <typename T> class Uniform;

    class Program
    {
        template <typename T> friend class Uniform;

        struct Data
        {
            GLuint handle = 0;
        };
        Data data;

        inline static GLuint binding = 0;

        static void BindHandle(GLuint handle)
        {
            if (binding == handle)
                return;
            binding = handle;
            glUseProgram(handle);
        }

        template <typename T> static std::string AppendAttributesToSource(const std::string &source, const Config &cfg, const Preferences &pref)
        {
            if constexpr (std::is_void_v<T> || std::is_same_v<T, None_t>)
            {
                return source;
            }
            else
            {
                std::string header;

                using refl = Refl::Interface<T>;
                refl::for_each_field([&](auto index)
                {
                    constexpr int i = index.value;
                    using field_type = typename refl::template field_type<i>;
                    header += cfg.attribute;
                    header += ' ';
                    header += GlslTypeName<field_type>();
                    header += ' ';
                    header += pref.attribute_prefix;
                    header += refl::field_name(i);
                    header += ";\n";
                });

                return header + source;
            }
        }

        template <typename T> static std::string AppendUniformsToSource(const std::string &source, const Config &cfg, const Preferences &pref)
        {
            if constexpr (std::is_void_v<T> || std::is_same_v<T, None_t>)
            {
                return source;
            }
            else
            {
                std::string header;

                using refl = Refl::Interface<T>;
                refl::for_each_field([&](auto index)
                {
                    constexpr int i = index.value;
                    using field_type = typename refl::template field_type<i>::type;
                    header += cfg.uniform;
                    header += ' ';
                    header += GlslTypeName<std::remove_extent_t<field_type>>();
                    header += ' ';
                    header += pref.uniform_prefix;
                    header += refl::field_name(i);
                    if constexpr (std::is_array_v<field_type>)
                    {
                        header += '[';
                        header += std::to_string(std::extent_v<field_type>);
                        header += ']';
                    }
                    header += ";\n";
                });

                return header + source;
            }
        }

        template <typename T> static std::vector<std::string> MakeAttributeList()
        {
            if constexpr (std::is_void_v<T> || std::is_same_v<T, None_t>)
            {
                return {};
            }
            else
            {
                std::vector<std::string> ret;

                using refl = Refl::Interface<T>;
                refl::for_each_field([&](auto index)
                {
                    ret.push_back(refl::field_name(index.value));
                });

                return ret;
            }
        }

        template <typename T> void AssignUniformLocation(Uniform<T> &uniform, int loc);
      public:
        Program() {}

        Program(std::string name, const Config &cfg, const std::string &vert_source, const std::string &frag_source, const std::vector<std::string> &attributes = {})
        {
            data.handle = glCreateProgram();
            if (!data.handle)
                ::Program::Error("Unable to create shader program: `", name, "`.");
            FINALLY_ON_THROW( glDeleteProgram(data.handle); )

            for (std::string source : {vert_source, frag_source})
            {
                bool is_vertex = source == vert_source.c_str();

                std::string header = cfg.common_header + "\n" + (is_vertex ? cfg.vertex_header : cfg.fragment_header) + "\n";
                source = header + source;

        std::cout << "\n==================\n" << source << "\n==================\n";

                GLuint object = glCreateShader(is_vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
                if (!object)
                    ::Program::Error("Unable to create ", is_vertex ? "vertex" : "fragment", " shader object: `", name, "`.");
                FINALLY( glDeleteShader(object); ) // Note that we unconditionally delete the shader. GL keeps it alive as long as it's attached to a program.

                const char *source_ptr = source.c_str();
                glShaderSource(object, 1, &source_ptr, 0);

                glCompileShader(object);

                GLint status;
                glGetShaderiv(object, GL_COMPILE_STATUS, &status);

                if (!status) // Compilation error
                {
                    GLint log_len = 1;
                    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_len);

                    std::string log;
                    if (log_len == 0) // No log
                    {
                        log = "N/A";
                    }
                    else
                    {
                        log.resize(log_len - 1); // `log_len` includes null terminator.
                        glGetShaderInfoLog(object, log_len, 0, log.data());
                    }

                    ::Program::Error("Unable to compile ", is_vertex ? "vertex" : "fragment", " shader: `", name, "`.\nLog:\n", Strings::Trim(log));
                }

                glAttachShader(data.handle, object);
            }

            int attrib_index = 0;
            for (const std::string &attrib : attributes)
                glBindAttribLocation(data.handle, attrib_index++, attrib.c_str());

            glLinkProgram(data.handle);

            GLint status;
            glGetProgramiv(data.handle, GL_LINK_STATUS, &status);

            if (!status) // Compilation error
            {
                GLint log_len = 1;
                glGetProgramiv(data.handle, GL_INFO_LOG_LENGTH, &log_len);

                std::string log;
                if (log_len == 0) // No log
                {
                    log = "N/A";
                }
                else
                {
                    log.resize(log_len - 1); // `log_len` includes null terminator.
                    glGetProgramInfoLog(data.handle, log_len, 0, log.data());
                }

                ::Program::Error("Unable to link shader program: `", name, "`.\nLog:\n", Strings::Trim(log));
            }
        }

        template <typename AttributesT, typename UniformsT>
        Program(std::string name, const Config &cfg, const Preferences &pref, Meta::tag<AttributesT>, UniformsT &uniforms, const std::string &vert_source, const std::string &frag_source)
        : Program(name, cfg, AppendUniformsToSource<UniformsT>(AppendAttributesToSource<AttributesT>(vert_source, cfg, pref), cfg, pref),
                  AppendUniformsToSource<UniformsT>(frag_source, cfg, pref), MakeAttributeList<AttributesT>())
        {
            if constexpr (std::is_void_v<UniformsT> || std::is_same_v<UniformsT, None_t>)
            {
                (void)uniforms;
            }
            else
            {
                auto refl = Refl::Interface(uniforms);
                refl.for_each_field([&](auto index)
                {
                    constexpr int i = index.value;
                    // Note that we don't need to check the return value. Even if a uniform is not found and -1 location is returned, glUniform* silently no-op when it's used.
                    AssignUniformLocation(refl.template field_value<i>(), glGetUniformLocation(data.handle, refl.field_name(i).c_str()));
                });
            }
        }

        ~Program()
        {
            if (Bound())
                Unbind(); // GL doesn't auto-unbind shaders on deletion.
            glDeleteProgram(data.handle);
        }

        Program(Program &&other) noexcept : data(std::exchange(other.data, {})) {}
        Program &operator=(Program other) noexcept // Note the pass by value to utilize copy&swap idiom.
        {
            std::swap(data, other.data);
            return *this;
        }

        explicit operator bool() const
        {
            return bool(data.handle);
        }

        [[nodiscard]] bool Bound() const
        {
            return data.handle == binding;
        }

        GLuint Handle() const
        {
            return data.handle;
        }

        void Bind()
        {
            BindHandle(data.handle);
        }
        static void Unbind()
        {
            BindHandle(0);
        }
    };

    template <typename T> class Uniform
    {
        GLuint handle = 0;
        int location = -1;

        friend class Program;

        void modify(GLuint new_handle, int new_location)
        {
            handle = new_handle;
            location = new_location;
        }

      public:
        using type = T;
        using elem_type = std::remove_extent_t<T>;

        inline static constexpr bool
            is_array   = std::is_array_v<type>,
            is_texture = std::is_same_v<elem_type, Texture>,
            is_bool    = std::is_same_v<Math::vec_base_t<elem_type>, bool>;

        inline static constexpr int array_elements = std::extent_v<std::conditional_t<is_array, type, type[1]>>;

        using effective_elem_type = std::conditional_t<is_texture || is_bool, int, elem_type>; // Textures and bools become ints here

        using elem_base_type = Math::vec_base_t<effective_elem_type>; // Vectors and matrices become scalars here.

        Uniform() {}

        const elem_type &operator=(const elem_type &object) const // Binds the shader.
        {
            static_assert(!is_array, "Use .set() to set arrays.");

            if (!handle)
                return object;

            Program::BindHandle(handle);

            if constexpr (is_texture) glUniform1i(location, object.Slot());
            else if constexpr (std::is_same_v<effective_elem_type, float       >) glUniform1f (location, object);
            else if constexpr (std::is_same_v<effective_elem_type, fvec2       >) glUniform2f (location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_elem_type, fvec3       >) glUniform3f (location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_elem_type, fvec4       >) glUniform4f (location, object.x, object.y, object.z, object.w);
            else if constexpr (std::is_same_v<effective_elem_type, int         >) glUniform1i (location, object);
            else if constexpr (std::is_same_v<effective_elem_type, ivec2       >) glUniform2i (location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_elem_type, ivec3       >) glUniform3i (location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_elem_type, ivec4       >) glUniform4i (location, object.x, object.y, object.z, object.w);
            else if constexpr (std::is_same_v<effective_elem_type, unsigned int>) glUniform1ui(location, object);
            else if constexpr (std::is_same_v<effective_elem_type, uvec2       >) glUniform2ui(location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_elem_type, uvec3       >) glUniform3ui(location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_elem_type, uvec4       >) glUniform4ui(location, object.x, object.y, object.z, object.w);
            else set_no_bind(&object, 1);
            return object;
        }
        void set(const effective_elem_type *ptr, int count, int offset = 0) const
        {
            if (!handle)
                return;

            Program::BindHandle(handle);

            set_no_bind(ptr, count, offset);
        }
      private:
        void set_no_bind(const effective_elem_type *ptr, int count, int offset = 0) const
        {
            if (count < 0 || offset + count > array_elements)
                ::Program::Error("Invalid shader uniform array range.");
            int l = location + offset;
                 if constexpr (std::is_same_v<effective_elem_type, float       >) glUniform1fv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fvec2       >) glUniform2fv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fvec3       >) glUniform3fv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fvec4       >) glUniform4fv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, int         >) glUniform1iv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, ivec2       >) glUniform2iv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, ivec3       >) glUniform3iv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, ivec4       >) glUniform4iv (l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, unsigned int>) glUniform1uiv(l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, uvec2       >) glUniform2uiv(l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, uvec3       >) glUniform3uiv(l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, uvec4       >) glUniform4uiv(l, count, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat2       >) glUniformMatrix2fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat3       >) glUniformMatrix3fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat4       >) glUniformMatrix4fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat3x2     >) glUniformMatrix3x2fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat4x2     >) glUniformMatrix4x2fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat2x3     >) glUniformMatrix2x3fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat4x3     >) glUniformMatrix4x3fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat2x4     >) glUniformMatrix2x4fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_elem_type, fmat3x4     >) glUniformMatrix3x4fv(l, count, 0, reinterpret_cast<elem_base_type *>(ptr));
            else static_assert(std::is_void_v<effective_elem_type>, "Uniforms of this type are not supported.");
        }
    };

    template <typename T> inline void Program::AssignUniformLocation(Uniform<T> &uniform, int loc)
    {
        uniform.modify(data.handle, loc);
    }
}

#endif
