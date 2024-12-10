#include "Renderer.hpp"

#include "Base.hpp"
#include "World.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

using namespace m;
using namespace renderer;

/*****************************************************************************/
// helper functions
/*****************************************************************************/

constexpr PixelCoordinate vpc2pc(const Vec2f& vpc, const ViewConfig& v)
{
    // observation: round()-ing vpcs snaps them to the correct pixel (hopefully)

    fp row_from_top = vpc.x;
    fp column_from_left = v.pixel_grid_rows - vpc.y;
    // printf("Half-a-row %lf\n", (static_cast<fp>(v.pixel_grid_rows) / 2.0));
    // printf("Half-a-col %lf\n", (static_cast<fp>(v.pixel_grid_columns)
    // / 2.0)); printf("ROW: %lf\t%lf\t%li\n", vpc.y, row_from_top,
    //        static_cast<long>(round(row_from_top)));
    // printf("COL: %lf\t%lf\t%li\n", vpc.x, column_from_left,
    //        static_cast<long>(round(column_from_left)));
    return {
        static_cast<long>(round(row_from_top)),
        static_cast<long>(round(column_from_left)),
    };
}

// // TODO this is completely wrong. Use a zfail approach!
// std::vector<Fragment> ztest(std::vector<std::vector<float>>& zbuffer,
//                             const std::vector<Fragment>& fbuffer)
// {
//     std::vector<Fragment> out {};
//     for (auto frag : fbuffer)
//     {
//         const size_t row = frag.pc.row_from_top;
//         const size_t column = frag.pc.column_from_left;
//         if (zbuffer[row][column] > frag.attrib.depth)
//         {
//             out.push_back(frag);
//             zbuffer[row][column] = frag.attrib.depth;
//         }
//     }
//     return out;
// }

std::vector<Vec2f> midpoint_algorithm(Vec2f a, Vec2f b)
{
    assert(std::isfinite(a.x) && std::isfinite(b.x) && std::isfinite(a.y) &&
           std::isfinite(b.y));

    printf("Drawing a line from (%lf, %lf) to (%lf, %lf)\n", a.x, a.y, b.x,
           b.y);

    Vec2f start, end;
    if (a.x > b.x)
    {
        end = a;
        start = b;
    }
    else
    {
        start = a;
        end = b;
    }

    fp dx = end.x - start.x;
    fp dy = end.y - start.y;

    fp slope = dy / dx;
    bool steep = std::abs(slope) > 1.0;

    if (steep)
    {
        std::swap(start.x, start.y);
        std::swap(end.x, end.y);
        dx = end.x - start.x;
        dy = end.y - start.y;
    }

    fp d = 2 * std::abs(dy) - dx;
    fp y = start.y;
    std::vector<Vec2f> out;

    for (fp x = start.x; x <= end.x; x += 1.0f)
    {
        if (steep)
        {
            out.push_back({ y, x });
        }
        else
        {
            out.push_back({ x, y });
        }

        if (d > 0)
        {
            y += (dy > 0 ? 1.0f : -1.0f);
            d -= 2 * dx;
        }
        d += 2 * std::abs(dy);
    }

    return out;
}

/*****************************************************************************/
// step-by-step of our forward rendering pipeline
/*****************************************************************************/

S1Face step1_camera(const WorldFace& f, const ViewConfig& v)
{
    return {
        f,
        v.t_camera * f.v0.wc.homopoint(),
        v.t_camera * f.v1.wc.homopoint(),
        v.t_camera * f.v2.wc.homopoint(),
    };
}

S2Face step2_bfc(const S1Face& f, const ViewConfig& v)

{
    // FIXME
    // Vec4f normal =
    //     v.t_camera * surface_normal(f.cc0.dehomogenize(),
    //     f.cc1.dehomogenize(),
    //                                 f.cc2.dehomogenize())
    //                      .homovector();
    // if (v.cull_backface and dot(v.gaze.homovector(), normal) >= 0)
    // {
    //     return { f.mother, true, f.cc0, f.cc1, f.cc2 };
    // }
    return { f.mother, false, f.cc0, f.cc1, f.cc2 };
}

S3Face step3_device(const S2Face& f, const ViewConfig& v)
{
    const std::vector ndc { v.t_projection * f.cc0, v.t_projection * f.cc1,
                            v.t_projection * f.cc2 };
    return { f.mother, ndc[0], ndc[1], ndc[2] };
}

