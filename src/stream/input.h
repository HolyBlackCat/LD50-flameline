#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "macros/check.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "stream/better_fopen.h"
#include "stream/readonly_data.h"
#include "stream/utils.h"
#include "strings/common.h"
#include "strings/escape.h"
#include "strings/symbol_position.h"
#include "utils/bit_manip.h"
#include "utils/byte_order.h"
#include "utils/memory_access.h"
#include "utils/robust_math.h"
#include "utils/unicode.h"

namespace Stream
{
    namespace Char
    {
        // A base class for character categories.
        struct Category
        {
            [[nodiscard]] virtual bool operator()(char ch) const = 0;
            [[nodiscard]] virtual std::string name() const = 0;
        };

        // A category matching a single character.
        class EqualTo final : public Category
        {
            char saved_char = 0;

          public:
            EqualTo(char ch) : saved_char(ch) {}

            [[nodiscard]] bool operator()(char ch) const override
            {
                return ch == saved_char;
            }
            [[nodiscard]] std::string name() const override
            {
                return "`" + Strings::Escape(saved_char) + "`";
            }
        };

        // A generic character category.
        // Usage: `Is("fancy character", [](char ch){return condition;})`
        template <typename F, CHECK(std::is_convertible_v<decltype(std::declval<F>()(char())), bool>)>
        class Is final : public Category
        {
            F &&func;
            const char *name_str;

          public:
            Is(const char *name, F &&func) : func(std::move(func)), name_str(name) {}

            [[nodiscard]] bool operator()(char ch) const override
            {
                return func(ch);
            }
            [[nodiscard]] std::string name() const override
            {
                return name_str;
            }
        };


        // Some character categories.

        #define CHAR_CATEGORY(class_name_, string_, expr_) \
            struct class_name_ final : Category \
            { \
                [[nodiscard]] bool operator()(char ch) const override {return expr_;} \
                [[nodiscard]] std::string name() const override {return string_;} \
            };

        // Character categories corresponding to the functions from `<cctype>`:

        #define CHAR_CATEGORY_STD(class_name_, func_, string_) CHAR_CATEGORY(class_name_, string_, std::func_((unsigned char)ch))
        // 0-31, 127
        CHAR_CATEGORY_STD( IsControl      , iscntrl  , "a control character"     )
        // !IsControl
        CHAR_CATEGORY_STD( IsNotControl   , isprint  , "a non-control character" )
        // space, \r, \n, \t, \v (vertical tab), \f (form feed)
        CHAR_CATEGORY_STD( IsWhitespace   , isspace  , "a whitespace"            )
        // space, \t
        CHAR_CATEGORY_STD( IsSpaceOrTab   , isblank  , "a space or a tab"        )
        // !IsControl and not a space
        CHAR_CATEGORY_STD( IsVisible      , isgraph  , "a visible character"     )
        // a-z,A-Z
        CHAR_CATEGORY_STD( IsAlpha        , isalpha  , "a letter"                )
        // 0-9
        CHAR_CATEGORY_STD( IsDigit        , isdigit  , "a digit"                 )
        // 0-9,a-f,A-F
        CHAR_CATEGORY_STD( IsHexDigit     , isxdigit , "a hexadecimal digit"     )
        // IsAlpha || IsDigit
        CHAR_CATEGORY_STD( IsAlphaOrDigit , isalnum  , "a letter or a digit"     )
        // IsVisible && !IsAlphaOrDigit
        CHAR_CATEGORY_STD( IsPunctuation  , ispunct  , "a punctuation character" )
        // A-Z
        CHAR_CATEGORY_STD( IsUppercase    , isupper  , "an uppercase letter"     )
        // a-z
        CHAR_CATEGORY_STD( IsLowercase    , islower  , "a lowercase letter"      )
        #undef CHAR_CATEGORY_STD

        #undef CHAR_CATEGORY


        // Fancy stateful character categories.

        // Matches a c-style identifier.
        class SeqIdentifier final : public Category
        {
            mutable bool first_char = true;

