#include "Renderer.hpp"

#include "Base.hpp"
#include "World.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

using namespace m;
using namespace renderer;

/*****************************************************************************/
// helper functions
/*****************************************************************************/

constexpr PixelCoordinate vpc2pc(const Vec2f& vpc, const ViewConfig& v)
{
    return { static_cast<long>(vpc.x - (v.pixel_grid_columns / 2.0)),
             static_cast<long>(-vpc.y + (v.pixel_grid_rows / 2.0)) };
}

// TODO this is completely wrong. Use a zfail approach!
std::vector<Fragment> ztest(std::vector<std::vector<float>>& zbuffer,
                            const std::vector<Fragment>& fbuffer)
{
    std::vector<Fragment> out {};
    for (auto frag : fbuffer)
    {
        const size_t row = frag.pc.row_from_top;
        const size_t column = frag.pc.column_from_left;
        if (zbuffer[row][column] > frag.attrib.depth)
        {
            out.push_back(frag);
            zbuffer[row][column] = frag.attrib.depth;
        }
    }
    return out;
}

std::vector<Vec2f> midpoint_algorithm(Vec2f start, Vec2f end)
{
    assert(std::isfinite(start.x) and std::isfinite(end.x) and
           std::isfinite(start.y) and std::isfinite(end.y));
    if (start.x > end.x)
    {
        return midpoint_algorithm(end, start);
    }
    // end.x >= start.x or we have a bad FP non-value

    // it is a lambda because the division may be undefined
    auto slope = [start, end]()
    {
        return (end.y - start.y) / (end.x - start.x);
    };

    auto interpolate = [start, end, slope](fp x)
    {
        const fp a = slope();
        fp y = start.y;
        while (x++ <= end.x)
        {
            y += a;
        }
        return y;
    };

    std::vector<Vec2f> out {};

    if (end.x > start.x)
    {
        // Classical Case: the line is gently going up
        // TODO kinda dangerous FP loop?
        for (fp px = start.x; px <= end.x; px++)
        {
            out.push_back({ px, interpolate(px) });
        }
    }
    else if (end.x == start.x)
    {
        // Line is perfectly verical
        for (fp py = start.y; py <= start.y; py++)
        {
            out.push_back({ end.x, py });
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
    Vec4f normal =
        v.t_camera * f.mother.uniform.uniform_surface_normal.homovector();
    if (v.cull_backface and dot(v.gaze.homovector(), normal) >= 0)
    {
        return { f.mother, true, f.cc0, f.cc1, f.cc2 };
    }
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
    std::vector<Vec4f> ndc { f.ndc0, f.ndc1, f.ndc2 };
    for (auto normal : plane_normals)
    {
        std::vector<Vec4f> new_ndc {};
        for (int i = 0; i < ndc.size(); i++)
        {
            const Vec4f first = ndc[i];
            const Vec4f second = ndc[(i + 1) % ndc.size()];
            auto clip = clip_aa_inner(normal, { first, second });
            // TODO does it even make a difference if I used clip's output?
            switch (clip.t)
            {
            case m::Clip::ClipType::NonExistant :
                // Hope for the best!
                break;
            case m::Clip::ClipType::CutHead :
                ndc.push_back(clip.l.start);
                ndc.push_back(second);
                break;
            case m::Clip::ClipType::CutTail :
                ndc.push_back(first);
                ndc.push_back(clip.l.end);
                break;
            case m::Clip::ClipType::NoCut :
                ndc.push_back(first);
                ndc.push_back(second);
                break;
            }
        }
        ndc = new_ndc;
    }
    return { f.mother, f.ndc0, f.ndc1, f.ndc2, ndc };
}

S5Raster step5_rasterize(const S4Polygon& f, const ViewConfig& v)
{
    // TODO currently focusing to get wirefram right. implement solid later!

    auto dehomogenize_to_2d = [](Vec4f x) -> Vec2f
    {
        return { x.dehomogenize().x, x.dehomogenize().y };
    };
    const Vec2f trig_vpc0 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc0);
    const Vec2f trig_vpc1 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc1);
    const Vec2f trig_vpc2 = dehomogenize_to_2d(v.t_viewport * f.trig_ndc2);
    // TODO how do you "map" in C++?
    std::vector<Vec2f> poly_vpc {};
    for (auto x : f.poly_ndc)
    {
        poly_vpc.push_back(dehomogenize_to_2d(v.t_viewport * x));
    }

    std::vector<Vec2f> vp_candidates {};
    for (size_t i = 0; i < poly_vpc.size(); i++)
    {
        // FIXME implement a hack so that clipped boundaries don't get drawn!
        const size_t ei = (i + 1) % poly_vpc.size();
        const Vec2f start = poly_vpc[i];
        const Vec2f end = poly_vpc[ei];
        const std::vector<Vec2f> l = midpoint_algorithm(start, end);
        vp_candidates.insert(vp_candidates.end(), l.begin(), l.end());
    }

    auto noperspective = [&](Vec2f vpp, size_t attrib_index) -> GenericAttribute
    {
        // perform barycentric interpolation for each component of generic
        // attribute (currently x,y,z)
        const Vec3f b = barycentric(trig_vpc0, trig_vpc1, trig_vpc2, vpp);
        const Vec3f axs = { f.mother.v0.interp[attrib_index].x,
                            f.mother.v1.interp[attrib_index].x,
                            f.mother.v2.interp[attrib_index].x };
        const Vec3f ays = { f.mother.v0.interp[attrib_index].y,
                            f.mother.v1.interp[attrib_index].y,
                            f.mother.v2.interp[attrib_index].y };
        const Vec3f azs = { f.mother.v0.interp[attrib_index].z,
                            f.mother.v1.interp[attrib_index].z,
                            f.mother.v2.interp[attrib_index].z };
        return { dot(b, axs), dot(b, ays), dot(b, azs) };
    };

    std::vector<Fragment> frag {};
    for (auto vpp : vp_candidates)
    {
        frag.push_back({
            vpc2pc(vpp, v),
            {noperspective(vpp, i_noperspective_ceng477_color),
                      noperspective(vpp, i_noperspective_depth)}
        });
    }

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

// Rasterize every triangle, perform depth test, write to an output buffer
std::vector<std::vector<Pixel>> render(const World& w, const ViewConfig& v)
{
    const long count = w.face_count();
    std::vector<Fragment> fragments {};
    std::vector<Fragment> draw {};
    std::vector<std::vector<float>> zbuffer(
        v.pixel_grid_rows, std::vector<float>(v.pixel_grid_columns, 1.0F));
    std::vector<std::vector<Pixel>> pbuffer(
        v.pixel_grid_rows,
        std::vector<Pixel>(v.pixel_grid_columns, v.bg_color));

    // rasterize every triangle in the scene
    for (size_t i = 0; i < count; i++)
    {
        auto raster = rasterize(w, v, i);
        fragments.insert(fragments.end(), raster.out.begin(), raster.out.end());
    }

    // perform depth-test
    draw = ztest(zbuffer, fragments);

    // turn surviving fragments into pixels
    for (auto d : draw)
    {
        pbuffer[d.pc.row_from_top][d.pc.column_from_left] =
            d.attrib.ceng477_color;
    }

    return pbuffer;
}
