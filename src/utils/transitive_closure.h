#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "strings/format.h"

// A 'transitive closure' of an oriented graph is a similar graph with edges added.
// If in the original graph, B is reachable from A in 1+ steps, then in the transitive closure it will be reachable in 1 step.

namespace TransitiveClosure
{
    struct Data
    {
        // The nodes of the graph are grouped into 'components'.
        // In a component, each node is reachable (possibly indirectly) from every other node.
        // If the number of components is same as the number of nodes, your graph has no cycles.
        // Otherwise there will be less components.
        // The components form a graph called 'condensation graph', which is always acyclic.
        // The components are numbered in a way that 'B is reachable from A' implies `B <= A`.

        struct Node
        {
            // An arbitrarily selected node in the same component. Same for all nodes in this component.
            std::size_t root = -1;
            // Index if the component this node belongs to.
            std::size_t comp = -1;

            [[nodiscard]] std::string DebugToString() const
            {
                return FMT("({},{})", root, comp);
            }
        };
        // Size is the number of nodes, which was passed in the argument.
        std::vector<Node> nodes;

        struct Component
        {
            // Nodes that are a part of this component.
            std::vector<std::size_t> nodes;
            // Which components are reachable (possibly indirectly) from this one.
            // Unordered, but has no duplicates. May or may not contain itself.
            std::vector<std::size_t> next;
            // A convenicene array.
            // `next_contains[i]` is 1 if and only if `next` contains `i`.
            // Some trailing zeroes might be missing, check the size before accessing it.
            // More specifically, i-th component has i+1 numbers in this array.
            std::vector<unsigned char/*boolean*/> next_contains;

            [[nodiscard]] std::string DebugToString() const
            {
                return FMT("{{nodes=[{}],next=[{}],next_contains=[{}]}}", fmt::join(nodes, ","), fmt::join(next, ","), fmt::join(next_contains, ","));
            }
        };
        // Size is the number of components.
        std::vector<Component> components;

        [[nodiscard]] std::string DebugToString() const
        {
            std::string ret = "{nodes=[";
            for (std::size_t i = 0; i < nodes.size(); i++)
            {
                if (i != 0) ret += ",";
                ret += nodes[i].DebugToString();
            }
            ret += "],components=[";
            for (std::size_t i = 0; i < components.size(); i++)
            {
                if (i != 0) ret += ",";
                ret += components[i].DebugToString();
            }
            ret += "]}";
            return ret;
        }

        // Invokes the callback for every pair of components that's unordered relative to each other.
        // Only checks pairs such that `a < b`.
        // Returns the number of such pairs.
        std::size_t FindUnorderedComponentPairs(std::function<void(std::size_t a, std::size_t b)> callback = nullptr) const;

        // Invokes the callback for every component that has a cycle in it (i.e. either has >1 node, or the only node has an edge to itself).
        // Returns the number of such components.
        std::size_t FindComponentsWithCycles(std::function<void(std::size_t i)> callback = nullptr) const;
    };

    [[nodiscard]] Data Compute(std::size_t n, std::function<bool(std::size_t a, std::size_t b)> have_edge_from_to);

    namespace Tests
    {
        // Run some tests, printing output to `std::cout`.
        void RunAll();
    }
}