          public:
            [[nodiscard]] bool operator()(char ch) const override
            {
                bool ok = IsAlpha{}(ch) || ch == '_' || (!first_char && IsDigit{}(ch));
                first_char = false;
                return ok;
            }

            [[nodiscard]] std::string name() const override {return "an identifier";}
        };
    }

    namespace impl
    {
        template <typename T>
        using detect_appendable_byte_seq = decltype(
            std::declval<T&>().push_back(std::uint8_t{}),
            void()
        );

        template <typename T>
        inline constexpr bool is_appendable_byte_seq_v = Meta::is_detected<impl::detect_appendable_byte_seq, T>;

        template <typename T>
        inline constexpr bool is_appendable_byte_seq_ptr_or_null_v = std::is_null_pointer_v<T> || (std::is_pointer_v<T> && is_appendable_byte_seq_v<std::remove_pointer_t<T>>);
    }

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

    enum ExtractMode
    {
        one, // Exactly one.
        any, // Any amount.
        if_present, // If present.
        at_least_one, // At least one.
    };

    class Input
    {
      public:
        static constexpr capacity_t default_capacity = capacity_t(512); // This is what `FILE *` appears to use by default.

        // Retrieves bytes from the underlying object.
        // Can throw on failure.
        // Will never be copied. If your functor is non-copyable, consider using `Meta::fake_copyable`.
        using read_func_t = std::function<void(Input &stream, std::size_t offset, std::size_t size, std::uint8_t *dst)>;

      private:
        struct Buffer
        {
            std::uint8_t *storage = nullptr;
            std::size_t position = -1; // `-1` means 'unassigned'. Otherwise lower `log2(capacity)` bits must be set to 0.

            std::uint8_t ReadByte(std::size_t byte_pos) const
            {
                return storage[byte_pos - position];
            }

            void Read(std::size_t pos, std::size_t size, std::uint8_t *target) const
            {
                std::copy_n(storage + (pos - position), size, target);
            }
        };

        struct Data
        {
            std::size_t buffer_capacity = -1; // This MUST be a power of two. 0 means that the stream is null.
            std::unique_ptr<std::uint8_t[]> buffer_storage;
            Buffer buffer_a, buffer_b;
            bool last_accessed_buffer_is_b = true;

            read_func_t read;
            std::size_t position = 0;
            std::size_t size = 0; // This value must be representable as `ptrdiff_t`.

            std::optional<LocationStyle> location_style;

            std::string name;

            ReadOnlyData readonly_data_storage; // Optional. Set if the stream is based on a ReadOnlyData.
        };
        Data data;


        struct FileHandleInfo
        {
            std::size_t pos, size;
        };
        // Collect some information about a file handle. Throws on failure.
        static FileHandleInfo CollectFileHandleInfo(FILE *handle, bool file_is_pristine)
        {
            FileHandleInfo ret;

            long file_pos_raw = 0;

            if (!file_is_pristine)
            {
                std::ftell(handle);
                if (file_pos_raw < 0)
                    Program::Error("Unable to get current position in the file.");
            }

            if (std::fseek(handle, 0, SEEK_END))
                Program::Error("Unable to seek in the file.");

            auto file_size_raw = std::ftell(handle);
            if (file_size_raw < 0)
                Program::Error("Unable to get the size of the file.");
            if (Robust::conversion_fails(file_size_raw, ret.size))
                Program::Error("File is too large.");

            // We don't need to check representability here, since we already did it for `file_size_raw`.
            ret.pos = std::min(file_pos_raw, file_size_raw);

            if (std::fseek(handle, file_pos_raw, SEEK_SET))
                Program::Error("Unable to seek in the file.");

            return ret;
        }

