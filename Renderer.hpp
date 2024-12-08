#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cassert>
#include <cstddef>
#include <vector>

namespace renderer
{
using GenericAttribute = m::Vec3f;

class AttributeArray
{
public:
    std::vector<GenericAttribute> a;

    AttributeArray() :
        a(16, { 0, 0, 0 })
    {
    }

    AttributeArray(const std::vector<GenericAttribute>& rhs) :
        a(rhs)
    {
        assert(rhs.size() <= 16);
        int diff = 16 - rhs.size();
        while (diff-- > 0)
        {
            a.emplace_back();
        }
        assert(a.size() == 16);
    }

    inline const GenericAttribute& operator[](size_t i) const
    {
        return a[i];
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
