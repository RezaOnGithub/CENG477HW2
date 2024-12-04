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

    

    const Vec4f vp0 =
        v.t_viewport * v.t_projection * v.t_camera * f.v0.wc.homopoint();
    const Vec4f vp1 =
        v.t_viewport * v.t_projection * v.t_camera * f.v1.wc.homopoint();
    const Vec4f vp2 =
        v.t_viewport * v.t_projection * v.t_camera * f.v2.wc.homopoint();

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