        // This is used to simplify writing functors for reading from files.
        // Does everything a proper functor should do. Throws on failure.
        static void FileHandleReader(FILE *handle, std::size_t &last_offset, Input &stream, std::size_t offset, std::size_t size, std::uint8_t *dst)
        {
            if (last_offset != offset && std::fseek(handle, offset, SEEK_SET))
                Program::Error(stream.GetExceptionPrefix(), "Unable to seek in the file.");

            if (std::fread(dst, size, 1, handle) != 1)
            {
                if (std::feof(handle))
                    Program::Error(stream.GetExceptionPrefix(), "Unable to read from the file: EOF was reported.");
                else
                    Program::Error(stream.GetExceptionPrefix(), "Unable to read from the file: ", std::strerror(errno));
            }

            last_offset = offset + size;
        }

        // Rounds the position down to a multiple of the buffer capacity.
        std::size_t PositionToSegmentOffset(std::size_t pos)
        {
            return pos & ~(data.buffer_capacity - std::size_t(1));
        }

        // `segment_offset` must be a multiple of the buffer capacity.
        // Unconditionally overwrites the specified buffer with new data.
        // Returns the reference to that buffer.
        Buffer &LoadSegmentToBuffer(bool use_buffer_b, std::size_t segment_offset)
        {
            Buffer &buffer = use_buffer_b ? data.buffer_b : data.buffer_a;

            std::size_t segment_size = std::min(data.size - segment_offset, data.buffer_capacity);
            data.read(*this, segment_offset, segment_size, buffer.storage);

            buffer.position = segment_offset;

            return buffer;
        }

        // `segment_offset` must be a multiple of the buffer capacity.
        // If the segment with the specified offset is already loaded, changes `last_accessed_buffer_is_b` to indicate that buffer.
        // Otherwise overwrites one of the two buffers (the one that was accessed less recently that the other) with the new data, and
        // changes `last_accessed_buffer_is_b` to indicate which buffer was overwritten.
        Buffer &NeedSegment(std::size_t segment_offset)
        {
            if (data.buffer_a.position == segment_offset)
            {
                data.last_accessed_buffer_is_b = false;
                return data.buffer_a;
            }
            if (data.buffer_b.position == segment_offset)
            {
                data.last_accessed_buffer_is_b = true;
                return data.buffer_b;
            }

            bool use_buffer_b = !data.last_accessed_buffer_is_b;
            Buffer &buffer = LoadSegmentToBuffer(use_buffer_b, segment_offset);
            data.last_accessed_buffer_is_b = use_buffer_b;

            return buffer;
        }

        void ThrowIfNoData(std::size_t bytes)
        {
            if (data.position + bytes > data.size)
                Program::Error(GetExceptionPrefix() + "Unexpected end of input.");
        }

      public:
        // Constructs an empty stream.
        Input() {}

        // Constructs a custom stream.
        // `buffer_capacity` is rounded down to the nearest positive power of two.
        Input(std::string name, std::size_t size, read_func_t read_func, capacity_t buffer_capacity = default_capacity)
        {
            if (Robust::not_representable_as<std::ptrdiff_t>(size))
                Program::Error("Unable to create an input stream `", name, "`: the specified size is too large.");

            data.name = std::move(name);
            data.size = size;
            data.read = std::move(read_func);
            data.buffer_capacity = BitManip::RoundDownToPositivePowerOfTwo(std::size_t(buffer_capacity));
            data.buffer_storage = std::make_unique<std::uint8_t[]>(data.buffer_capacity * 2);
            data.buffer_a.storage = data.buffer_storage.get();
            data.buffer_b.storage = data.buffer_storage.get() + data.buffer_capacity;
        }

        // Attaches the stream to a ReadOnlyData.
        Input(ReadOnlyData source)
        {
            if (!source)
                Program::Error("Attempt to bind an input stream to a null ReadOnlyData.");

            data.name = source.name();
            data.size = source.size();
            // The largest power-of-two capacity we can get.
            // It will always be larger than `source.size()`, because the latter must be representable as `std::ptrdiff_t`.
            data.buffer_capacity = std::numeric_limits<std::size_t>::max() / 2 + 1;
            data.buffer_a.position = 0;
            data.buffer_a.storage = const_cast<std::uint8_t *>(source.data()); // Since our functor is a null, this is safe.

            data.readonly_data_storage = std::move(source);
        }

