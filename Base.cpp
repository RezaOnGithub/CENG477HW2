#include "Base.hpp"

#include <cmath>
#include <string>

namespace m
{
double Vec3f::operator[](size_t i) const
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
        // Locked behind C++23 (would've also been constexpr-compatible)
        // std::unreachable();
        throw std::string("Invalid index");
    }
}

Vec3f Vec3f::mapto(fp a, fp b) const
{
    return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z) };
}

Vec3f Vec3f::scale(fp a) const
{
    return { x * a, y * a, z * a };
}

Vec3f Matrix3::operator[](size_t i) const
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
        throw std::string("Invalid index");
    }
}

double Vec4f::operator[](size_t i) const
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
        // Locked behind C++23 (would've also been constexpr-compatible)
        // std::unreachable();
        throw std::string("Invalid index");
    }
}

Vec4f Vec4f::mapto(fp a, fp b) const
{
    return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z),
             std::lerp(a, b, w) };
}

Vec4f Vec4f::scale(fp a) const
{
    return { x * a, y * a, z * a, w * a };
}

Vec4f Matrix4::column(size_t i) const
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

Vec4f Matrix4::row(size_t i) const
{
    return { c0[i], c1[i], c2[i], c3[i] };
}

Matrix4 Matrix4::transpose() const
{
    return { row(0), row(1), row(2), row(3) };
}

Vec4f Matrix4::operator[](size_t i) const
{
    return row(i);
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    // because operator[] does not return a reference, can't write it in
    // loop form
    const auto& self = *this;
    auto rc = [self, rhs](size_t i, size_t j) constexpr
    {
        return dot(self.row(i), rhs.column(j));
    };
    const Matrix4 t {
        { rc(0, 0), rc(0, 1), rc(0, 2), rc(0, 3) },
        { rc(1, 0), rc(1, 1), rc(1, 2), rc(1, 3) },
        { rc(2, 0), rc(2, 1), rc(2, 2), rc(2, 3) },
        { rc(3, 0), rc(3, 1), rc(3, 2), rc(3, 3) },
    };
    return t.transpose();
}

Vec3f pointwise(const Vec3f& l, const Vec3f& r)
{
    return {
        l.x * r.x,
        l.y * r.y,
        l.z * r.z,
    };
}

Vec4f pointwise(const Vec4f& l, const Vec4f& r)
{
    return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w };
}

Vec4f scale(fp l, const Vec4f& r)
{
    return { l * r.x, l * r.y, l * r.z, l * r.w };
}

Vec3f scale(fp l, const Vec3f& r)
{
    return { l * r.x, l * r.y, l * r.z };
}

Pixel lerp(Pixel a, Pixel b, fp t)
{
    return { static_cast<unsigned char>(std::lerp(a.r, b.r, t)),
             static_cast<unsigned char>(std::lerp(a.g, b.g, t)),
             static_cast<unsigned char>(std::lerp(a.b, b.b, t)) };
}

Vec3f normalize(const Vec3f& a)
{
    const fp len = sqrt(dot(a, a));
    return { a.x / len, a.y / len, a.z / len };
}

Matrix4 homotranslate(const Vec3f& disp)
{
    // disp stands for "displacement"
    const Matrix4 t = {
        { 1, 0, 0, disp.x },
        { 0, 1, 0, disp.y },
        { 0, 0, 1, disp.z },
        { 0, 0, 0, 1      }
    };
    return t.transpose();
}

// Axis direction is significant! Right-hand curl!
Matrix4 homorotate(double ccw_angle, const Ray& axis)
{
    Vec3f u = normalize(axis.v);
    const Matrix4 step1_translate_away_from_origin =
        homotranslate(axis.o.scale(-1));
    // const Matrix4 Matrix4 translate_back_to_origin =
    //     homotranslate(axis.o);
    return {};
}

};   // namespace m
