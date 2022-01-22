#pragma once

#include <box2d/b2_draw.h>
#include <imgui.h>

#include "physics_2d/math_adapters.h"


// An imgui-based debug renderer for box2d.

namespace Physics2d
{
    class DebugRenderer : public b2Draw
    {
      public:
        // Camera location.
        fvec2 camera_pos{};
        float camera_scale = 1;

        // Visual options.
        float line_width = 1;
        float alpha_contour = 1;
        float alpha_fill = 0.6f;
        float xf_pixel_size = 16; // Pixel size for rendering transforms (aka coordinate systems?).

        // Target draw list.
        std::function<ImDrawList *()> get_draw_list = ImGui::GetBackgroundDrawList;

        [[nodiscard]] ImU32 ToImGuiColor(const b2Color &c, bool is_fill)
        {
            return ImGui::ColorConvertFloat4ToU32(fvec4(c.r, c.g, c.b, c.a * (is_fill ? alpha_fill : alpha_contour)));
            // return ImGui::ColorConvertFloat4ToU32(ImVec4(fvec4(c.r, c.g, c.b, c.a) * (is_fill ? alpha_fill : alpha_contour)))z;
        }

        [[nodiscard]] ImVec2 TransformPos(b2Vec2 vec)
        {
            return ImVec2((fvec2(vec) - camera_pos) * camera_scale + fvec2(ImGui::GetIO().DisplaySize) / 2);
        }

    	void DrawPolygon(const b2Vec2 *vertices, int32 vertex_count, const b2Color& color) override
        {
            // The points are always in a CCW order.
            auto list = get_draw_list();
            std::vector<ImVec2> xfed_vertices;
            xfed_vertices.reserve(vertex_count);
            for (decltype(vertex_count) i = 0; i < vertex_count; i++)
                xfed_vertices.push_back(TransformPos(vertices[i]));
            list->AddPolyline(xfed_vertices.data(), vertex_count, ToImGuiColor(color, false), ImDrawFlags_Closed, line_width);
        }

    	void DrawSolidPolygon(const b2Vec2 *vertices, int32 vertex_count, const b2Color& color) override
        {
            // The points are always in a CCW order.
            auto list = get_draw_list();
            std::vector<ImVec2> xfed_vertices;
            xfed_vertices.reserve(vertex_count);
            for (decltype(vertex_count) i = 0; i < vertex_count; i++)
                xfed_vertices.push_back(TransformPos(vertices[i]));
            list->AddConvexPolyFilled(xfed_vertices.data(), vertex_count, ToImGuiColor(color, true));
            list->AddPolyline(xfed_vertices.data(), vertex_count, ToImGuiColor(color, false), ImDrawFlags_Closed, line_width);
        }

    	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override
        {
            auto list = get_draw_list();
            list->AddCircle(TransformPos(center), radius * camera_scale, ToImGuiColor(color, false), 0, line_width);
        }

    	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override
        {
            auto list = get_draw_list();
            list->AddCircleFilled(TransformPos(center), radius * camera_scale, ToImGuiColor(color, true));
            DrawCircle(center, radius, color);
            DrawSegment(center, b2Vec2(fvec2(center) + fvec2(axis) * radius), color);
        }

    	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
        {
            auto list = get_draw_list();
            list->AddLine(TransformPos(p1), TransformPos(p2), ToImGuiColor(color, false), line_width);
        }

    	void DrawTransform(const b2Transform& xf) override
        {
            b2Vec2 x = xf.q.GetXAxis();
            b2Vec2 y = xf.q.GetYAxis();
            x *= xf_pixel_size / camera_scale;
            y *= xf_pixel_size / camera_scale;
            DrawSegment(xf.p, xf.p + x, b2Color(1, 0, 0));
            DrawSegment(xf.p, xf.p + y, b2Color(0, 1, 0));
        }

    	void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override
        {
            // I think `size` means a diameter?
            // Usage suggests the popular values range from 4,5,10.
            auto list = get_draw_list();
            list->AddCircleFilled(fvec2(p), size / 2, ToImGuiColor(color, false));
        }
    };
}
