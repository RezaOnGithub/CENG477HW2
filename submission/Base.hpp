#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace m
{

using fp = double;
constexpr double ceng_epsilon = 0.000000001;

/*****************************************************************************/
// struct forward declerations
/*****************************************************************************/

struct Pixel
{
    unsigned char r, g, b;
};

struct Vec2f;
struct Vec3f;
struct Vec4f;
struct Matrix2;
struct Matrix3;
struct Matrix4;

struct Ray;
struct HomoLine;
struct Clip;

struct IndexPair
{
    size_t row, column;
};

/*****************************************************************************/
// function forward declerations
/*****************************************************************************/

Vec3f normalize(const Vec3f& a);
Matrix4 homotranslate(const Vec3f& additive);
Matrix4 homorotate(fp ccw_angle, const Ray& axis);
Matrix4 homoscale(fp sx, fp sy, fp sz);
Clip clip_aa_inner(const Vec3f& n, const HomoLine& l,
                   fp epsilon = ceng_epsilon);
Vec3f barycentric(const Vec2f& a, const Vec2f& b, const Vec2f& c,
                  const Vec2f& p);

constexpr fp dot(const Vec2f& lhs, const Vec2f& rhs);
constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs);
constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs);
constexpr Vec3f pointwise(const Vec3f& lhs, const Vec3f& rhs);
constexpr Vec4f pointwise(const Vec4f& lhs, const Vec4f& rhs);
constexpr Vec3f cross(const Vec3f& a, const Vec3f& b);
constexpr Vec3f surface_normal(const Vec3f& v0, const Vec3f& v1,
                               const Vec3f& v2);
constexpr Pixel vec2color(const Vec3f& c);

/*****************************************************************************/
// debug forward decleration
// DONE comment these out!
/*****************************************************************************/

// void dprint(const char* s, const Vec2f& m);
// void dprint(const char* s, const Vec3f& m);
// void dprint(const char* s, const Vec4f& m);
// void dprint(const char* s, const Matrix2& m);
// void dprint(const char* s, const Matrix3& m);
// void dprint(const char* s, const Matrix4& m);

/*****************************************************************************/
// struct definition, method declerations
/*****************************************************************************/

struct Vec2f
{
    fp x, y;

    inline fp operator[](size_t i) const
    {
        switch (i)
        {
        case 0 :
            return x;
        case 1 :
            return y;
        default :
            // throw std::runtime_error("Invalid index");
            break;
        }
    }

    inline fp row(size_t i) const
    {
        return (*this)[i];
    }

    inline Vec2f scale(fp s) const
    {
        return { x * s, y * s };
    }

    Vec2f operator-(const Vec2f& v) const
    {
        return { x - v.x, y - v.y };
    }
};

struct Vec3f
{
    fp x, y, z;

    fp operator[](size_t i) const;

    inline fp row(size_t i) const
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
            // throw std::runtime_error("Invalid index");
            break;
        }
    };

    Vec3f mapto(fp a, fp b) const;
    Vec3f scale(fp a) const;
    Vec4f homopoint(fp w = 1) const;
    Vec4f homovector() const;
};

struct Vec4f
{
    fp x, y, z, w;

    fp operator[](size_t i) const;
    [[nodiscard]] Vec4f mapto(fp a, fp b) const;
    [[nodiscard]] Vec4f scale(fp a) const;
    [[nodiscard]] fp row(size_t i) const;

    // NOTE: minimize use of this function! don't lose precision!
    [[nodiscard]] constexpr Vec3f dehomogenize() const
    {
        if (w == 0)
        {
            return { x, y, z };
        }
        return { x / w, y / w, z / w };
    }
};

struct Ray
{
    Vec3f o;
    Vec3f v;
};

struct HomoLine
{
    Vec4f start, end;

    [[nodiscard]] inline HomoLine cohomogenize() const
    {
        // assert(start.w != 0 and end.w != 0);
        int sign = 1;
        if (start.w < 0)
        {
            sign *= -1;
        }
        if (end.w < 0)
        {
            sign *= -1;
        }
        return { start.scale(sign * end.w), end.scale(sign * start.w) };
    }
};

