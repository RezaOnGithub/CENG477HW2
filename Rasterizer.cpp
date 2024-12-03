#include "Rasterizer.hpp"

#include "Base.hpp"
#include "World.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>

// TODO absolutely no clipping or culling before math is confirmed working!

using namespace m;

using DotVP = Vec2f;

struct LineVP
{
    DotVP start, end;
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
std::vector<std::pair<long, long>> draw_line_midpoint(const ViewConfig& v,
                                                      const LineVP& l)
{
    return {};
}

RasterTriangle rasterize(const World& w, const ViewConfig& v,
                         const size_t face_index)
{
    // TODO currently focusing to get wirefram right. implement solid later!
    const Face f = w.get_face(face_index);
    const Vec4f w0 = v.t_projection * v.t_camera * f.v0.coord.homopoint();
    const Vec4f w1 = v.t_projection * v.t_camera * f.v1.coord.homopoint();
    const Vec4f w2 = v.t_projection * v.t_camera * f.v2.coord.homopoint();
}