S4Polygon step4_sutherland_hodgman(const S3Face& f)
{
    // using m::Clip;
    // const std::vector<Vec3f> plane_normals {
    //     {0,   0,  1 },
    //     { 0,  0,  -1},
    //     { 0,  1,  0 },
    //     { 0,  -1, 0 },
    //     { 1,  0,  0 },
    //     { -1, 0,  0 }
    // };
    std::vector<HomoLine> ndc {
        {f.ndc0,  f.ndc1},
        { f.ndc1, f.ndc2},
        { f.ndc2, f.ndc0}
    };
    // std::vector<HomoLine> new_ndc {};

    // for (auto normal : plane_normals)
    // {
    //     for (auto [start, end] : ndc)
    //     {
    //         auto clip = clip_aa_inner(normal, { start, end });
    //         // TODO does it even make a difference if I used clip's output?
    //         switch (clip.t)
    //         {
    //         case Clip::ClipType::NonExistant :
    //             // Hope for the best!
    //             break;
    //         case Clip::ClipType::CutHead :
    //             new_ndc.emplace_back(clip.l.start, end);
    //             break;
    //         case Clip::ClipType::CutTail :
    //             new_ndc.emplace_back(start, clip.l.end);
    //             break;
    //         case Clip::ClipType::NoCut :
    //             new_ndc.emplace_back(start, end);
    //             break;
    //         }
    //     }
    //     ndc.erase(ndc.begin(), ndc.end());
    //     ndc.insert(ndc.end(), new_ndc.begin(), new_ndc.end());
    //     new_ndc.erase(new_ndc.begin(), new_ndc.end());
    // }
    return { f.mother, f.ndc0, f.ndc1, f.ndc2, ndc };
}

std::vector<S5Raster> step5_rasterize(const S4Polygon& f, const ViewConfig& v,
                                      bool in_layers)
{
    auto tvp_and_dehomogenize_to_2d = [v](const Vec4f& x) -> Vec2f
    {
        const Vec4f& transformed = v.t_viewport * x;
        return { transformed.dehomogenize().x, transformed.dehomogenize().y };
    };
    const Vec2f trig_vpc0 = tvp_and_dehomogenize_to_2d(f.trig_ndc0);
    const Vec2f trig_vpc1 = tvp_and_dehomogenize_to_2d(f.trig_ndc1);
    const Vec2f trig_vpc2 = tvp_and_dehomogenize_to_2d(f.trig_ndc2);

    std::vector<Line2d> vpc_edge {};
    // for (const auto& [start, end] : f.edge)
    // {
    //     vpc_edge.push_back({ tvp_and_dehomogenize_to_2d(start),
    //                          tvp_and_dehomogenize_to_2d(end) });
    // }
    vpc_edge = {
        {trig_vpc0,  trig_vpc1},
        { trig_vpc1, trig_vpc2},
        { trig_vpc2, trig_vpc0}
    };

    // Candidates are not-quite-fragments in viewport coordinates
    std::vector<Vec2f> vp_candidates {};
    std::vector<std::vector<Vec2f>> layers_vp_candidates {};
    for (const auto& [start, end] : vpc_edge)
    {
        const std::vector<Vec2f> l = midpoint_algorithm(start, end);
        vp_candidates.insert(vp_candidates.end(), l.begin(), l.end());
        // single extra line to enable layering
        layers_vp_candidates.push_back(l);
    }

    std::vector<Fragment::Attribute> a(3);
    auto ndc2depth = [](const fp x) -> fp
    {
        return (x + 1) / 2.0;
    };
    a[0].ceng477_color = { static_cast<fp>(f.mother.v0.ceng477_color.r),
                           static_cast<fp>(f.mother.v0.ceng477_color.g),
                           static_cast<fp>(f.mother.v0.ceng477_color.b) };
    a[1].ceng477_color = { static_cast<fp>(f.mother.v1.ceng477_color.r),
                           static_cast<fp>(f.mother.v1.ceng477_color.g),
                           static_cast<fp>(f.mother.v1.ceng477_color.b) };
    a[2].ceng477_color = { static_cast<fp>(f.mother.v2.ceng477_color.r),
                           static_cast<fp>(f.mother.v2.ceng477_color.g),
                           static_cast<fp>(f.mother.v2.ceng477_color.b) };
    a[0].depth = { 0, 0, ndc2depth(f.trig_ndc0.z) };
    a[1].depth = { 0, 0, ndc2depth(f.trig_ndc1.z) };
    a[2].depth = { 0, 0, ndc2depth(f.trig_ndc2.z) };
    auto noperspective = [&](const Vec2f vpp, const Vec3f& c0, const Vec3f& c1,
                             const Vec3f& c2) -> Vec3f
    {
        // perform barycentric interpolation for each component of generic
        // attribute (currently x,y,z)
        const Vec3f b = barycentric(trig_vpc0, trig_vpc1, trig_vpc2, vpp);
        const Vec3f xs = { c0.x, c1.x, c2.x };
        const Vec3f ys = { c0.y, c1.y, c2.y };
        const Vec3f zs = { c0.z, c1.z, c2.z };
        return { dot(b, xs), dot(b, ys), dot(b, zs) };
    };

    // No layering- nothing's changed
    std::vector<Fragment> frag {};
    for (auto vpp : vp_candidates)
    {
        frag.push_back({
            vpc2pc(vpp, v),
            {noperspective(vpp, a[0].ceng477_color, a[1].ceng477_color,
                      a[2].ceng477_color),
                      noperspective(vpp, a[0].depth, a[1].depth, a[2].depth)}
        });
    }
    if (not in_layers)
    {
        return {
            {f.mother, frag}
        };
    }

    std::vector<std::vector<Fragment>> layers_frag;
    for (auto layer_viewport : layers_vp_candidates)
    {
        std::vector<Fragment> frag {};
        for (auto vpp : layer_viewport)
        {
            frag.push_back({
                vpc2pc(vpp, v),
                {noperspective(vpp, a[0].ceng477_color, a[1].ceng477_color,
                          a[2].ceng477_color),
                          noperspective(vpp, a[0].depth, a[1].depth, a[2].depth)}
            });
        }
        layers_frag.push_back(frag);
    }
    std::vector<S5Raster> out;
    for (auto fl : layers_frag)
    {
        out.push_back({ f.mother, fl });
    }
    return out;
}

