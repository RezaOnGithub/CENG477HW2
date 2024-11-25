#pragma once
#include <cmath>
#include <cstddef>

namespace m
{

using fp = double;

struct Vec3f
{
    fp x, y, z;

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

    [[nodiscard]] constexpr Vec3f mapto(fp a, fp b) const
    {
        return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z) };
    }

    [[nodiscard]] constexpr Vec3f scale(fp a) const
    {
        return { x * a, y * a, z * a };
    }
};

[[nodiscard]] constexpr fp dot(const Vec3f& v, const Vec3f& w)
{
    return v.x * w.x + v.y * w.y + v.z * w.z;
}

struct Ray
{
    Vec3f o;
    Vec3f v;
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
    fp x, y, z, w;

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

    [[nodiscard]] constexpr Vec4f mapto(fp a, fp b) const
    {
        return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z),
                 std::lerp(a, b, w) };
    }

    [[nodiscard]] constexpr Vec4f scale(fp a) const
    {
        return { x * a, y * a, z * a, w * a };
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

[[nodiscard]] constexpr Vec4f scale(fp l, const Vec4f& r)
{
    return { l * r.x, l * r.y, l * r.z, l * r.w };
}

[[nodiscard]] constexpr Vec3f scale(fp l, const Vec3f& r)
{
    return { l * r.x, l * r.y, l * r.z };
}

struct Pixel
{
    unsigned char r, g, b;

    // [[nodiscard]] constexpr Pixel gamma_correction(F gamma) {}
};

[[nodiscard]] constexpr Pixel lerp(Pixel a, Pixel b, fp t)
{
    return { static_cast<unsigned char>(std::lerp(a.r, b.r, t)),
             static_cast<unsigned char>(std::lerp(a.g, b.g, t)),
             static_cast<unsigned char>(std::lerp(a.b, b.b, t)) };
}

// // Calculate homogenous equation of a line, and plug an arbitrary point.
// // Direction matters!
// [[nodiscard]] constexpr double line(const Ray& r, const Vec3f& p)
// {
// }

// non-constexpr functions
Vec3f normalize(const Vec3f& a);
Matrix4 homotranslate(const Vec3f& additive);
Matrix4 homorotate(double ccw_angle, const Ray& axis);

}   // namespace m
