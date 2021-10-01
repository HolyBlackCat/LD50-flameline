// This example demonstrates `namespace TransitiveClosure`.
// It's an interactive console program.
// Input comma-separated chains of inequalities ('less' only), get the array sorted according to your input.
// Examples: (user input starts with `#`)
//     # a
//     Result: a
//     # a, b
//     Assuming: a<b
//     Result: a,b
//     # b < c, a < b
//     Result: a,b,c
//     # a < b, c < d
//     Assuming: a<c, b<c, a<d, b<d
//     Result: a,b,c,d
//     # a<b<d, a<c<d
//     Assuming: b<c
//     Result: a,b,c,d
//     # a<b<d, a<c<d, c<d, d<c
//     Conflicting requirements for: c,d
//     Result: a,b,c,d


#include "utils/transitive_closure.h"

#include <map>
#include <set>
#include <utility>

IMP_MAIN(,)
{
    std::string line;
    while (std::cout << "# ", std::getline(std::cin, line))
    {
        try
        {
            std::set<std::pair<char, char>> pairs;
            std::vector<char> index_to_char;
            std::map<char, std::size_t> char_to_index;

            Stream::Input input = Stream::ReadOnlyData::mem_reference(line);
            input.WantLocationStyle(Stream::text_byte_position);
            input.WantExceptionPrefixStyle(Stream::with_location);

            input.Discard<Stream::any>(Stream::Char::IsWhitespace{});
            while (true)
            {
                auto GetChar = [&]() -> char
                {
                    char ch = input.Extract<Stream::one>(Stream::Char::IsAlpha());
                    if (!char_to_index.contains(ch))
                    {
                        char_to_index.emplace(ch, index_to_char.size());
                        index_to_char.push_back(ch);
                    }
                    return ch;
                };

                char a = GetChar();
                input.Discard<Stream::any>(Stream::Char::IsWhitespace{});

                while (input.MoreData() && input.PeekChar() == '<')
                {
                    input.Discard('<');
                    input.Discard<Stream::any>(Stream::Char::IsWhitespace{});
                    char b = GetChar();
                    input.Discard<Stream::any>(Stream::Char::IsWhitespace{});
                    pairs.emplace(b, a); // With reverse order here, everything else works much more naturally.
                    a = b;
                }

                if (!input.MoreData())
                    break;

                input.Discard(',');
                input.Discard<Stream::any>(Stream::Char::IsWhitespace{});
            }

            if (index_to_char.empty())
                continue;

            TransitiveClosure::Data data = TransitiveClosure::Compute(index_to_char.size(), [&](std::size_t a, std::size_t b){return pairs.contains({index_to_char[a], index_to_char[b]});});

            // std::cout << data.DebugToString() << '\n';

            auto PrintComponent = [&](std::size_t i, bool allow_brackets = true)
            {
                if (data.components[i].nodes.size() <= 1)
                    allow_brackets = false;
                if (allow_brackets)
                    std::cout << '[';
                bool first = true;
                for (std::size_t x : data.components[i].nodes)
                {
                    if (first)
                        first = false;
                    else
                        std::cout << ',';
                    std::cout << index_to_char[x];
                }
                if (allow_brackets)
                    std::cout << ']';
            };

            data.FindComponentsWithCycles([&](std::size_t i)
            {
                std::cout << "Conflicting requirements for: ";
                PrintComponent(i, false);
                std::cout << '\n';
            });

            if (data.FindUnorderedComponentPairs([&, first = true](std::size_t a, std::size_t b) mutable
            {
                if (first)
                {
                    std::cout << "Assuming: ";
                    first = false;
                }
                else
                {
                    std::cout << ", ";
                }

                PrintComponent(a);
                std::cout << "<";
                PrintComponent(b);
            }))
            {
                std::cout << '\n';
            }

            std::cout << "Result: ";
            for (std::size_t i = 0; i < data.components.size(); i++)
            {
                if (i != 0)
                    std::cout << ',';
                PrintComponent(i, false);
            }
            std::cout << '\n';
        }
        catch (std::exception &e)
        {
            std::cout << "Error: " << e.what() << '\n';
        }
    }

    return 0;
}
