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

inline m::Matrix4 viewport_transformation(const ViewConfig& v)
{
    return Matrix4::from_rows({
        {v.pixel_grid_rows / 2.0, 0,                          0, (v.pixel_grid_rows - 1) / 2.0   },
        { 0,                      v.pixel_grid_columns / 2.0, 0, (v.pixel_grid_columns - 1) / 2.0},
        { 0,                      0,                          1, 0                               },
        { 0,                      0,                          0, 1                               }
    });
}

RasterTriangle rasterize(const World& w, const ViewConfig& v,
                         size_t face_index);
