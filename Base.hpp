#pragma once
#include <cmath>
#include <cstddef>

namespace m
{

using fp = double;

/*****************************************************************************/
// structs and method forward declerations
/*****************************************************************************/

struct Pixel
{
    unsigned char r, g, b;
};

struct Vec2f;
struct Vec3f;
struct Vec4f;

struct Vec2f
{
    fp x, y;
};

struct Vec3f
{
    fp x, y, z;

    double operator[](size_t i) const;
    Vec3f mapto(fp a, fp b) const;
    Vec3f scale(fp a) const;
};

struct Vec4f
{
    fp x, y, z, w;

    double operator[](size_t i) const;
    Vec4f mapto(fp a, fp b) const;
    Vec4f scale(fp a) const;
};

struct Ray
{
    Vec3f o;
    Vec3f v;
};

struct LineSegment
{
    Vec3f start;
    Vec3f end;
};

using box2_d = std::pair<Vec2f, Vec2f>;

struct Matrix3
{
    Vec3f c0, c1, c2;

    Vec3f operator[](size_t i) const;
};

// TODO column-major not intuitive! write a row-major constructor!
struct Matrix4
{
    Vec4f c0, c1, c2, c3;

    Vec4f column(size_t i) const;
    Vec4f row(size_t i) const;
    Matrix4 transpose() const;
    Vec4f operator[](size_t i) const;
    Matrix4 operator*(const Matrix4& rhs) const;
};

/*****************************************************************************/
// function forward declerations
/*****************************************************************************/

Vec3f normalize(const Vec3f& a);
Matrix4 homotranslate(const Vec3f& additive);
Matrix4 homorotate(double ccw_angle, const Ray& axis);

constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs);
constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs);

/*****************************************************************************/
// constexpr function implementation
/*****************************************************************************/

constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

/*****************************************************************************/
// constexpr method implementation
/*****************************************************************************/

}   // namespace m
