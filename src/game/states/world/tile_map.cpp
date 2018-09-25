#include "game/states/world/tile_map.h"

#include "game/meta.h"

namespace States::Details::World
{
    void TileMap::Load(const std::string &name)
    {
        map = Map(format, name);

        { // Autotiling
            Map::TileLayer &la = map.Layer(0);
            Map::TileLayer &la_front = map.Layer(1);

            // mid
            for (int y = 0; y < la.Size().y; y++)
            for (int x = 0; x < la.Size().x; x++)
            {
                int index = la.UnsafeGet(ivec2(x,y)).index;
                if (index == Map::index_none)
                    continue;

                bool a = index == la.ClampGet(ivec2(x,y-1)).index;
                bool b = index == la.ClampGet(ivec2(x+1,y)).index;
                bool c = index == la.ClampGet(ivec2(x,y+1)).index;
                bool d = index == la.ClampGet(ivec2(x-1,y)).index;
                bool ab = index == la.ClampGet(ivec2(x+1,y-1)).index;
                bool bc = index == la.ClampGet(ivec2(x+1,y+1)).index;
                bool cd = index == la.ClampGet(ivec2(x-1,y+1)).index;
                bool da = index == la.ClampGet(ivec2(x-1,y-1)).index;

                if (index == 1) // Grass
                {
                    std::string name;
                    int count = 4;
                    if (!a && !b && c && d)
                        name = "^>", count = 1;
                    else if (a && !b && !c && d)
                        name = "v>", count = 1;
                    else if (a && b && !c && !d)
                        name = "<v", count = 1;
                    else if (!a && b && c && !d)
                        name = "<^", count = 1;
                    else if (a && b && c && d && !ab)
                        name = ">^", count = 1;
                    else if (a && b && c && d && !bc)
                        name = ">v", count = 1;
                    else if (a && b && c && d && !cd)
                        name = "v<", count = 1;
                    else if (a && b && c && d && !da)
                        name = "^<", count = 1;
                    else if (!a && b && c && d)
                        name = "^";
                    else if (a && !b && c && d)
                        name = ">";
                    else if (a && b && !c && d)
                        name = "v";
                    else if (a && b && c && !d)
                        name = "<";
                    else
                        name = "*";

                    la.UnsafeSetVariantName(ivec2(x,y), Str(name, random.integer() < count));

                    if (!a)
                        la_front.UnsafeSetIndexName(ivec2(x,y-1), "grass");
                }
            }

            // front
            for (int y = 0; y < la.Size().y; y++)
            for (int x = 0; x < la.Size().x; x++)
            {
                int index = la_front.UnsafeGet(ivec2(x,y)).index;
                if (index == Map::index_none)
                    continue;

    //            bool a = index == la_front.ClampGet(ivec2(x,y-1)).index;
                bool b = index == la_front.ClampGet(ivec2(x+1,y)).index;
    //            bool c = index == la_front.ClampGet(ivec2(x,y+1)).index;
                bool d = index == la_front.ClampGet(ivec2(x-1,y)).index;
    //            bool ab = index == la_front.ClampGet(ivec2(x+1,y-1)).index;
    //            bool bc = index == la_front.ClampGet(ivec2(x+1,y+1)).index;
    //            bool cd = index == la_front.ClampGet(ivec2(x-1,y+1)).index;
    //            bool da = index == la_front.ClampGet(ivec2(x-1,y-1)).index;

                if (index == 10001) // grass
                {
                    std::string name;
                    int count = 4;

                    if (b && !d)
                        name = "<", count = 1;
                    else if (!b && d)
                        name = ">", count = 1;
                    else if (b && d)
                        name = "^";
                    else
                        count = 0;

                    if (count == 0)
                        la_front.UnsafeSetIndex(ivec2(x,y), Map::index_none);
                    else
                        la_front.UnsafeSetVariantName(ivec2(x,y), Str(name, random.integer() < count));
                }
            }

            map.ValidateVariantIndices();
        }
    }

    void TileMap::Render() const
    {
        const fvec3 a = fvec3(12,31,127)/255, b = fvec3(50,111,186)/255;
        render.iquad(-screen_size/2, screen_size).color(a, a, b, b);
//        render.fquad(mouse.pos(), fvec2(32)).center().color(fvec3(1,0.5,0));
//        render.fquad(mouse.pos().add_y(32), fvec2(16)).center().tex(fvec2(16,0));
        map.Layer(0).Render(render, screen_size, mouse.pos());
        map.Layer(1).Render(render, screen_size, mouse.pos());
    }
}
