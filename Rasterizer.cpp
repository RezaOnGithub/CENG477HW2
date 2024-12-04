#include "Rasterizer.hpp"

#include "Base.hpp"
#include "World.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

// TODO absolutely no clipping or culling before math is confirmed working!

using namespace m;

struct LineInViewport
{
    Vec4f start, end;
};

// class Box2D
// {
// private:
//     fp maxx, maxy, minx, miny;
// public:
//     Box2D() = delete;

//     Box2D(fp maxx, fp maxy, fp minx, fp miny)
//     {
//         // TODO disable/delete assertions for submission!
//         assert(maxx > minx and maxy > miny);
//         this->maxx = maxx;
//         this->maxy = maxy;
//         this->minx = minx;
//         this->miny = miny;
//     }

//     [[nodiscard]] bool is_above(Vec2f a)
//     {
//         return a.y > maxy;
//     }

//     [[nodiscard]] bool is_below(Vec2f a)
//     {
//         return a.y < miny;
//     }

//     [[nodiscard]] bool is_left(Vec2f a)
//     {
//         return a.x < minx;
//     }

//     [[nodiscard]] bool is_right(Vec2f a)
//     {
//         return a.x > maxx;
//     }

//     [[nodiscard]] bool is_inside(Vec2f a)
//     {
//         return not(is_above(a) or is_below(a) or is_left(a) or is_right(a));
//     }
// };

// LineVP clip_line_cohen_sutherland(const LineVP i, const Box2D)
// {

// }

// use midpoint algorithm to draw a raster line through two point in viewport
// coordinates. Return screen coordinates
std::vector<PixelCoordinate> draw_line_midpoint(const ViewConfig& v,
                                                const LineInViewport& l)
{
    return {};
}

RasterTriangle rasterize(const World& w, const ViewConfig& v,
                         const size_t face_index)
{
    // TODO currently focusing to get wirefram right. implement solid later!
    // TODO throwing away Z values for now!
    const Face f = w.get_face(face_index);
    // const Vec4f ndc0 = v.t_projection * v.t_camera * f.v0.coord.homopoint();
    // const Vec4f ndc1 = v.t_projection * v.t_camera * f.v1.coord.homopoint();
    // const Vec4f ndc2 = v.t_projection * v.t_camera * f.v2.coord.homopoint();

    const Matrix4 t_viewport = Matrix4::from_rows({
        {v.pixel_grid_rows / 2.0, 0,                          0, (v.pixel_grid_rows - 1) / 2.0   },
        { 0,                      v.pixel_grid_columns / 2.0, 0, (v.pixel_grid_columns - 1) / 2.0},
        { 0,                      0,                          1, 0                               },
        { 0,                      0,                          0, 1                               }
    });

    const Vec4f vp0 =
        t_viewport * v.t_projection * v.t_camera * f.v0.wc.homopoint();
    const Vec4f vp1 =
        t_viewport * v.t_projection * v.t_camera * f.v1.wc.homopoint();
    const Vec4f vp2 =
        t_viewport * v.t_projection * v.t_camera * f.v2.wc.homopoint();

    const Vec4f vp0h = vp0.scale(vp1.w * vp2.w);
    const Vec4f vp1h = vp0.scale(vp0.w * vp2.w);
    const Vec4f vp2h = vp0.scale(vp0.w * vp1.w);

    auto l0 = draw_line_midpoint(v, { vp0h, vp1h });
    auto l1 = draw_line_midpoint(v, { vp1h, vp2h });
    auto l2 = draw_line_midpoint(v, { vp2h, vp0h });

    std::vector<PixelCoordinate> fragc {};
    fragc.insert(fragc.end(), l0.begin(), l0.end());
    fragc.insert(fragc.end(), l1.begin(), l1.end());
    fragc.insert(fragc.end(), l2.begin(), l2.end());

    RasterTriangle r { f, {} };
    for (auto c : fragc)
    {
        // TODO throwing away Z for now!
        r.out.push_back({
            c, {{ 255, 0, 0 }, -1}
        });
    }
    return r;
}