        // Attaches the stream to a file handle (without taking ownership).
        // The current position of the new stream is set to match the current position in the file,
        // but after that the stream expects the position in the file to not change between reads.
        Input(std::string name, FILE *handle, capacity_t buffer_capacity = default_capacity)
        {
            FileHandleInfo info;

            try
            {
                info = CollectFileHandleInfo(handle, false);
            }
            catch (std::exception &e)
            {
                Program::Error("Unable to attach an input stream `", name, "` to a file handle ", (void *)handle, ":\n", e.what());
            }

            auto lambda = [handle, last_offset = info.pos](Input &stream, std::size_t offset, std::size_t size, std::uint8_t *dst) mutable
            {
                FileHandleReader(handle, last_offset, stream, offset, size, dst);
            };

            *this = Input(std::move(name), info.size, std::move(lambda), buffer_capacity);

            Seek(info.pos, absolute);
        }

        // Attaches the stream to a file.
        Input(std::string file_name, capacity_t buffer_capacity = default_capacity)
        {
            auto deleter = [](FILE *file)
            {
                // We don't check for errors here, since there is nothing we could do.
                // And `file` is always closed anyway, even if `fclose` doesn't return `0`
                std::fclose(file);
            };

            std::unique_ptr<FILE, decltype(deleter)> handle(better_fopen(file_name.c_str(), "rb"));
            if (!handle)
                Program::Error("Unable to open `", file_name, "` for reading.");

            // This function can fail, but it doesn't report errors in any way.
            // Even if it did, we would still ignore it.
            std::setbuf(handle.get(), 0);

            FileHandleInfo info;

            try
            {
                info = CollectFileHandleInfo(handle.get(), true);
            }
            catch (std::exception &e)
            {
                Program::Error("Unable to attach an input stream to `", file_name, "`:\n", e.what());
            }

            auto lambda = Meta::fake_copyable([handle = std::move(handle), last_offset = (std::size_t)0](Input &stream, std::size_t offset, std::size_t size, std::uint8_t *dst) mutable
            {
                FileHandleReader(handle.get(), last_offset, stream, offset, size, dst);
            });

            *this = Input(std::move(file_name), info.size, std::move(lambda), buffer_capacity);
        }

        // Attaches the stream to a file.
        // Without this helper, `Input(ReadOnlyData source)` would cause an ambiguity.
        Input(const char *file_name, capacity_t buffer_capacity = default_capacity) : Input(std::string(file_name), buffer_capacity) {}

        // Attaches the stream to a file.
        // Without this helper, `Input(ReadOnlyData source)` would cause an ambiguity.
        Input(std::string_view file_name, capacity_t buffer_capacity = default_capacity) : Input(std::string(file_name), buffer_capacity) {}

        Input(Input &&other) noexcept : data(std::exchange(other.data, {})) {}
        Input &operator=(Input other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.buffer_capacity);
        }

        // Returns a name of the data source the stream is bound to.
        [[nodiscard]] std::string GetTarget() const
        {
            return data.name;
        }

        // Does nothing if a style is already selected.
        Input &WantLocationStyle(LocationStyle style)
        {
            if (!data.location_style)
                data.location_style = style;

            return *this;
        }

        // Returns `none` by default.
        [[nodiscard]] LocationStyle GetLocationStyle() const
        {
            return data.location_style.value_or(none);
        }

        // Returns a string describing current location in the stream.
        // This function can be costly for some location flavors, use it wisely.
        // It's not `const` because it might need to read parts of the file.
        [[nodiscard]] std::string GetLocation()
        {
            auto style = GetLocationStyle();
            switch (style)
            {
              case none:
              default:
                return "";
              case byte_offset:
                return Str("offset 0x", std::hex, std::uppercase, data.position);
              case text_position:
              case text_byte_position:
                {
                    std::size_t old_pos = Position();
                    Seek(0, absolute);
                    FINALLY( Seek(old_pos, absolute); ) // Roll back to the original posiiton, in case we end up in the middle of a multibyte character, or something throws.

                    Strings::SymbolPosition pos;
                    Strings::SymbolPosition::State pos_state;

                    if (style == text_byte_position)
                    {
                        while (Position() < old_pos)
                            pos.AddSymbol(ReadChar(), pos_state);
                    }
                    else
                    {
                        while (Position() < old_pos)
                            pos.AddSymbol(ReadUnicodeChar(), pos_state);
                    }

                    return pos.ToString();
                }
            }
        }

