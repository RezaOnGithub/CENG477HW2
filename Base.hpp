#pragma once
#include <cmath>
#include <cstddef>
#include <sys/types.h>

using F = double;

struct Vec3f
{
    F x, y, z;

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
            // Locked behind C++23
            // std::unreachable();
            return NAN;
        }
    }

    [[nodiscard]] constexpr Vec3f lerp(F a, F b) const
    {
        return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z) };
    }
};

struct Matrix3
{
    Vec3f c0, c1, c2;

    [[nodiscard]] constexpr Vec3f operator[](size_t i) const
    {
        switch (i)
        {
        case 0 :
            return c0;
        case 1 :
            return c1;
        case 2 :
            return c2;
        default :
            // Locked behind C++23
            // std::unreachable();
            return { NAN, NAN, NAN };
        }
    }
};

struct Vec4f
{
    F x, y, z, w;

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
        case 3 :
            return w;
        default :
            // Locked behind C++23
            // std::unreachable();
            return NAN;
        }
    }

    [[nodiscard]] constexpr Vec4f lerp(F a, F b) const
    {
        return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z),
                 std::lerp(a, b, w) };
    }
};

struct Matrix4
{
    Vec4f c0, c1, c2, c3;

    [[nodiscard]] constexpr Vec4f operator[](size_t i) const
    {
        switch (i)
        {
        case 0 :
            return c0;
        case 1 :
            return c1;
        case 2 :
            return c2;
        case 3 :
            return c3;
        default :
            // Locked behind C++23
            // std::unreachable();
            return { NAN, NAN, NAN, NAN };
        }
    }
};

[[nodiscard]] constexpr Vec3f pointwise(const Vec3f& l, const Vec3f& r)
{
    return {
        l.x * r.x,
        l.y * r.y,
        l.z * r.z,
    };
}

[[nodiscard]] constexpr Vec4f pointwise(const Vec4f& l, const Vec4f& r)
{
    return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w };
}

struct Pixel
{
    unsigned char r, g, b;
    // [[nodiscard]] constexpr Pixel gamma_correction(F gamma) {}x
};

// Store per-vertex data (hoping to use it for texture mapping and lighting
// later). Three "types" of per-vertex data:
//      - uniform: Absolutely NO interpolation. For a given triangle, all three
//      vertices HAVE to have the same uniform values
//      - smooth: perspective-correct interpolation
//      - noperspective: lerp without taking perspective into account
//      - closest: no interpolation, give the closest one (OpenGL doesn't have
//      it) (UNUSED)
//      - flat: no interpolation, give value for "provoking" vertex (OpenGL has
//      it) (UNUSED)
struct VertexPayload
{
    double noperspective_color;   // CENG477 specific
    Vec3f noperspective_normal;
};

// TODO making payload const ... somehow ... to avoid ever editing it
struct Vertex
{
    Vec3f c;
    VertexPayload payload;

    constexpr double operator[](size_t i) const
    {
        return c[i];
    }
};

struct FragmentPayload
{
    F depth;
    VertexPayload interp;
};

// Fragments hold a payload that has data from "birthing" vertices, alongwith
// depth data. Vertex data that is computed and passed on to Fragment comes in
// multiple flavors. See VertexPayload
// TODO making payload const ... somehow ... to avoid ever editing it
struct Fragment
{
    Vec3f c;
    FragmentPayload payload;

    constexpr double operator[](size_t i) const
    {
        return c[i];
    }
};
