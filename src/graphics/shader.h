#pragma once

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

namespace Graphics
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
        else if constexpr (std::is_same_v<T, TexUnit     >) return "sampler2D";
        else if constexpr (std::is_same_v<T, bool        >) return "bool";
        else if constexpr (std::is_same_v<T, float       >) return "float";
        else if constexpr (std::is_same_v<T, double      >) return "double";
        else if constexpr (std::is_same_v<T, int         >) return "int";
        else if constexpr (std::is_same_v<T, unsigned int>) return "uint";
        else static_assert(!sizeof(T), "No name for this type.");
    }

    struct ShaderConfig
    {
        /* GLSL version chart
            OpenGL    GLSL
            2.0       110
            2.1       120
            3.0       130
            3.1       140
            3.2       150
            3.3       330
            4.0       400
            4.1       410
            4.2       420
            4.3       430
            4.4       440
            4.5       450
            4.6       460
            ES 2      100

            GLSL ES has `GL_ES` macro predefined.
            GLSL ES lacks default precision for float inside of fragment shaders.
        */

        std::string common_header = "#version 330 compatibility";
        std::string vertex_header = "/* vertex */";
        std::string fragment_header = "/* fragment */";

        std::string attribute = "attribute";
        std::string uniform = "uniform";

        static ShaderConfig Old(std::string common = "")
        {
            return
            {
                common,
                "/* vertex */",
                "/* fragment */",
                "attribute",
                "uniform",
            };
        }
        static ShaderConfig Compat(int ver = 150)
        {
            return
            {
                Str("#version ", ver, " compatibility"),
                "/* vertex */",
                "/* fragment */",
                "attribute",
                "uniform",
            };
        }
        static ShaderConfig Core(int ver = 150)
        {
            return
            {
                Str("#version ", ver),
                "/* vertex */\n#define varying out",
                "/* fragment */\n"
                    "out vec4 _gl_FragData[gl_MaxDrawBuffers];\n" // A single leading underscore is allowed, I've checked.
                    "#define gl_FragData _gl_FragData\n"
                    "#define gl_FragColor gl_FragData[0]\n"
                    "#define varying in\n"
                    "#define texture2D texture",
                "in",
                "uniform",
            };
        }
    };

    struct ShaderPreferences
    {
        std::string attribute_prefix;
        std::string uniform_prefix;

        ShaderPreferences() : ShaderPreferences("a_", "u_") {}
        ShaderPreferences(std::string a, std::string u) : attribute_prefix(a), uniform_prefix(u) {}
    };

    static constexpr struct None_t {} None; // Means no attributes or no uniforms.

    template <typename T> class Uniform;

    class Shader
    {
        template <typename T> void AssignUniformLocation(Uniform<T> &uniform, int loc);

        struct Data
        {
            GLuint handle = 0;
        };
        Data data;

        inline static GLuint binding = 0;

      public:
        static void BindHandle(GLuint handle)
        {
            if (binding == handle)
                return;
            binding = handle;
            glUseProgram(handle);
        }

        template <typename T> static std::string AppendAttributesToSource(const std::string &source, const ShaderConfig &cfg, const ShaderPreferences &pref)
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

        template <typename T> static std::string AppendUniformsToSource(const std::string &source, const ShaderConfig &cfg, const ShaderPreferences &pref, bool is_vertex)
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
                    using field_type_raw = typename refl::template field_type<i>;
                    if ((field_type_raw::is_vertex && is_vertex) || (field_type_raw::is_fragment && !is_vertex))
                    {
                        using field_type = typename field_type_raw::type;
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
                    }
                });

                return header + source;
            }
        }

        template <typename T> static std::vector<std::string> MakeAttributeList(const ShaderPreferences &pref)
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
                    ret.push_back(pref.attribute_prefix + refl::field_name(index.value));
                });

                return ret;
            }
        }


        Shader(decltype(nullptr)) {}

        Shader(std::string name, const ShaderConfig &cfg, std::string vert_source, std::string frag_source, const std::vector<std::string> &attributes = {})
        {
            data.handle = glCreateProgram();
            if (!data.handle)
                Program::Error("Unable to create shader program: `", name, "`.");
            FINALLY_ON_THROW( glDeleteProgram(data.handle); )

            for (std::string *source_ptr : {&vert_source, &frag_source})
            {
                bool is_vertex = source_ptr == &vert_source;
                std::string &source = *source_ptr;

                std::string header = cfg.common_header + "\n" + (is_vertex ? cfg.vertex_header : cfg.fragment_header) + "\n";
                source = header + source;

                // Uncomment to dump source:
                // std::cout << "\n==================\n" << source << "\n==================\n";

                GLuint object = glCreateShader(is_vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
                if (!object)
                    Program::Error("Unable to create ", is_vertex ? "vertex" : "fragment", " shader object: `", name, "`.");
                FINALLY( glDeleteShader(object); ) // Note that we unconditionally delete the shader. GL keeps it alive as long as it's attached to a program.

                const char *source_bytes = source.c_str();
                glShaderSource(object, 1, &source_bytes, 0);

                glCompileShader(object);

                GLint status;
                glGetShaderiv(object, GL_COMPILE_STATUS, &status);

                if (!status) // Compilation error
                {
                    GLint log_len = 0;
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

                    Program::Error("Unable to compile ", is_vertex ? "vertex" : "fragment", " shader: `", name, "`.\nLog:\n", Strings::Trim(log));
                }

                glAttachShader(data.handle, object);
            }

            int attrib_index = 0;
            for (const std::string &attrib : attributes)
                glBindAttribLocation(data.handle, attrib_index++, attrib.c_str());

            glLinkProgram(data.handle);

            GLint status;
            glGetProgramiv(data.handle, GL_LINK_STATUS, &status);

            if (!status) // Linking error
            {
                GLint log_len = 0;
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

                Program::Error("Unable to link shader program: `", name, "`.\nLog:\n", Strings::Trim(log));
            }
        }

        template <typename AttributesT, typename UniformsT>
        Shader(std::string name, const ShaderConfig &cfg, const ShaderPreferences &pref, Meta::tag<AttributesT>, UniformsT &uniforms, const std::string &vert_source, const std::string &frag_source)
            : Shader(name, cfg, AppendUniformsToSource<UniformsT>(AppendAttributesToSource<AttributesT>(vert_source, cfg, pref), cfg, pref, 1),
                      AppendUniformsToSource<UniformsT>(frag_source, cfg, pref, 0), MakeAttributeList<AttributesT>(pref))
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
                    // Note that we don't need to check the return value. Even if a uniform is not found and -1 location is returned, glUniform* will silently no-op on it.
                    AssignUniformLocation(refl.template field_value<i>(), glGetUniformLocation(data.handle, (pref.uniform_prefix + refl.field_name(i)).c_str()));
                });
            }
        }

        ~Shader()
        {
            if (Bound())
                Unbind(); // GL doesn't auto-unbind shaders on deletion, and keeps them alive as long as they're bound.
            glDeleteProgram(data.handle); // Deleting 0 is a no-op.
        }

        Shader(Shader &&other) noexcept : data(std::exchange(other.data, {})) {}
        Shader &operator=(Shader other) noexcept // Note the pass by value to utilize copy&swap idiom.
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
            return data.handle && data.handle == binding;
        }

        GLuint Handle() const
        {
            return data.handle;
        }

        void Bind() const
        {
            if (!*this)
                return;
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

        friend class Shader;

        void modify(GLuint new_handle, int new_location)
        {
            handle = new_handle;
            location = new_location;
        }

      public:
        // Derived classes override those.
        static constexpr bool is_vertex = 1;
        static constexpr bool is_fragment = 1;

        using type_with_extent = T;
        using type = std::remove_extent_t<T>;

        static_assert(!std::is_same_v<type, TexObject> && !std::is_same_v<type, Texture>, "Use `TexUnit` template parameter for texture uniforms.");

        inline static constexpr bool
            is_array   = std::is_array_v<type_with_extent>,
            is_texture = std::is_same_v<type, TexUnit>,
            is_bool    = std::is_same_v<Math::vec_base_t<type>, bool>;

        inline static constexpr int array_elements = std::extent_v<std::conditional_t<is_array, type_with_extent, type_with_extent[1]>>;

        using effective_type = std::conditional_t<is_texture || is_bool, int, type>; // Textures and bools become ints here

        using base_type = typename std::conditional_t<Math::is_scalar_v<effective_type>, std::enable_if<1, effective_type>, effective_type>::type; // Vectors and matrices become scalars here.


        Uniform() {}

        const type &operator=(const type &object) const // Binds the shader.
        {
            static_assert(!is_array, "Use .set() to set arrays.");

            // We don't need to check location here. glUniform* silently ignores `-1`.

            if (!handle)
                return object;

            Shader::BindHandle(handle);

                 if constexpr (is_texture) glUniform1i(location, object.Index());
            else if constexpr (std::is_same_v<effective_type, float       >) glUniform1f (location, object);
            else if constexpr (std::is_same_v<effective_type, fvec2       >) glUniform2f (location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_type, fvec3       >) glUniform3f (location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_type, fvec4       >) glUniform4f (location, object.x, object.y, object.z, object.w);
            else if constexpr (std::is_same_v<effective_type, int         >) glUniform1i (location, object);
            else if constexpr (std::is_same_v<effective_type, ivec2       >) glUniform2i (location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_type, ivec3       >) glUniform3i (location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_type, ivec4       >) glUniform4i (location, object.x, object.y, object.z, object.w);
            OnPlatform(PC)(
            else if constexpr (std::is_same_v<effective_type, unsigned int>) glUniform1ui(location, object);
            else if constexpr (std::is_same_v<effective_type, uvec2       >) glUniform2ui(location, object.x, object.y);
            else if constexpr (std::is_same_v<effective_type, uvec3       >) glUniform3ui(location, object.x, object.y, object.z);
            else if constexpr (std::is_same_v<effective_type, uvec4       >) glUniform4ui(location, object.x, object.y, object.z, object.w);
            )
            else set_no_bind(&object, 1);
            return object;
        }
        void set(const effective_type *ptr, int count, int offset = 0) const
        {
            // We don't need to check location here. glUniform* silently ignores `-1`.

            if (!handle)
                return;

            Shader::BindHandle(handle);

            set_no_bind(ptr, count, offset);
        }
      private:
        void set_no_bind(const effective_type *ptr, int count, int offset = 0) const
        {
            if (count < 0 || offset < 0 || offset + count > array_elements)
                Program::Error("Invalid shader uniform array range.");
            int l = location + offset;
                 if constexpr (std::is_same_v<effective_type, float       >) glUniform1fv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fvec2       >) glUniform2fv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fvec3       >) glUniform3fv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fvec4       >) glUniform4fv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, int         >) glUniform1iv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, ivec2       >) glUniform2iv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, ivec3       >) glUniform3iv (l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, ivec4       >) glUniform4iv (l, count, reinterpret_cast<const base_type *>(ptr));
            OnPlatform(PC)(
            else if constexpr (std::is_same_v<effective_type, unsigned int>) glUniform1uiv(l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, uvec2       >) glUniform2uiv(l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, uvec3       >) glUniform3uiv(l, count, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, uvec4       >) glUniform4uiv(l, count, reinterpret_cast<const base_type *>(ptr));
            )
            else if constexpr (std::is_same_v<effective_type, fmat2       >) glUniformMatrix2fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat3       >) glUniformMatrix3fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat4       >) glUniformMatrix4fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            OnPlatform(PC)(
            else if constexpr (std::is_same_v<effective_type, fmat3x2     >) glUniformMatrix3x2fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat4x2     >) glUniformMatrix4x2fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat2x3     >) glUniformMatrix2x3fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat4x3     >) glUniformMatrix4x3fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat2x4     >) glUniformMatrix2x4fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            else if constexpr (std::is_same_v<effective_type, fmat3x4     >) glUniformMatrix3x4fv(l, count, 0, reinterpret_cast<const base_type *>(ptr));
            )
            else static_assert(std::is_void_v<effective_type>, "Uniforms of this type are not supported.");
        }
    };

    template <typename T> class VertUniform : public Uniform<T>
    {
      public:
        static constexpr bool is_fragment = 0;
        using Uniform<T>::Uniform;
        using Uniform<T>::operator=;
    };

    template <typename T> class FragUniform : public Uniform<T>
    {
      public:
        static constexpr bool is_vertex = 0;
        using Uniform<T>::Uniform;
        using Uniform<T>::operator=;
    };

    template <typename T> inline void Shader::AssignUniformLocation(Uniform<T> &uniform, int loc)
    {
        uniform.modify(data.handle, loc);
    }
}
