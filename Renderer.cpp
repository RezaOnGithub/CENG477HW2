#include "Renderer.hpp"

#include "Base.hpp"
#include "World.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
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

    // Ensure `start` is always the point with the smaller x-coordinate
    Vec2f start = (a.x < b.x) ? a : b;
    Vec2f end = (a.x < b.x) ? b : a;

    fp dx = end.x - start.x;
    fp dy = end.y - start.y;

    std::vector<Vec2f> out;

    // Handle vertical lines explicitly
    if (dx == 0)
    {
        fp step = (dy > 0) ? 1.0f : -1.0f;
        for (fp y = start.y; (step > 0 ? y <= end.y : y >= end.y); y += step)
        {
            out.push_back({ start.x, y });
        }
        return out;
    }

    // Determine if the slope is steep
    bool steep = std::abs(dy) > std::abs(dx);

    // Swap x and y if the line is steep
    if (steep)
    {
        std::swap(start.x, start.y);
        std::swap(end.x, end.y);
        dx = end.x - start.x;
        dy = end.y - start.y;
    }

    // Determine the direction of increment for y
    fp y_step = (dy > 0) ? 1.0f : -1.0f;

    fp d = 2 * std::abs(dy) - dx;
    fp y = start.y;

    for (fp x = start.x; x <= end.x; x += 1.0f)
    {
        if (steep)
        {
            out.push_back({ y, x });   // Swap back x and y for steep lines
        }
        else
        {
            out.push_back({ x, y });
        }

        if (d > 0)
        {
            y += y_step;
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
    using m::Clip;
    const std::vector<Vec3f> plane_normals {
        { 0,  0,  1  },
        { 0,  0,  -1 },
        { 0,  1,  0  },
        { 0,  -1, 0  },
        { 1,  0,  0  },
        { -1, 0,  0  }
    };
    std::vector<HomoLine> ndc {
        { f.ndc0, f.ndc1 },
        { f.ndc1, f.ndc2 },
        { f.ndc2, f.ndc0 }
    };
    std::vector<HomoLine> new_ndc {};

    for (auto normal : plane_normals)
    {
        for (auto [start, end] : ndc)
        {
            auto clip = clip_aa_inner(normal, { start, end });
            // TODO does it even make a difference if I used clip's output?
            switch (clip.t)
            {
            case Clip::ClipType::NonExistant :
                // Hope for the best!
                break;
            case Clip::ClipType::CutHead :
                new_ndc.emplace_back(clip.l.start, end);
                break;
            case Clip::ClipType::CutTail :
                new_ndc.emplace_back(start, clip.l.end);
                break;
            case Clip::ClipType::NoCut :
                new_ndc.emplace_back(start, end);
                break;
            }
        }
        ndc.erase(ndc.begin(), ndc.end());
        ndc.insert(ndc.end(), new_ndc.begin(), new_ndc.end());
        new_ndc.erase(new_ndc.begin(), new_ndc.end());
    }
    return { f.mother, f.ndc0, f.ndc1, f.ndc2, ndc };
}

S5Raster step5_rasterize(const S4Polygon& f, const ViewConfig& v)
{
    auto tvp_and_dehomogenize_to_2d = [v](const Vec4f& x) -> Vec2f
    {
        const Vec4f& transformed = v.t_viewport * x;
        return { transformed.dehomogenize().x, transformed.dehomogenize().y };
    };
    const Vec2f trig_vpc0 = tvp_and_dehomogenize_to_2d(f.trig_ndc0);
    const Vec2f trig_vpc1 = tvp_and_dehomogenize_to_2d(f.trig_ndc1);
    const Vec2f trig_vpc2 = tvp_and_dehomogenize_to_2d(f.trig_ndc2);

    // dprint("trig_vpc0", trig_vpc0);
    // dprint("trig_ndc0", f.trig_ndc0);
    // printf("fragment coordinate X%li Y%li!\n",
    //        vpc2pc(trig_vpc0, v).column_from_left,
    //        vpc2pc(trig_vpc0, v).row_from_top);

    std::vector<Line2d> vpc_edge {};
    for (const auto& [start, end] : f.edge)
    {
        vpc_edge.push_back({ tvp_and_dehomogenize_to_2d(start),
                             tvp_and_dehomogenize_to_2d(end) });
    }

    // Candidates are not-quite-fragments in viewport coordinates
    std::vector<Vec2f> vp_candidates {};
    for (const auto& [start, end] : vpc_edge)
    {
        const std::vector<Vec2f> l = midpoint_algorithm(start, end);
        vp_candidates.insert(vp_candidates.end(), l.begin(), l.end());
    }

    Fragment::Attribute a0 {};
    Fragment::Attribute a1 {};
    Fragment::Attribute a2 {};

    auto ndc2depth = [](const fp x) -> fp
    {
        return (x + 1) / 2.0;
    };

    a0.ceng477_color = { static_cast<fp>(f.mother.v0.ceng477_color.r),
                         static_cast<fp>(f.mother.v0.ceng477_color.g),
                         static_cast<fp>(f.mother.v0.ceng477_color.b) };
    a1.ceng477_color = { static_cast<fp>(f.mother.v1.ceng477_color.r),
                         static_cast<fp>(f.mother.v1.ceng477_color.g),
                         static_cast<fp>(f.mother.v1.ceng477_color.b) };
    a2.ceng477_color = { static_cast<fp>(f.mother.v2.ceng477_color.r),
                         static_cast<fp>(f.mother.v2.ceng477_color.g),
                         static_cast<fp>(f.mother.v2.ceng477_color.b) };
    a0.depth = { 0, 0, ndc2depth(f.trig_ndc0.z) };
    a1.depth = { 0, 0, ndc2depth(f.trig_ndc1.z) };
    a2.depth = { 0, 0, ndc2depth(f.trig_ndc2.z) };

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

    std::vector<Fragment> frag {};
    for (auto vpp : vp_candidates)
    {
        Vec3f xxx = noperspective(vpp, a0.ceng477_color, a1.ceng477_color,
                                  a2.ceng477_color);
        frag.push_back({
            vpc2pc(vpp, v),
            { noperspective(vpp, a0.ceng477_color, a1.ceng477_color,
                      a2.ceng477_color),
                      noperspective(vpp, a0.depth, a1.depth, a2.depth) }
        });
    }

    // m::Vec3f red { 255, 0, 0 };
    // std::vector<Fragment> frag {
    //     {vpc2pc(trig_vpc0,  v), { red, red }},
    //     { vpc2pc(trig_vpc1, v), { red, red }},
    //     { vpc2pc(trig_vpc2, v), { red, red }},
    // };

    return { f.mother, frag };
}

// Rasterize every triangle, perform depth test, write to an output buffer
std::vector<std::vector<Pixel>> render(const World& w, const ViewConfig& v)
{
    std::vector<Fragment> fragments {};
    std::vector pbuffer(v.pixel_grid_rows,
                        std::vector(v.pixel_grid_columns, v.bg_color));

    std::vector<std::vector<Fragment>> debug_ls {};
    // rasterize every triangle in the scene
    for (size_t i = 0; i < w.face_count(); i++)
    {
        const WorldFace& f = w.get_face(i);
        const auto& s1 = step1_camera(f, v);
        const auto& s2 = step2_bfc(s1, v);
        const auto& s3 = step3_device(s2, v);
        const auto& s4 = step4_sutherland_hodgman(s3);
        const auto& s5 = step5_rasterize(s4, v);
        debug_ls.push_back(s5.out);
        fragments.insert(fragments.end(), s5.out.begin(), s5.out.end());
    }

    printf("resultant fragment count %lu\n", fragments.size());

    // turn surviving fragments into pixels
    std::vector<long> seen_x {};
    std::vector<long> seen_y {};
    for (const auto& d : fragments)
    {
        auto color = [](const Vec3f& c) -> Pixel
        {
            return { static_cast<unsigned char>(c.x > 255 ? 255 :
                                                c.x < 0   ? 0 :
                                                            c.x),
                     static_cast<unsigned char>(c.y > 255 ? 255 :
                                                c.y < 0   ? 0 :
                                                            c.y),
                     static_cast<unsigned char>(c.z > 255 ? 255 :
                                                c.z < 0   ? 0 :
                                                            c.z) };
        };

        // FIXME comment out these sanity checks at some point

        if (d.pc.row_from_top < 0 || d.pc.column_from_left < 0 ||
            d.pc.row_from_top >= v.pixel_grid_rows ||
            d.pc.column_from_left >= v.pixel_grid_columns)
        {
            printf("invalid fragment coordinate %li %li!\n",
                   d.pc.column_from_left, d.pc.row_from_top);
            continue;
        }
        if (std::ranges::find(seen_x, d.pc.column_from_left) != seen_x.end())
        {
            if (std::ranges::find(seen_y, d.pc.row_from_top) != seen_y.end())
            {
                // FIXME yeah yeah overdraw
                // printf("OVERDRAW! At %lu %lu\n", d.pc.column_from_left,
                // d.pc.row_from_top);
            }
        }
        seen_x.push_back(d.pc.column_from_left);
        seen_y.push_back(d.pc.row_from_top);
        pbuffer[d.pc.column_from_left][d.pc.row_from_top] =
            color(d.a.ceng477_color);
    }

    return pbuffer;
}
