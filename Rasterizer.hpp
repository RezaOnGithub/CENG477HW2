#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cstddef>
#include <utility>
#include <vector>
using namespace m;

struct PixelCoordinate;
struct FragmentAttributeFinal;
struct Fragment;
struct RasterTriangle;

struct PixelCoordinate
{
    long row_from_top;
    long column_from_left;
};

struct FragmentAttributeFinal
{
    m::Pixel ceng477_color;
    float depth;
};

struct Fragment
{
    PixelCoordinate pc;
    // Vec4f vpc;
    FragmentAttributeFinal attrib;
};

struct RasterTriangle
{
    Face mother;
    std::vector<Fragment> out;
};

RasterTriangle rasterize(const World& w, const ViewConfig& v,
                         size_t face_index);
