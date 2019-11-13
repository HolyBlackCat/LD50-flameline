#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "program/errors.h"
#include "utils/byte_order.h"
#include "utils/memory_access.h"
#include "utils/memory_file.h"
#include "utils/meta.h"
#include "utils/strings.h"
#include "utils/unicode.h"

namespace Stream
{
    enum PositionCategory
    {
        absolute,
        relative,
        end,
    };

    enum LocationStyle
    {
        none,
        byte_offset,
        text_position,
        text_byte_position,
    };

    class Input
    {
        struct Data
        {
            MemoryFile file;
            std::size_t position = 0;
            LocationStyle location_style = none;
        };
        Data data;

      public:
        Input() {}

        Input(MemoryFile file, LocationStyle location_style = none)
        {
            data.file = std::move(file);
            data.location_style = location_style;
        }

        Input(Input &&other) noexcept : data(std::exchange(other.data, {})) {}
        Input &operator=(Input &&other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        // Returns a name of the data source the stream is bound to.
        std::string GetTarget() const
        {
            return data.file.name();
        }

        // Returns a string describing current location in the stream.
        // This function can be costly for some location flavors.
        std::string GetLocation() const
        {
            switch (data.location_style)
            {
              case none:
              default:
                return "";
              case byte_offset:
                return Str("offset 0x", std::hex, std::uppercase, data.position);
              case text_position:
              case text_byte_position:
                return Strings::GetSymbolPosition(data.file.data_char(), data.file.data_char() + data.position).ToString();
            }
        }

        // Uses `GetLocationString` to a prefix for exception messages.
        std::string GetExceptionPrefix() const
        {
            std::string ret = "In an input stream bound to `" + GetTarget() + "`";

            if (std::string loc = GetLocation(); loc.size() > 0)
            {
                ret += ", at " + loc;
            }

            ret += ": ";
            return ret;
        }

        void Seek(std::ptrdiff_t offset, PositionCategory category)
        {
            std::size_t base_pos =
                category == relative ? data.position    :
                category == end      ? data.file.size() : 0;

            std::size_t new_pos = base_pos + offset;
            if (new_pos >= data.file.size())
                Program::Error(GetExceptionPrefix() + "Cursor position is out of bounds.");
        }
    };
}
