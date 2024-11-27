#pragma once
#include <cmath>
#include <cstddef>

namespace m
{

using fp = double;

// declerations needed for constexpr
struct Vec3f;
struct Vec4f;
constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs);
constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs);

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

// TODO Design Mistake: because I was too lazy to write a row-oriented
// constructor, there are places I write things in row format (visually more
// intuitive/standard) but then have to transpose Make sure a transpose takes
// place in such spots!
struct Matrix4
{
    Vec4f c0, c1, c2, c3;

    [[nodiscard]] constexpr Vec4f column(size_t i) const
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

    [[nodiscard]] constexpr Vec4f row(size_t i) const
    {
        return { c0[i], c1[i], c2[i], c3[i] };
    }

    [[nodiscard]] constexpr Matrix4 transpose() const
    {
        return { row(0), row(1), row(2), row(3) };
    }

    [[nodiscard]] constexpr Vec4f operator[](size_t i) const
    {
        return row(i);
    }

    [[nodiscard]] constexpr Matrix4 operator*(const Matrix4& rhs) const
    {
        // because operator[] does not return a reference, can't write it in
        // loop form
        const auto& self = *this;
        auto rc = [self, rhs](size_t i, size_t j) constexpr
        {
            return dot(self.row(i), rhs.column(j));
        };
        const Matrix4 t {
            {rc(0,  0), rc(0, 1), rc(0, 2), rc(0, 3)},
            { rc(1, 0), rc(1, 1), rc(1, 2), rc(1, 3)},
            { rc(2, 0), rc(2, 1), rc(2, 2), rc(2, 3)},
            { rc(3, 0), rc(3, 1), rc(3, 2), rc(3, 3)},
        };
        return t.transpose();
    }
};

[[nodiscard]] constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[nodiscard]] constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

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