struct Line2d
{
    Vec2f start, end;
};

struct Clip
{
    HomoLine l;
    enum class ClipType
    {
        NonExistant,
        CutHead,
        CutTail,
        NoCut,
    } t;
};

class Matrix2

{
private:
    inline Matrix2(Vec2f a, Vec2f b) :
        c0(a),
        c1(b)
    {
    }
public:
    Vec2f c0, c1;

    static inline Matrix2 from_rows(const std::vector<Vec2f>& t)
    {
        return Matrix2({ t[0][0], t[1][0] }, { t[0][1], t[1][1] });
    }

    inline fp det() const
    {
        return c0[0] * c1[1] - c0[1] * c1[0];
    }

    inline Matrix2 invert() const
    {
        return Matrix2::from_rows(
            { Vec2f(column(1).row(1), column(1).row(0)).scale(1.0 / det()),
              Vec2f(column(0).row(1), column(0).row(0)).scale(1.0 / det()) });
    }

    inline Vec2f column(size_t i) const
    {
        switch (i)
        {
        case 0 :
            return c0;
        case 1 :
            return c1;
        default :
            // throw std::runtime_error("Invalid index");
            break;
        }
    }

    inline Vec2f row(size_t i) const
    {
        return { column(0).row(i), column(1).row(i) };
    }

    inline Vec2f operator*(const Vec2f& v) const
    {
        return { dot(row(0), v), dot(row(1), v) };
    }

    inline fp rc(IndexPair x) const
    {
        return column(x.column).row(x.row);
    }
};

class Matrix3
{
private:
    inline Matrix3(Vec3f a, Vec3f b, Vec3f c) :
        c0(a),
        c1(b),
        c2(c)
    {
    }
public:
    Vec3f c0, c1, c2;

    static inline Matrix3 from_columns(const std::vector<Vec3f>& m)
    {
        return { m[0], m[1], m[2] };
    }

    static inline Matrix3 from_rows(const std::vector<Vec3f>& t)
    {
        return from_columns(t).transpose();
    }

    static inline Matrix3 i3()
    {
        return {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
        };
    }

    inline fp rc(IndexPair x) const
    {
        // printf("ROW %lu COLUMN %lu\t:\t%f\n", x.row, x.column,
        //        column(x.column).row(x.row));
        // dprint("\t\t rc Column", column(x.column));
        return column(x.column).row(x.row);
    }

    Vec3f column(size_t i) const;
    Vec3f row(size_t i) const;
    fp det() const;
    fp minor(IndexPair x) const;
    Matrix3 transpose() const;
    Vec3f operator*(const Vec3f& v) const;
    [[nodiscard]] Matrix3 invert() const;
    [[nodiscard]] Matrix3 scale(fp s) const;
};

class Matrix4
{
private:
    inline Matrix4(Vec4f a, Vec4f b, Vec4f c, Vec4f d) :
        c0(a),
        c1(b),
        c2(c),
        c3(d)
    {
    }
public:
    Vec4f c0, c1, c2, c3;

    static inline Matrix4 from_columns(const std::vector<Vec4f>& m)
    {
        return { m[0], m[1], m[2], m[3] };
    }

    static inline Matrix4 from_rows(const std::vector<Vec4f>& t)
    {
        return from_columns(t).transpose();
    }

    static inline Matrix4 i4()
    {
        return {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        };
    }

    inline fp rc(IndexPair x) const
    {
        return column(x.column).row(x.row);
    }

    Vec4f column(size_t i) const;
    Vec4f row(size_t i) const;
    Matrix4 transpose() const;
    Matrix4 operator*(const Matrix4& rhs) const;
    Vec4f operator*(const Vec4f& rhs) const;
    fp minor(IndexPair x) const;
    fp det() const;
    Matrix4 scale(fp fp) const;
    Matrix4 invert() const;
};

/*****************************************************************************/
// constexpr function implementation
/*****************************************************************************/

