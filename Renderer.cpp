#include "Renderer.hpp"

#include "Base.hpp"
#include "Rasterizer.hpp"
#include "World.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

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