        // Uses `GetLocationString` to construct a prefix for exception messages.
        [[nodiscard]] std::string GetExceptionPrefix()
        {
            std::string ret = "In an input stream bound to `" + GetTarget() + "`";

            if (std::string loc = GetLocation(); loc.size() > 0)
                ret += ", at " + loc;

            ret += ":\n";
            return ret;
        }

        // Reads all available data to memory and returns it. The result is not cached, so avoid calling this function more than once.
        // If the stream was created from a `ReadOnlyData`, doesn't copy anything and returns the internal object of this type.
        [[nodiscard]] ReadOnlyData ReadToMemory()
        {
            if (!*this)
                return {};

            if (data.readonly_data_storage)
                return data.readonly_data_storage;

            return ReadOnlyData::copy_from_function(data.name, data.size, [&](std::uint8_t *dst)
            {
                data.read(*this, 0, data.size, dst);
            });
        }
        // Reads all available data to memory. Then the underlying data source is closed, and the stream starts to operate on memory instead.
        ReadOnlyData CacheToMemory()
        {
            if (data.readonly_data_storage)
                return data.readonly_data_storage;

            auto readonly_data = ReadToMemory();

            std::size_t pos = Position();
            *this = Input(readonly_data);
            Seek(pos, absolute);

            return readonly_data;
        }

        // File size. This should always be representable as `ptrdiff_t`.
        [[nodiscard]] std::size_t Size() const
        {
            return data.size;
        }

        // Moves the cursor.
        // Throws if it ends up out of bounds.
        void Seek(std::ptrdiff_t offset, PositionCategory category)
        {
            std::size_t base_pos =
                category == relative ? data.position :
                category == end      ? data.size     : 0;

            std::size_t new_pos = base_pos + offset;

            // Note the `>` rather than `>=`. Pointing to a single byte past the end of the file is allowed.
            // Note that we don't check for an overflow here. It shouldn't be necessary,
            // since we require `data.size` to be representable as `ptrdiff_t`.
            if (new_pos > data.size)
                Program::Error(GetExceptionPrefix() + "Cursor position is out of bounds.");

            data.position = new_pos;
        }

        // Returns the current byte offset of the cursor.
        [[nodiscard]] std::size_t Position() const
        {
            return data.position;
        }

        // Returns the amount of remaining bytes.
        [[nodiscard]] std::size_t RemainingBytes() const
        {
            return data.size - data.position;
        }

        // Checks if the stream has more data available at the current cursor position.
        [[nodiscard]] bool MoreData() const
        {
            return data.position < data.size;
        }

        // Throws if there is more data available at the current cursor position.
        void ExpectEnd()
        {
            if (MoreData())
                Program::Error(GetExceptionPrefix() + "Unexpected junk at the end of input.");
        }

        // Returns the next byte, without advancing the cursor.
        [[nodiscard]] std::uint8_t PeekByte()
        {
            ThrowIfNoData(1);
            return NeedSegment(PositionToSegmentOffset(data.position)).ReadByte(data.position);
        }
        [[nodiscard]] char PeekChar()
        {
            return PeekByte();
        }

        // Reads a single byte.
        [[nodiscard]] std::uint8_t ReadByte()
        {
            std::uint8_t ret = PeekByte();
            data.position++; // If this would go out of bounds, `PeekByte()` would throw.
            return ret;
        }
        [[nodiscard]] char ReadChar()
        {
            return ReadByte();
        }

