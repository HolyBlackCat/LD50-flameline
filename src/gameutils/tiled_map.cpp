#include "tiled_map.h"

#include "program/errors.h"
#include "utils/mat.h"

namespace Tiled
{
    Json::View FindLayer(Json::View map, std::string name)
    {
        Json::View ret;

        map["layers"].ForEachArrayElement([&](Json::View elem)
        {
            if (elem["name"].GetString() == name)
            {
                if (!ret)
                    ret = elem;
                else
                    Program::Error("More than one layer is named `", name, "`.");
            }
        });

        return ret;
    }

    TileLayer LoadTileLayer(Json::View source)
    {
        if (source["type"].GetString() != "tilelayer")
            Program::Error("Expected `", source["name"].GetString(), "` to be a tile layer.");

        ivec2 size(source["width"].GetInt(), source["height"].GetInt());

        Json::View array_view = source["data"];
        if (array_view.GetArraySize() != size.prod())
            Program::Error("Expected the layer of size ", size, " to have exactly " , size.prod(), " tiles.");

        TileLayer ret(size);
        int index = 0;

        for (int y = 0; y < ret.size().y; y++)
        for (int x = 0; x < ret.size().x; x++)
            ret.unsafe_at(ivec2(x,y)) = array_view[index++].GetInt();

        return ret;
    }

    PointLayer LoadPointLayer(Json::View source)
    {
        if (source["type"].GetString() != "objectgroup")
            Program::Error("Expected `", source["name"].GetString(), "` to be an object layer.");

        PointLayer ret;

        source["objects"].ForEachArrayElement([&](Json::View elem)
        {
            if (!elem.HasElement("point") || elem["point"].GetBool() != true)
                Program::Error("Expected every object on layer `", source["name"].GetString(), "` to be a point.");

            ret.points.insert({elem["name"].GetString(), fvec2(elem["x"].GetReal(), elem["y"].GetReal())});
        });

        return ret;
    }

    Properties LoadProperties(Json::View map)
    {
        Properties ret;
        map["properties"].ForEachArrayElement([&](Json::View elem)
        {
            std::string type = elem["type"].GetString();
            if (type == "string")
                ret.strings.insert({elem["name"].GetString(), elem["value"].GetString()});
        });
        return ret;
    }
}
