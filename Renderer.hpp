#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <utility>
#include <vector>

namespace renderer
{

struct S1Face
{
    WorldFace mother;
    m::Vec4f cc0, cc1, cc2;
};

struct S2Face
{
    WorldFace mother;
    m::Vec4f ndc0, ndc1, ndc2;
};

struct S3FaceCulling
{
    WorldFace mother;
    bool bfc_cullable;
    m::Vec4f ndc0, ndc1, ndc2;
};

struct S4Polygon
{
    WorldFace mother;
    m::Vec4f trig_ndc0, trig_ndc1, trig_ndc2;
    std::vector<m::HomoLine> edge;
};

struct PixelCoordinate
{
    long row_from_top;
    long column_from_left;
};

struct Fragment
{
    PixelCoordinate pc;

    struct Attribute
    {
        m::Vec3f ceng477_color;
        m::Vec3f depth;
    } a;
};

struct S5Raster
{
    WorldFace mother;
    std::vector<Fragment> out;
};
}   // namespace renderer

// All-in-one
std::vector<std::vector<m::Pixel>> render(const World& w, const ViewConfig& v);
std::vector<std::vector<std::vector<m::Pixel>>>
    render_layers(const World& w, const ViewConfig& v);