        // Reads a single UTF8 character.
        // Shouldn't throw if the encoding is incorrect, instead `Unicode::default_char` will be returned.
        [[nodiscard]] Unicode::Char ReadUnicodeChar()
        {
            std::uint8_t first = ReadByte();
            int len = Unicode::FirstByteToCharacterLength(first);

            // Check if the first byte is valid.
            if (len == 0)
            {
                while (MoreData() && !Unicode::IsFirstByte(PeekByte()))
                    SkipOne();
                return Unicode::default_char;
            }

            // Check if the character occupies a single-byte.
            if (len == 1)
                return first;

            // Extract bits from the first byte.
            Unicode::Char ret = first & (0x7f >> len); // `0xff` would also work.

            // Read remaining bytes.
            for (int i = 1; i < len; i++)
            {
                if (!MoreData())
                    return Unicode::default_char;

                std::uint8_t byte = PeekByte();

                // Make sure it's not a first byte of some character.
                if (Unicode::IsFirstByte(byte))
                    return Unicode::default_char;

                // Now we can safely move the cursor.
                SkipOne();

                // Extract bits from the byte and append them to the result.
                ret = (ret << 6) | (byte & 0b00111111);
            }

            return ret;
        }

        // Reads a sequence of bytes.
        void Read(std::uint8_t *buffer, std::size_t size)
        {
            if (size == 0)
                return;
            ThrowIfNoData(size);

            std::size_t first_segment = PositionToSegmentOffset(data.position);
            std::size_t last_segment = PositionToSegmentOffset(data.position + size - 1);

            if (first_segment == last_segment)
            {
                // The entire range fits into a single segment, load it.
                NeedSegment(first_segment).Read(data.position, size, buffer);
            }
            else
            {
                // Otherwise load the first and the last segments, and use a single unbuffered read for everything in between.
                std::size_t second_segment = first_segment + data.buffer_capacity;

                std::size_t first_size = second_segment - data.position;
                NeedSegment(first_segment).Read(data.position, first_size, buffer);

                data.read(*this, second_segment, last_segment - second_segment, buffer + first_size);

                std::size_t last_size = data.position + size - last_segment;
                NeedSegment(last_segment).Read(last_segment, last_size, buffer + size - last_size);
            }

            data.position += size;
        }
        void Read(char *buffer, std::size_t size)
        {
            Read(reinterpret_cast<std::uint8_t *>(buffer), size);
        }

        // Skips one byte.
        void SkipOne()
        {
            Seek(1, relative);
        }
        // Skips several bytes.
        void Skip(std::size_t count)
        {
            Seek(count, relative);
        }

        // Reads a single arithmetic value with a specified byte order.
        template <typename T>
        [[nodiscard]] T ReadWithByteOrder(ByteOrder::Order order)
        {
            T ret;
            Read(reinterpret_cast<std::uint8_t *>(&ret), sizeof ret);
            ByteOrder::Convert(ret, order);
            return ret;
        }
        template <typename T>
        [[nodiscard]] T ReadLittle()
        {
            return ReadWithByteOrder<T>(ByteOrder::little);
        }
        template <typename T>
        [[nodiscard]] T ReadBig()
        {
            return ReadWithByteOrder<T>(ByteOrder::big);
        }
        template <typename T>
        [[nodiscard]] T ReadNative()
        {
            return ReadWithByteOrder<T>(ByteOrder::native);
        }

