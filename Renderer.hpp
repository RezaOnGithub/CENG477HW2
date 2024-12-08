#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cstddef>
#include <vector>

namespace renderer
{
using GenericAttribute = m::Vec3f;

struct AttributeArray
{
    std::vector<GenericAttribute> a;

    constexpr GenericAttribute operator[](size_t i)
    {
        return a[i];
    }
};

constexpr size_t i_noperspective_ceng477_color = 1;

constexpr m::Pixel a_ceng477_color(const GenericAttribute& a)
{
    return { static_cast<unsigned char>(a.x), static_cast<unsigned char>(a.y),
             static_cast<unsigned char>(a.z) };
}

constexpr size_t i_noperspective_depth = 2;

constexpr m::fp a_depth(const GenericAttribute& a)
{
    return a.z;
}

struct S1Face
{
    WorldFace mother;
    m::Vec4f cc0, cc1, cc2;
};

struct S2Face
{
    WorldFace mother;
    bool bfc_cullable;
    m::Vec4f cc0, cc1, cc2;
};

struct S3Face
{
    WorldFace mother;
    m::Vec4f ndc0, ndc1, ndc2;
};

struct S4Polygon
{
    WorldFace mother;
    m::Vec4f trig_ndc0, trig_ndc1, trig_ndc2;
    std::vector<m::Vec4f> poly_ndc;
};

struct PixelCoordinate
{
    long row_from_top;
    long column_from_left;
};

struct Fragment
{
    PixelCoordinate pc;
    AttributeArray a;
};

struct S5Raster
{
    WorldFace mother;
    std::vector<Fragment> out;
};
}   // namespace renderer

// All-in-one
std::vector<std::vector<m::Pixel>> render(const World& w, const ViewConfig& v);
