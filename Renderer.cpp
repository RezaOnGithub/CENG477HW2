#include "Renderer.hpp"

#include "Base.hpp"
#include "Rasterizer.hpp"
#include "World.hpp"

#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

S1Face step1_camera(const Face& f, const ViewConfig& v)
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
        return {};
    }
    return { f };
}

S3Face step3_device(const S1Face& f, const ViewConfig& v)
{
    std::vector<Vec4f> ndc { v.t_projection * f.v0, v.t_projection * f.v1,
                             v.t_projection * f.v2 };
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
    std::vector<Vec4f> ndc { f.v0, f.v1, f.v2 };
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
    return { f.mother, ndc };
}

// BEWARE: mutable reference to zbuffer
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
