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

std::vector<Fragment> ztest(const std::vector<Fragment>& fbuffer,
                            const ViewConfig& v)
{
    struct ZAttrib
    {
        float candidate_depth;
        std::optional<size_t> candidate_index;
    };

    std::vector zbuffer(v.pixel_grid_rows,
                        std::vector(v.pixel_grid_columns, ZAttrib(1.0, {})));

    long changed = 0;

    for (size_t fragi = 0; fragi < fbuffer.size(); fragi++)
    {
        const Fragment& frag = fbuffer[fragi];
        const long row = frag.pc.row_from_top;
        const long column = frag.pc.column_from_left;
        if (row < 0 or column < 0 or row >= v.pixel_grid_rows or
            column >= v.pixel_grid_columns)
        {
            // FIXME will clipping ever work ?!
            printf("invalid fragment coordinate R%li C%li! Fix clipping!\n",
                   row, column);
            continue;
        }
        if (frag.a.depth.z <= zbuffer[row][column].candidate_depth)
        {
            changed++;
            zbuffer[row][column] = { static_cast<float>(frag.a.depth.z),
                                     { fragi } };
        }
    }

    std::vector<Fragment> out {};
    out.reserve(fbuffer.size());
    for (const auto& zfrag_row : zbuffer)
    {
        for (const auto& zfrag : zfrag_row)
        {
            if (zfrag.candidate_index.has_value())
            {
                const size_t i = zfrag.candidate_index.value();
                out.push_back(fbuffer[i]);
            }
        }
    }
    return out;
}

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
        fp step = (start.y <= end.y) ? 1.0f : -1.0f;
        for (fp y = start.y; (step > 0 ? y <= end.y : y >= end.y); y += step)
        {
            out.push_back({ start.x, y });
        }
        return out;
    }

    // Determine if the line is steep
    bool steep = std::abs(dy) > std::abs(dx);

    // Swap coordinates if the line is steep to simplify drawing
    if (steep)
    {
        std::swap(start.x, start.y);
        std::swap(end.x, end.y);
        std::swap(dx, dy);
    }

    // Ensure we always draw from left to right
    if (start.x > end.x)
    {
        std::swap(start, end);
        dx = -dx;
        dy = -dy;
    }

    // Determine the direction of increment for y
    fp y_step = (dy > 0) ? 1.0f : -1.0f;

    // Initialize decision parameter
    fp d = 2 * std::abs(dy) - dx;
    fp y = start.y;

    // Draw the line pixel by pixel
    for (fp x = start.x; x <= end.x; x += 1.0f)
    {
        // Swap back coordinates for steep lines
        if (steep)
        {
            out.push_back({ y, x });
        }
        else
        {
            out.push_back({ x, y });
        }

        // Update decision parameter and y coordinate
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

S2Face step2_device(const S1Face& f, const ViewConfig& v)
{
    const std::vector ndc { v.t_projection * f.cc0, v.t_projection * f.cc1,
                            v.t_projection * f.cc2 };
    return { f.mother, ndc[0], ndc[1], ndc[2] };
}

S3FaceCulling step3_bfc(const S2Face& f, const ViewConfig& v)
{
    Vec3f normal = surface_normal(f.ndc0.dehomogenize(), f.ndc1.dehomogenize(),
                                  f.ndc2.dehomogenize());
    Vec3f ndc_gaze =
        (v.t_camera * (v.t_projection * v.gaze.homovector())).dehomogenize();

    fp dot_product = dot(ndc_gaze, normal);
    if (v.cull_backface and dot_product >= 0)
    {
        return { f.mother, true, f.ndc0, f.ndc1, f.ndc2 };
    }
    return { f.mother, false, f.ndc0, f.ndc1, f.ndc2 };
}

S4Polygon step4_sutherland_hodgman(const S3FaceCulling& f)
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

    // TODO: eliminate degenerate cases
    // if (cross(trig_vpc1 - trig_vpc0, trig_vpc2 - trig_vpc0) == 0)
    // {
    //     return { f.mother, {} };
    // }

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

    auto ndc2depth = [](const Vec4f& ndc) -> Vec3f
    {
        auto z = ndc.dehomogenize().z;
        auto depth = (z + 1) / 2.0;
        return { 0, 0, depth };
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
    a0.depth = ndc2depth(f.trig_ndc0);
    a1.depth = ndc2depth(f.trig_ndc1);
    a2.depth = ndc2depth(f.trig_ndc2);

    // auto perspective_correct = [&](const Vec2f vpp, const Vec3f& c0,
    //                                const Vec3f& c1, const Vec3f& c2) -> Vec3f
    // {
    //     Vec3f b = barycentric(trig_vpc0, trig_vpc1, trig_vpc2, vpp);
    //     b.x = std::max(0.F, std::min(1.F, static_cast<float>(b.x)));
    //     b.y = std::max(0.F, std::min(1.F, static_cast<float>(b.y)));
    //     b.z = std::max(0.F, std::min(1.F, static_cast<float>(b.z)));

    //     const Vec3f w_reciprocal = { 1.0 / f.trig_ndc0.w, 1.0 /
    //     f.trig_ndc1.w,
    //                                  1.0 / f.trig_ndc2.w };

    //     const Vec3f c0_div_w = c0.scale(w_reciprocal.x);
    //     const Vec3f c1_div_w = c1.scale(w_reciprocal.y);
    //     const Vec3f c2_div_w = c2.scale(w_reciprocal.z);

    //     // Perform component-wise barycentric interpolation
    //     Vec3f interpolated = {
    //         b.x * c0_div_w.x + b.y * c1_div_w.x + b.z * c2_div_w.x,
    //         b.x * c0_div_w.y + b.y * c1_div_w.y + b.z * c2_div_w.y,
    //         b.x * c0_div_w.z + b.y * c1_div_w.z + b.z * c2_div_w.z,
    //     };

    //     fp interpolated_w =
    //         b.x * w_reciprocal.x + b.y * w_reciprocal.y + b.z *
    //         w_reciprocal.z;
    //     return interpolated.scale(1.F / interpolated_w);
    // };

    std::vector<Fragment> frag {};
    for (auto vpp : vp_candidates)
    {
        const Vec3f b = barycentric(trig_vpc0, trig_vpc1, trig_vpc2, vpp);
        frag.push_back({
            vpc2pc(vpp, v),
            { interpolate(b, a0.ceng477_color, a1.ceng477_color,
                      a2.ceng477_color),
                      interpolate(b, a0.depth, a1.depth, a2.depth) }
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
        const auto& s2 = step2_device(s1, v);
        const auto& s3 = step3_bfc(s2, v);
        if (s3.bfc_cullable)
        {
            continue;
        }
        const auto& s4 = step4_sutherland_hodgman(s3);
        const auto& s5 = step5_rasterize(s4, v);
        debug_ls.push_back(s5.out);
        fragments.insert(fragments.end(), s5.out.begin(), s5.out.end());
    }

    printf("resultant fragment count %lu\n", fragments.size());
    fragments = ztest(fragments, v);
    printf("surviving fragment count %lu\n", fragments.size());

    // turn surviving fragments into pixels
    std::vector<PixelCoordinate> seen_pc;
    seen_pc.reserve(fragments.size());
    for (const auto& d : fragments)
    {
        if (std::ranges::find_if(seen_pc,
                                 [&](PixelCoordinate spc)
                                 {
                                     return spc.column_from_left ==
                                                d.pc.column_from_left and
                                            spc.row_from_top ==
                                                d.pc.row_from_top;
                                 }) != seen_pc.end())
        {
            printf("overdrawn fragment At R%li C%li! fix ztest!\n",
                   d.pc.row_from_top, d.pc.column_from_left);
        }
        else
        {
            seen_pc.push_back(d.pc);
        }
        pbuffer[d.pc.column_from_left][d.pc.row_from_top] =
            m::vec2color(d.a.ceng477_color);
    }

    return pbuffer;
}
