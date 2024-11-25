#pragma once
#include <math.h>

constexpr double lerp(double x, double lo, double hi)
{
    // TODO
    return NAN;
}

struct Vec3f
{
    double x, y, z;

    constexpr double operator[](size_t i) const
    {
        switch (i)
        {
        case 0 :
            return x;
        case 1 :
            return y;
        case 2 :
            return z;
        default :
            return NAN;
        }
    }

    constexpr Vec3f lerp(double a, double b)
    {
        return { NAN, NAN, NAN };
    }
};

struct Vertex
{
};

struct VertexPayload
{
};

struct Frag
{
};
