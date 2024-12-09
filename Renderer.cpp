#include "Renderer.hpp"

#include "Base.hpp"
#include "World.hpp"

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

std::vector<Vec2f> midpoint_algorithm(Vec2f start, Vec2f end)
{
    assert(std::isfinite(start.x) and std::isfinite(end.x) and
           std::isfinite(start.y) and std::isfinite(end.y));
    if (start.x > end.x)
    {
        return midpoint_algorithm(end, start);
    }

    // end.x >= start.x guarenteed

    printf("Drawing a line from (%lf, %lf) to (%lf, %lf)\n", start.x, start.y,
           end.x, end.y);

    auto implicit_line = [start, end](fp x, fp y) -> fp
    {
        return (start.y - end.y) * x + (end.x - start.x) * y + start.x * end.y -
               start.y * end.x;
    };

    // midpoint algorithm, Page 165
    std::vector<Vec2f> out {};
    fp y = start.y;
    fp d = implicit_line(start.x + 1, start.y + 0.5);
    for (int i = 0; start.x + i <= end.x; i++)
    {
        out.push_back({ start.x + i, y });
        if (d < 0)
        {
            y++;
            d += (end.x - start.x) + (start.y - end.y);
        }
        else
        {
            d += (start.y - end.y);
        }
    }
    return out;
}

// constexpr FragmentAttributeFinal interpolate_attributes()
// {

// }

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
    std::vector<Vec4f> ndc { v.t_projection * f.cc0, v.t_projection * f.cc1,
                             v.t_projection * f.cc2 };
    return { f.mother, ndc[0], ndc[1], ndc[2] };
}

S4Polygon step4_sutherland_hodgman(const S3Face& f)
{
    using m::Clip;
    const std::vector<Vec3f> plane_normals {
        {0,   0,  1 },
        { 0,  0,  -1},
        { 0,  1,  0 },
        { 0,  -1, 0 },
        { 1,  0,  0 },
        { -1, 0,  0 }
    };
    std::vector<m::HomoLine> ndc {
        {f.ndc0,  f.ndc1},
        { f.ndc1, f.ndc2},
        { f.ndc2, f.ndc0}
    };
    std::vector<m::HomoLine> new_ndc {};

    for (auto normal : plane_normals)
    {
        for (auto line : ndc)
        {
            auto clip = clip_aa_inner(normal, { line.start, line.end });
            // TODO does it even make a difference if I used clip's output?
            switch (clip.t)
            {
            case m::Clip::ClipType::NonExistant :
                // Hope for the best!
                break;
            case m::Clip::ClipType::CutHead :
                new_ndc.emplace_back(clip.l.start, line.end);
                break;
            case m::Clip::ClipType::CutTail :
                new_ndc.emplace_back(line.start, clip.l.end);
                break;
            case m::Clip::ClipType::NoCut :
                new_ndc.emplace_back(line.start, line.end);
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
    auto dehomogenize_to_2d = [](Vec4f v) -> Vec2f
    {
        return { v.dehomogenize().x, v.dehomogenize().y };
    };
    const Vec2f trig_vpc0 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc0);
    const Vec2f trig_vpc1 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc1);
    const Vec2f trig_vpc2 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc2);

    // dprint("trig_vpc0", trig_vpc0);
    // dprint("trig_ndc0", f.trig_ndc0);
    // printf("fragment coordinate X%li Y%li!\n",
    //        vpc2pc(trig_vpc0, v).column_from_left,
    //        vpc2pc(trig_vpc0, v).row_from_top);

    std::vector<m::Line2d> vpc_edge {};
    for (const auto& line : f.edge)
    {
        vpc_edge.push_back({ dehomogenize_to_2d(v.t_viewport * line.start),
                             dehomogenize_to_2d(v.t_viewport * line.end) });
    }

    // Candidates are not-quite-fragments in viewport coordinates
    std::vector<Vec2f> vp_candidates {};
    for (const auto& vpc_line : vpc_edge)
    {
        const std::vector<Vec2f> l =
            midpoint_algorithm(vpc_line.start, vpc_line.end);
        vp_candidates.insert(vp_candidates.end(), l.begin(), l.end());
    }

    Fragment::Attribute a0 {};
    Fragment::Attribute a1 {};
    Fragment::Attribute a2 {};

    auto ndc2depth = [](fp x) -> fp
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

    auto noperspective = [&](Vec2f vpp, m::Vec3f c0, m::Vec3f c1,
                             m::Vec3f c2) -> m::Vec3f
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
        frag.push_back({
            vpc2pc(vpp, v),
            {noperspective(vpp, a0.ceng477_color, a1.ceng477_color,
                      a2.ceng477_color),
                      noperspective(vpp, a0.depth, a1.depth, a2.depth)}
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
    std::vector<std::vector<Pixel>> pbuffer(
        v.pixel_grid_rows,
        std::vector<Pixel>(v.pixel_grid_columns, v.bg_color));

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
        auto color = [](const m::Vec3f& c) -> m::Pixel
        {
            return { static_cast<unsigned char>(
                         c.x > 255 ? 255 : (c.x < 0 ? 0 : c.x)),
                     static_cast<unsigned char>(
                         c.y > 255 ? 255 : (c.y < 0 ? 0 : c.y)),
                     static_cast<unsigned char>(
                         c.z > 255 ? 255 : (c.z < 0 ? 0 : c.z)) };
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
        if (std::find(seen_x.begin(), seen_x.end(), d.pc.column_from_left) !=
            seen_x.end())
        {
            if (std::find(seen_y.begin(), seen_y.end(), d.pc.row_from_top) !=
                seen_y.end())
            {
                printf("OVERDRAW! At %lu %lu\n", d.pc.column_from_left,
                       d.pc.row_from_top);
            }
        }
        seen_x.push_back(d.pc.column_from_left);
        seen_y.push_back(d.pc.row_from_top);
        pbuffer[d.pc.column_from_left][d.pc.row_from_top] =
            color(d.a.ceng477_color);
    }

    return pbuffer;
}
