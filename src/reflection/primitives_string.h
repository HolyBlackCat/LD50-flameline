#pragma once

#include <string>

#include "interface.h"

namespace Refl::Custom
{
    template <> struct Primitive<std::string, void>
    {
        inline static const std::string name = "string";

        static char _to_escape_seq(char ch)
        {
            switch (ch)
            {
                case '\0': return '0'; break;
                case '\a': return 'a'; break;
                case '\b': return 'b'; break;
                case '\f': return 'f'; break;
                case '\n': return 'n'; break;
                case '\r': return 'r'; break;
                case '\t': return 't'; break;
                case '\v': return 'v'; break;
            }
            return -1;
        }

        static char _from_escape_seq(char ch)
        {
            switch (ch)
            {
                case '0': return '\0'; break;
                case 'a': return '\a'; break;
                case 'b': return '\b'; break;
                case 'f': return '\f'; break;
                case 'n': return '\n'; break;
                case 'r': return '\r'; break;
                case 't': return '\t'; break;
                case 'v': return '\v'; break;
            }
            return -1;
        }

        static char _to_hex_digit(int index)
        {
            return "0123456789abcdef"[index];
        }

        static int _from_hex_digit(char ch)
        {
            if (ch >= '0' && ch <= '9')
                return ch - '0';
            if (ch >= 'a' && ch <= 'f')
                return ch - 'a' + 10;
            if (ch >= 'A' && ch <= 'F')
                return ch - 'A' + 10;
            return -1;
        }

        static std::string to_string(const std::string &object)
        {
            std::string ret;
            ret += '"';
            for (char ch : object)
            {
                if (char esc = _to_escape_seq(ch); esc != -1)
                {
                    ret += '\\';
                    ret += esc;
                }
                else if (ch < ' ' || ch >= 0x7f)
                {
                    ret += "\\x";
                    ret += _to_hex_digit((unsigned char)ch >> 4);
                    ret += _to_hex_digit(ch & 0xf);
                }
                else
                {
                    ret += ch;
                }
            }
            ret += '"';
            return ret;
        }

        static bool from_string(std::string &object, const char *&string)
        {
            if (*string != '"') return 0;
            string++;

            object = {};
            while (*string != '"')
            {
                if (string[0] == '\\')
                {
                    if (string[1] == 'x')
                    if (int high = _from_hex_digit(string[2]); high != -1)
                    if (int low = _from_hex_digit(string[3]); low != -1)
                    {
                        object += char(high * 16 + low);
                        string += 4;
                        continue;
                    }

                    if (char ch = _from_escape_seq(string[1]); ch != -1)
                    {
                        object += ch;
                        string += 2;
                        continue;
                    }

                    return 0;
                }

                object += *string;
                string++;
            }

            string++; // Skip `"`.
            return 1;
        }
    };
}
