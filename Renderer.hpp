#pragma once
#include "Base.hpp"
#include "World.hpp"
#include <optional>
#include <vector>

struct S1Face
{
    Face mother;
    m::Vec4f v0, v1, v2;
};

using S2Face = std::optional<S1Face>;

struct S3Face
{
    Face mother;
    m::Vec4f v0, v1, v2;
};

struct S4Polygon {
    Face mother;
    std::vector<m::Vec4f> vpc;
};

struct S5Raster {

};

// Rasterize every triangle, perform depth test, return an output buffer
std::vector<std::vector<m::Pixel>> render(const World &w, const ViewConfig &v);