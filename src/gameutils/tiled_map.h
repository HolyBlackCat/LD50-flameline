#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <string>

#include "program/errors.h"
#include "strings/common.h"
#include "utils/json.h"
#include "utils/mat.h"
#include "utils/multiarray.h"

namespace Tiled
{
    Json::View FindLayer(Json::View map, std::string name);
    Json::View FindLayerOpt(Json::View map, std::string name);

    using TileLayer = MultiArray<2, int>;
    TileLayer LoadTileLayer(Json::View source);

    struct PointLayer
    {
        std::multimap<std::string, fvec2> points;

        template <typename F>
        void ForEachPointNamed(std::string name, F &&func) const // `func` is `void func(fvec2 pos)`.
        {
            auto [begin, end] = points.equal_range(name);
            while (begin != end)
            {
                func(begin->second);
                begin++;
            }
        }

        template <typename F>
        void ForEachPointWithNamePrefix(std::string prefix, F &&func) const // `func` is `void func(std::string_view suffix, fvec2 pos)`.
        {
            auto begin = points.lower_bound(prefix);
            while (begin != points.end() && begin->first.starts_with(prefix))
            {
                func(std::string_view(begin->first.begin() + prefix.size(), begin->first.end()), begin->second);
                begin++;
            }
        }

        std::optional<fvec2> GetSinglePointOpt(std::string name) const
        {
            auto [begin, end] = points.equal_range(name);
            if (begin == end)
                return {};
            if (std::next(begin) != end)
                Program::Error("Expected the map to contain one or less points named `", name, "`.");
            return begin->second;
        }

        fvec2 GetSinglePoint(std::string name) const
        {
            auto [begin, end] = points.equal_range(name);
            if (begin == points.end() || std::next(begin) != end)
                Program::Error("Expected the map to contain exactly one point named `", name, "`.");
            return begin->second;
        }

        std::vector<fvec2> GetPointList(std::string name) const
        {
            auto [begin, end] = points.equal_range(name);
            std::vector<fvec2> ret;
            ret.reserve(std::distance(begin, end));
            std::transform(begin, end, std::back_inserter(ret), [](const auto &pair){return pair.second;});
            return ret;
        }
    };

    PointLayer LoadPointLayer(Json::View source);

    struct Properties
    {
        std::map<std::string, std::string> strings;

        bool HasString(std::string name) const
        {
            return strings.find(name) != strings.end();
        }
        const std::string GetString(std::string name) const
        {
            auto it = strings.find(name);
            if (it == strings.end())
                Program::Error("Invalid map property string name: `", name, "`.");
            return it->second;
        }
    };

    Properties LoadProperties(Json::View map);
}
