#pragma once
#include "Base.hpp"
using namespace m;

// Store per-vertex data (hoping to use it for texture mapping and lighting
// later). Three "types" of per-vertex data:
//      - uniform: Absolutely NO interpolation. For a given triangle, all
//      three vertices OUGHT to have the same uniform values
//      - smooth: perspective-correct interpolation
//      - noperspective: lerp without taking perspective into account
//      - closest: no interpolation, give the closest one (OpenGL doesn't
//      have it)
//      - flat: no interpolation, give value for "provoking" vertex (Unused)
struct VertexAttribute
{
    Pixel noperspective_ceng477_color;
    Vec3f noperspective_normal;
};

struct Vertex
{
    Vec3f c;
    VertexAttribute payload;

    double operator[](size_t i) const
    {
        return c[i];
    }
};

struct FragmentAttribute
{
    fp depth;
    VertexAttribute interp;
};

// Fragments hold a payload that has data from "birthing" vertices, alongwith
// depth data. Vertex data that is computed and passed on to Fragment comes in
// multiple flavors. See VertexAttribute
// TODO making payload const ... somehow ... to avoid ever editing it
struct Fragment
{
    Vec3f clipspace;
    FragmentAttribute payload;

    double operator[](const size_t i) const;
};

class Rasterizer
{
public:
    Rasterizer() = delete;
};