        // Reads a sequence of arithmetic values with a specified byte order.
        template <typename T>
        void ReadWithByteOrder(ByteOrder::Order order, T *buffer, std::size_t count)
        {
            Read(reinterpret_cast<std::uint8_t *>(buffer), count * sizeof *buffer);
            for (std::size_t i = 0; i < count; i++)
                ByteOrder::Convert(buffer[i], order);
        }
        template <typename T>
        void ReadLittle(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::little, buffer, count);
        }
        template <typename T>
        void ReadBig(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::big, buffer, count);
        }
        template <typename T>
        void ReadNative(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::native, buffer, count);
        }

        // Reads matching characters from the input.
        // `mode` affects how many characters are read, and whether or not reading 0 characters causes an exception.
        // If `append_to` is not `nullptr`, the matching characters are appended to it.
        // Returns the amount of characters processed.
        template <ExtractMode mode = at_least_one, typename T, CHECK(impl::is_appendable_byte_seq_ptr_or_null_v<T>)>
        std::size_t Extract(const Char::Category &category, T append_to) // `append_to` can be null.
        {
            constexpr bool several = mode == at_least_one || mode == any;
            constexpr bool throw_if_none = mode == at_least_one || mode == one;

            std::size_t count = 0;

            do
            {
                if (!MoreData())
                    break;
                std::uint8_t byte = PeekByte();
                if (!category(byte))
                    break;
                SkipOne();
                if constexpr (!std::is_null_pointer_v<T>)
                    if (append_to)
                        append_to->push_back(byte);
                count++;
            }
            while (several);

            if (throw_if_none && count == 0)
                Program::Error(GetExceptionPrefix() + "Expected " + category.name() + ".");

            return count;
        }
        template <ExtractMode mode = at_least_one, typename T, CHECK(impl::is_appendable_byte_seq_ptr_or_null_v<T>)>
        std::size_t Extract(std::uint8_t byte, T append_to)
        {
            return Extract<mode>(Char::EqualTo(byte), append_to);
        }

        // Reads matching characters from the input.
        // `mode` affects whether or not reading 0 characters causes an exception.
        // Returns the matching characters in a container of type `C`.
        template <ExtractMode mode = at_least_one, typename C = std::string, CHECK(impl::is_appendable_byte_seq_v<C>)>
        [[nodiscard]] C Extract(const Char::Category &category)
        {
            static_assert(mode == at_least_one || mode == any, "Mode has to be `at_least_one` or `any`.");
            C ret;
            Extract<mode>(category, &ret);
            return ret;
        }
        template <ExtractMode mode = at_least_one, typename C = std::string, CHECK(impl::is_appendable_byte_seq_v<C>)>
        [[nodiscard]] C Extract(std::uint8_t byte)
        {
            return Extract<mode, C>(Char::EqualTo(byte));
        }

        // Discards matching characters from the input.
        // `mode` affects how many characters are read, and whether or not reading 0 characters causes an exception.
        // Returns the amount of characters processed.
        template <ExtractMode mode = one>
        std::size_t Discard(const Char::Category &category)
        {
            return Extract<mode>(category, nullptr);
        }
        template <ExtractMode mode = one>
        std::size_t Discard(std::uint8_t byte)
        {
            return Extract<mode>(Char::EqualTo(byte), nullptr);
        }

        // Discards a sequence of bytes from the input.
        // `mode` affects whether it returns `false` on failure or throws.
        template <ExtractMode mode = one>
        bool DiscardBytes(const std::uint8_t *bytes, std::size_t count)
        {
            static_assert(mode == one || mode == if_present, "Mode has to be `one` or `if_present`.");
            auto pos = Position();
            for (std::size_t i = 0; i < count; i++)
            {
                if (!MoreData() || PeekByte() != bytes[i])
                {
                    Seek(pos, absolute);
                    if constexpr (mode == one)
                        Program::Error(GetExceptionPrefix() + "Expected \"" + Strings::Escape(std::string_view(reinterpret_cast<const char *>(bytes), count)) + "\".");
                    return false;
                }
                SkipOne();
            }
            return true;
        }
        template <ExtractMode mode = one>
        bool DiscardChars(const char *chars, std::size_t count)
        {
            return DiscardBytes<mode>(reinterpret_cast<const std::uint8_t *>(chars), count);
        }
        template <ExtractMode mode = one>
        bool DiscardBytes(std::initializer_list<std::uint8_t> list)
        {
            return DiscardBytes<mode>(&*list.begin(), list.size());
        }
        template <ExtractMode mode = one>
        bool DiscardChars(std::string_view view)
        {
            return DiscardChars<mode>(&*view.begin(), view.size());
        }
    };
}
