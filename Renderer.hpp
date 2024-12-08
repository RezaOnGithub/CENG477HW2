#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cstddef>
#include <vector>

namespace renderer
{
using GenericAttribute = m::Vec3f;

class AttributeArray
{
private:
    std::vector<GenericAttribute> a;
public:
    AttributeArray() :
        a(16, { 0, 0, 0 })
    {
    }

    inline GenericAttribute& operator[](size_t i)
    {
        return a[i];
    }

    [[nodiscard]] inline m::Pixel ceng477_color() const
    {
        return { static_cast<unsigned char>(a[1].x),
                 static_cast<unsigned char>(a[1].y),
                 static_cast<unsigned char>(a[1].z) };
    }

    [[nodiscard]] inline float depth() const
    {
        return static_cast<float>(a[2].z);
    }
};

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