constexpr fp dot(const Vec2f& lhs, const Vec2f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

constexpr fp dot(const Vec3f& lhs, const Vec3f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

constexpr fp dot(const Vec4f& lhs, const Vec4f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

constexpr Vec3f pointwise(const Vec3f& lhs, const Vec3f& rhs)
{
    return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

constexpr Vec4f pointwise(const Vec4f& lhs, const Vec4f& rhs)
{
    return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w };
}

constexpr Vec3f cross(const Vec3f& a, const Vec3f& b)
{
    return {
        a.y * b.z - a.z * b.y,
        -1 * a.x * b.z + a.z * b.x,
        a.x * b.y - a.y * b.x,
    };
}

// take winding order and such into account
// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
constexpr Vec3f surface_normal(const Vec3f& v0, const Vec3f& v1,
                               const Vec3f& v2)
{
    const Vec3f u = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
    const Vec3f v = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
    return { u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
             u.x * v.y - u.y * v.x };
}

constexpr Pixel vec2color(const Vec3f& c)
{
    return { static_cast<unsigned char>(c.x > 255 ? 255 :
                                        c.x < 0   ? 0 :
                                                    c.x),
             static_cast<unsigned char>(c.y > 255 ? 255 :
                                        c.y < 0   ? 0 :
                                                    c.y),
             static_cast<unsigned char>(c.z > 255 ? 255 :
                                        c.z < 0   ? 0 :
                                                    c.z) };
};

/*****************************************************************************/
// misc
/*****************************************************************************/

inline bool eq_within(fp i, fp j, fp ep)
{
    // assert(ep >= 0);
    return i > j ? i - j <= ep : j - i <= ep;
}

inline bool within(fp x, fp a, fp b)
{
    // assert(a < b);
    return x - a > 0 and b - x > 0;
}

// Interpolate function for barycentric coordinates
Vec3f interpolate(const Vec3f& b, const Vec3f& c0, const Vec3f& c1,
                  const Vec3f& c2);

// Interpolate for depth (assuming depth is scalar)
fp interpolate(const Vec3f& b, const fp& d0, const fp& d1, const fp& d2);

/*****************************************************************************/
// for debugging
// DONE comment these out when done!
/*****************************************************************************/

// inline void dprint(const char* const s, const Matrix4& m)
// {
//     printf("Matrix4 %s:\n", s);
//     for (size_t i = 0; i < 4; i++)
//     {
//         for (size_t j = 0; j < 4; j++)
//         {
//             printf("\t%f\t", m.rc({ i, j }));
//         }
//         puts("");
//     }
// }

// inline void dprint(const char* const s, const Matrix3& m)
// {
//     printf("Matrix3 %s:\n", s);
//     for (size_t i = 0; i < 3; i++)
//     {
//         for (size_t j = 0; j < 3; j++)
//         {
//             printf("\t%f\t", m.rc({ i, j }));
//         }
//         puts("");
//     }
// }

// inline void dprint(const char* const s, const Matrix2& m)
// {
//     printf("Matrix2 %s:\n", s);
//     for (size_t i = 0; i < 2; i++)
//     {
//         for (size_t j = 0; j < 2; j++)
//         {
//             printf("\t%f\t", m.rc({ i, j }));
//         }
//         puts("");
//     }
// }

// inline void dprint(const char* const s, const Vec4f& m)
// {
//     printf("Vec4f %s:\n", s);
//     for (size_t i = 0; i < 4; i++)
//     {
//         printf("\t%f\n", m.row(i));
//     }
//     puts("");
// }

// inline void dprint(const char* const s, const Vec3f& m)
// {
//     printf("Vec3f %s:\n", s);
//     for (size_t i = 0; i < 3; i++)
//     {
//         printf("\t%f\n", m.row(i));
//     }
//     puts("");
// }

// inline void dprint(const char* const s, const Vec2f& m)
// {
//     printf("Vec3f %s:\n", s);
//     for (size_t i = 0; i < 2; i++)
//     {
//         printf("\t%f\n", m.row(i));
//     }
//     puts("");
// }

// inline void dprint(const char* const s, fp m)
// {
//     printf("double %s:\t%f\n", s, m);
// }

}   // namespace m
