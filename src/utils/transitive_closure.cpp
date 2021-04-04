#include "transitive_closure.h"

#include <algorithm>
#include <iostream>

#include "program/errors.h"

namespace TransitiveClosure
{
    std::size_t Data::FindUnorderedComponentPairs(std::function<void(std::size_t a, std::size_t b)> callback) const
    {
        std::size_t ret = 0;
        for (std::size_t i = 0; i < components.size(); i++)
        for (std::size_t j = 0; j < i; j++)
        {
            if (components[i].next_contains[j])
                continue;
            if (callback)
                callback(j, i);
            ret++;
        }
        return ret;
    }

    std::size_t Data::FindComponentsWithCycles(std::function<void(std::size_t i)> callback) const
    {
        std::size_t ret = 0;
        for (std::size_t i = 0; i < components.size(); i++)
        {
            if (!components[i].next_contains[i])
                continue;
            if (callback)
                callback(i);
            ret++;
        }
        return ret;
    }

    [[nodiscard]] Data Compute(std::size_t n, std::function<bool(std::size_t a, std::size_t b)> have_edge_from_to)
    {
        // Implementation of the 'STACK_TC' algorithm, described by Esko Nuutila (1995), in
        // 'Efficient Transitive Closure Computation in Large Digraphs'.

        constexpr std::size_t nil = -1;

        Data ret;
        ret.nodes.resize(n);
        std::vector<std::size_t> vstack, cstack; // Vertex and component stacks.
        vstack.reserve(n);
        cstack.reserve(n);

        auto StackTc = [&](auto &StackTc, std::size_t v)
        {
            if (ret.nodes[v].root != nil)
                return; // We already visited `v`.
            ret.nodes[v].root = v;
            ret.nodes[v].comp = nil;
            vstack.push_back(v);
            std::size_t saved_height = cstack.size();
            bool self_loop = false;
            for (std::size_t w = 0; w < n; w++)
            {
                if (!have_edge_from_to(v, w))
                    continue;
                if (v == w)
                {
                    self_loop = true;
                }
                else
                {
                    StackTc(StackTc, w);
                    if (ret.nodes[w].comp == nil)
                        ret.nodes[v].root = std::min(ret.nodes[v].root, ret.nodes[w].root);
                    else
                        cstack.push_back(ret.nodes[w].comp);

                    // The paper that this is based on had an extra condition on this last `else` branch,
                    // which I wasn't able to understand: `if (v,w) is not a forward edge`.
                    // However! Ivo Gabe de Wolff (2019) in "Higher ranked region inference for compile-time garbage collection"
                    // says that it doesn't affect correctness:
                    // > In the loop over the successors, the original algorithm Stack_TC checks whether
                    // > an edge is a so called forward edge. We do not perform this check, which may cause
                    // > that a component is pushed multiple times to cstack. As duplicates are removed in the
                    // > topological sort, these will be removed later on and not cause problems with correctness.
                }
            }
            if (ret.nodes[v].root == v)
            {
                std::size_t c = ret.components.size();
                ret.components.emplace_back();
                Data::Component &this_comp = ret.components.back();

                this_comp.next_contains.assign(ret.components.size(), false); // Sic.

                if (vstack.back() != v || self_loop)
                {
                    this_comp.next.push_back(c);
                    this_comp.next_contains[c] = true;
                }

                // Topologically sort a part of the component stack.
                std::sort(cstack.begin() + saved_height, cstack.end(), [&comp = ret.components](std::size_t a, std::size_t b) -> bool
                {
                    if (b >= comp[a].next_contains.size())
                        return false;
                    return comp[a].next_contains[b];
                });
                // Remove duplicates.
                cstack.erase(std::unique(cstack.begin() + saved_height, cstack.end()), cstack.end());

                while (cstack.size() != saved_height)
                {
                    std::size_t x = cstack.back();
                    cstack.pop_back();
                    if (!this_comp.next_contains[x])
                    {
                        if (!this_comp.next_contains[x])
                        {
                            this_comp.next.push_back(x);
                            this_comp.next_contains[x] = true;
                        }

                        this_comp.next.reserve(this_comp.next.size() + ret.components[x].next.size());
                        for (std::size_t c : ret.components[x].next)
                        {
                            if (!this_comp.next_contains[c])
                            {
                                this_comp.next.push_back(c);
                                this_comp.next_contains[c] = true;
                            }
                        }
                    }
                }

                std::size_t w;
                do
                {
                    w = vstack.back();
                    vstack.pop_back();
                    ret.nodes[w].comp = c;
                    this_comp.nodes.push_back(w);
                }
                while (w != v);
            }
        };

        for (std::size_t v = 0; v < n; v++)
            StackTc(StackTc, v);

        return ret;
    }

    namespace Tests
    {
        void RunAll()
        {
            std::cout << "Transitive closure tests:\n";

            auto test = [](std::size_t n, std::function<bool(std::size_t, std::size_t)> func, std::string_view target)
            {
                std::string result = Compute(n, func).DebugToString();
                if (result == target)
                {
                    std::cout << "OK\n";
                }
                else
                {
                    std::cout << "NOT OK\n";
                    std::cout << "EXPECTED: " << target << "\n";
                    std::cout << "GOT:      " << result << "\n";
                    Program::Error("Transitive closure test failed.");
                }
            };

            test(8, [](std::size_t a, std::size_t b)
            {
                bool arr[8][8] = {
                    {0,1,0,0,0,0,0,0},
                    {0,0,1,1,0,0,0,0},
                    {1,0,0,1,0,0,0,0},
                    {0,0,0,0,1,1,0,0},
                    {0,0,0,0,0,0,1,0},
                    {0,0,0,0,1,0,0,1},
                    {0,0,0,0,1,0,0,0},
                    {0,0,0,0,0,1,0,0},
                };
                return arr[a][b];
            }, "{nodes=[(0,3),(0,3),(0,3),(3,2),(4,0),(5,1),(4,0),(5,1)],components=[{nodes=[6,4],next=[0],next_contains=[1]},{nodes=[7,5],next=[1,0],next_contains=[1,1]},{nodes=[3],next=[0,1],next_contains=[1,1,0]},{nodes=[2,1,0],next=[3,2,0,1],next_contains=[1,1,1,1]}]}");


            test(10, [](std::size_t a, std::size_t b)
            {
                bool arr[10][10] = {
                    //      a b c d e f g h i j
                    /* a */{0,1,0,0,0,1,0,1,0,0},
                    /* b */{1,0,1,0,0,0,0,0,0,0},
                    /* c */{0,1,0,1,0,0,0,0,0,0},
                    /* d */{0,0,0,0,1,0,0,0,0,0},
                    /* e */{0,0,0,1,0,0,0,0,0,0},
                    /* f */{0,0,0,0,0,0,1,0,0,0},
                    /* g */{0,0,0,1,0,1,0,0,0,0},
                    /* h */{0,0,0,0,0,0,0,0,1,0},
                    /* i */{0,0,1,0,1,0,0,1,0,1},
                    /* j */{0,0,0,0,0,0,0,0,0,0},
                };
                return arr[a][b];
            }, "{nodes=[(0,3),(0,3),(0,3),(3,0),(3,0),(5,1),(5,1),(0,3),(0,3),(9,2)],components=[{nodes=[4,3],next=[0],next_contains=[1]},{nodes=[6,5],next=[1,0],next_contains=[1,1]},{nodes=[9],next=[],next_contains=[0,0,0]},{nodes=[8,7,2,1,0],next=[3,2,0,1],next_contains=[1,1,1,1]}]}");

            std::cout << "All tests passed.\n";
        }
    }
}
