#pragma once
#include <cmath>
#include <cstddef>
#include <cstdio>

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
    Vec3f  mapto(fp a, fp b) const;
    Vec3f  scale(fp a) const;
    Vec4f  homopoint(fp w = 1) const;
    Vec4f  homovector() const;
};

struct Vec4f
{
    fp x, y, z, w;

    double operator[](size_t i) const;
    Vec4f  mapto(fp a, fp b) const;
    Vec4f  scale(fp a) const;
    fp     row(size_t i) const;
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

    // Right-Handed Normal
    fp    line_equation(fp x);
    Vec2f normal();
};

struct Box
{
    Vec2f a, b;

    fp minx() const;
    fp maxx() const;
    fp miny() const;
    fp maxy() const;
};

struct Matrix3
{
    Vec3f c0, c1, c2;

    Vec3f   column(size_t i) const;
    Vec3f   row(size_t i) const;
    fp      det() const;
    Matrix3 transpose() const;
};

// TODO column-major not intuitive! write a row-major constructor!
struct Matrix4
{
    Vec4f c0, c1, c2, c3;

    Vec4f   column(size_t i) const;
    Vec4f   row(size_t i) const;
    Matrix4 transpose() const;
    Matrix4 operator*(const Matrix4& rhs) const;
    fp      minor(size_t row, size_t col) const;
    fp      det() const;
    Matrix4 scale(fp fp);
    Matrix4 invert() const;
};

/*****************************************************************************/
// function forward declerations
/*****************************************************************************/

Vec3f   normalize(const Vec3f& a);
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

/*****************************************************************************/
// for debugging
// TODO comment these out when done!
/*****************************************************************************/

inline bool eq_within(const float& i, const float& j, const float& ep)
{
    // assert(ep >= 0);
    return i > j ? i - j <= ep : j - i <= ep;
}

inline void dprint(const char* const s, const Matrix4& m)
{
    printf("Matrix4 %s:\n", s);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("\t%f\t", m.column(j).row(i));
        }
        puts("");
    }
}

}   // namespace m
