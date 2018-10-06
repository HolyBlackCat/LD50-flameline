#pragma once

#include <iosfwd>
#include <exception>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "program/errors.h"

class Json
{
  public:
    // If you decide to reorder this enum, you also have to reorder the variant below.
    enum type_t {null, boolean, num_int, num_real, string, array, object};

  private:
    using array_t = std::vector<Json>;
    using object_t = std::map<std::string, Json>;

    // If you decide to reorder this bariant, you also have to reorder the enum above.
    using variant_t = std::variant<
        std::monostate, // type_t::null
        bool,           // type_t::boolean
        int,            // type_t::num_int
        double,         // type_t::num_real
        std::string,    // type_t::string
        array_t,        // type_t::array
        object_t        // type_t::object
    >;

    variant_t variant;

    static Json FromVariant(const variant_t &var)
    {
        Json ret;
        ret.variant = var;
        return ret;
    }

    static void ParseSkipWhitespace(const char *&cur);
    static std::string ParseStringLow(const char *&cur);
    static Json ParseLow(const char *&cur, int allowed_depth);

  public:
    Json() {}
    Json(const char *string, int allowed_depth);

    class View
    {
        const Json *ptr = 0;
        std::string path;

        void ThrowExpectedType(std::string type) const
        {
            Program::Error("Expected JSON object `", path, "` to be ", type, ".");
        }

        std::string AppendElementIndexToPath(int index) const
        {
            std::string ret = path;
            ret += '[';
            ret += std::to_string(index);
            ret += ']';
            return ret;
        }
        std::string AppendElementNameToPath(std::string name) const
        {
            if (path.empty())
                return name;
            std::string ret = path;
            ret += '.';
            ret += name;
            return ret;
        }
      public:
        View() {}

        // Passed object has to remain alive.
        View(const Json &json, std::string name = "") : ptr(&json), path(std::move(name)) {}

        type_t Type() const
        {
            return type_t(ptr->variant.index());
        }

        bool IsNull()   const {return Type() == null;}
        bool IsBool()   const {return Type() == boolean;}
        bool IsInt()    const {return Type() == num_int;}
        bool IsReal()   const {return Type() == num_real || IsInt();}
        bool IsString() const {return Type() == string;}
        bool IsArray()  const {return Type() == array;}
        bool IsObject() const {return Type() == object;}

        bool GetBool() const
        {
            if (!IsBool())
                ThrowExpectedType("a boolean");
            return *std::get_if<int(boolean)>(&ptr->variant);
        }
        int GetInt() const
        {
            if (!IsInt())
                ThrowExpectedType("an integer");
            return *std::get_if<int(num_int)>(&ptr->variant);
        }
        double GetReal() const
        {
            if (IsInt())
                return GetInt();

            if (!IsReal())
                ThrowExpectedType("a real number");
            return *std::get_if<int(num_real)>(&ptr->variant);
        }
        std::string GetString() const
        {
            if (!IsString())
                ThrowExpectedType("a string");
            return *std::get_if<int(string)>(&ptr->variant);
        }

        int GetArraySize() const
        {
            if (!IsArray())
                ThrowExpectedType("an array");
            return std::get_if<int(array)>(&ptr->variant)->size();
        }
        View GetElement(int index) const
        {
            if (!IsArray())
                ThrowExpectedType("an array");
            const array_t &arr = *std::get_if<int(array)>(&ptr->variant);
            if (index < 0 || decltype(arr.size())(index) >= arr.size())
                Program::Error("Attempt to access element #", index, " of JSON object `", path, "`, but it only contains ", arr.size(), " elements.");
            return View(arr[index], AppendElementIndexToPath(index));
        }
        template <typename F> void ForEachArrayElement(F &&func) const // `func` should be `void func(const View &elem)`.
        {
            if (!IsArray())
                ThrowExpectedType("an array");
            const array_t &arr = *std::get_if<int(array)>(&ptr->variant);
            for (array_t::size_type i = 0; i < arr.size(); i++)
                func(View(arr[i], AppendElementIndexToPath(i)));
        }
        bool HasElement(int index) const
        {
            return index >= 0 && index < GetArraySize();
        }

        int GetObjectSize() const
        {
            if (!IsObject())
                ThrowExpectedType("an object");
            return std::get_if<int(object)>(&ptr->variant)->size();
        }
        View GetElement(std::string key) const
        {
            if (!IsObject())
                ThrowExpectedType("an object");
            const object_t &obj = *std::get_if<int(object)>(&ptr->variant);
            auto it = obj.find(key);
            if (it == obj.end())
                Program::Error("Attempt to access nonexistent element `", key, "` of JSON object `", path, "`.");
            return View(it->second, AppendElementNameToPath(key));
        }
        template <typename F> void ForEachObjectElement(F &&func) const // `func` should be `void func(const std::string &name, const View &elem)`.
        {
            if (!IsObject())
                ThrowExpectedType("an object");
            const object_t &obj = *std::get_if<int(object)>(&ptr->variant);
            for (const auto &elem : obj)
                func(View(elem.second, AppendElementNameToPath(elem.first)));
        }
        bool HasElement(std::string key) const
        {
            if (!IsObject())
                ThrowExpectedType("an object");
            const object_t &obj = *std::get_if<int(object)>(&ptr->variant);
            auto it = obj.find(key);
            return it != obj.end();
        }

        View operator[](int index) const // Same as GetElement(int).
        {
            return GetElement(index);
        }

        View operator[](std::string key) const // Same as GetElement(std::string).
        {
            return GetElement(key);
        }

        void DebugPrint(std::ostream &stream) const;
    };

    View GetView() const
    {
        return View(*this);
    }
};