// Rasterize every triangle, perform depth test, write to an output buffer
std::vector<std::vector<std::vector<Pixel>>> render(const World& w, const ViewConfig& v,
                                       bool in_layers)
{
    if (not in_layers)
    {
        std::vector<Fragment> fragments {};
        std::vector pbuffer(v.pixel_grid_rows,
                            std::vector(v.pixel_grid_columns, v.bg_color));

        // rasterize every triangle in the scene
        for (size_t i = 0; i < w.face_count(); i++)
        {
            const WorldFace& f = w.get_face(i);
            const auto& s1 = step1_camera(f, v);
            const auto& s2 = step2_bfc(s1, v);
            const auto& s3 = step3_device(s2, v);
            const auto& s4 = step4_sutherland_hodgman(s3);
            const auto& s5 = step5_rasterize(s4, v, false);
            fragments.insert(fragments.end(), s5[0].out.begin(),
                             s5[0].out.end());
        }

        for (const auto& d : fragments)
        {
            pbuffer[d.pc.column_from_left][d.pc.row_from_top] =
                vec2color(d.a.ceng477_color);
        }

        return {pbuffer};
    }
}

std::vector<std::vector<std::vector<Pixel>>> render_layers(const World& w,
                                                           const ViewConfig& v)
{
    // std::vector<Fragment> fragments {};

    std::vector<S5Raster> debug_tri_lines {};
    // rasterize every triangle in the scene
    for (size_t i = 0; i < w.face_count(); i++)
    {
        const WorldFace& f = w.get_face(i);
        const auto& s1 = step1_camera(f, v);
        const auto& s2 = step2_bfc(s1, v);
        const auto& s3 = step3_device(s2, v);
        const auto& s4 = step4_sutherland_hodgman(s3);
        const auto& s5 = step5_rasterize_layers(s4, v);
        debug_tri_lines.insert(debug_tri_lines.end(), s5.begin(), s5.end());
        printf("#%lu ^^^\n", debug_tri_lines.size() - 1);
        // fragments.insert(fragments.end(), s5.out.begin(), s5.out.end());
    }

    std::vector<std::vector<std::vector<Pixel>>> layers {};
    for (auto layer : debug_tri_lines)
    {
        std::vector pbuffer(v.pixel_grid_rows,
                            std::vector(v.pixel_grid_columns, v.bg_color));
        for (const auto& d : layer.out)
        {
            pbuffer[d.pc.column_from_left][d.pc.row_from_top] =
                vec2color(d.a.ceng477_color);
        }
        layers.push_back(pbuffer);
    }

    return layers;
}
