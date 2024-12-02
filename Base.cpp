#include "Base.hpp"

#include <cmath>
#include <string>
#include <vector>

namespace m
{

/*****************************************************************************/
// Vec3f Methods
/*****************************************************************************/
double Vec3f::operator[](const size_t i) const
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

Vec3f Vec3f::mapto(const fp a, const fp b) const
{
    return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z) };
}

Vec3f Vec3f::scale(const fp a) const
{
    return { x * a, y * a, z * a };
}

Vec4f Vec3f::homopoint(const fp w) const
{
    return { x, y, z, w };
}

Vec4f Vec3f::homovector() const
{
    return { x, y, z, 0 };
}

/*****************************************************************************/
// Vec4f Methods
/*****************************************************************************/

double Vec4f::operator[](const size_t i) const
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

Vec4f Vec4f::mapto(const fp a, const fp b) const
{
    return { std::lerp(a, b, x), std::lerp(a, b, y), std::lerp(a, b, z),
             std::lerp(a, b, w) };
}

Vec4f Vec4f::scale(const fp a) const
{
    return { x * a, y * a, z * a, w * a };
}

fp Vec4f::row(const size_t i) const
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
        throw std::string("Invalid index");
    }
}

/*****************************************************************************/
// Matrix3 Methods
/*****************************************************************************/

Vec3f Matrix3::row(size_t i) const
{
    return { c0[i], c1[i], c2[i] };
}

Vec3f Matrix3::column(size_t i) const
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
        throw std::string("Invalid index");
    }
}

fp Matrix3::det() const
{
    const auto r = row(0);
    const auto d0 = c1[1] * c2[2] - c2[1] * c1[2];
    const auto d1 = c0[1] * c2[2] - c2[1] * c0[2];
    const auto d2 = c0[1] * c1[2] - c1[1] * c0[2];
    return r[0] * d0 - r[1] * d1 + r[2] * d2;
}

Matrix3 Matrix3::transpose() const
{
    return { row(0), row(1), row(2) };
}

/*****************************************************************************/
// Matrix4 Methods
/*****************************************************************************/

Vec4f Matrix4::column(const size_t i) const
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
        throw std::string("Invalid Index!");
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

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    // because operator[] does not return a reference, can't write it in
    // loop form
    const auto& self = *this;
    auto rc = [self, rhs](size_t i, size_t j) constexpr
    {
        return dot(self.row(i), rhs.column(j));
    };
    return Matrix4::from_rows({
                                  {rc(0,  0), rc(0, 1), rc(0, 2), rc(0, 3)},
                                  { rc(1, 0), rc(1, 1), rc(1, 2), rc(1, 3)},
                                  { rc(2, 0), rc(2, 1), rc(2, 2), rc(2, 3)},
                                  { rc(3, 0), rc(3, 1), rc(3, 2), rc(3, 3)},
    })
        .transpose();
}

fp Matrix4::minor(size_t r, size_t c) const
{
    std::vector<std::pair<size_t, size_t>> p {};
    for (int i = 0; i < 4; ++i)
    {
        if (i == r)
        {
            continue;
        }
        for (int j = 0; j < 4; j++)
        {
            if (j == c)
            {
                continue;
            }
            p.push_back({ i, j });
        }
    }

    return Matrix3::from_rows({
                                  {column(p[0].first).row(p[0].second),
                                   column(p[1].first).row(p[1].second),
                                   column(p[2].first).row(p[2].second)},
                                  { column(p[3].first).row(p[3].second),
                                   column(p[4].first).row(p[4].second),
                                   column(p[5].first).row(p[5].second)},
                                  { column(p[6].first).row(p[6].second),
                                   column(p[7].first).row(p[7].second),
                                   column(p[8].first).row(p[8].second)},
    })
        .det();
}

fp Matrix4::det() const
{
    const auto primary = row(0);
    const auto minor = [this](const int pos)
    {
        std::vector<Vec4f> c {};
        for (int x = 0; x < 4; ++x)
        {
            if (x == pos)
                continue;
            c.push_back(column(x));
        }
        return Matrix3::from_rows(
                   {
                       {c[0].row(1),  c[1].row(1), c[2].row(1)},
                       { c[0].row(2), c[1].row(2), c[2].row(2)},
                       { c[0].row(3), c[1].row(3), c[2].row(3)}
        })
            .det();
    };
    return primary[0] * minor(0) - primary[1] * minor(1) +
           primary[2] * minor(2) - primary[3] * minor(3);
}

Matrix4 Matrix4::scale(fp fp)
{
    return { c0.scale(fp), c1.scale(fp), c2.scale(fp), c3.scale(fp) };
}

Matrix4 Matrix4::invert() const
{
    return Matrix4::from_columns(
               {
                   {minor(0,  0), minor(0, 1), minor(0, 2), minor(0, 3)},
                   { minor(1, 0), minor(1, 1), minor(1, 2), minor(1, 3)},
                   { minor(2, 0), minor(2, 1), minor(2, 2), minor(2, 3)},
                   { minor(3, 0), minor(3, 1), minor(3, 2), minor(3, 3)}
    })
        .scale(1 / det());
}

/*****************************************************************************/
// Functions - Misc
/*****************************************************************************/

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

/*****************************************************************************/
// Functions - Transformations
/*****************************************************************************/

Matrix4 homotranslate(const Vec3f& disp)
{
    // disp stands for "displacement"
    return Matrix4::from_rows({
        {1,  0, 0, disp.x},
        { 0, 1, 0, disp.y},
        { 0, 0, 1, disp.z},
        { 0, 0, 0, 1     }
    });
}

// Axis direction is significant! Right-hand curl!
Matrix4 homorotate(const fp ccw_angle, const Ray& axis)
{
    // auto rotx = [](const fp x) constexpr
    // {
    //     const Matrix4 r = {
    //         { 1, 0,      0,       0 },
    //         { 0, cos(x), -sin(x), 0 },
    //         { 0, sin(x), cos(x),  0 },
    //         { 0, 0,      0,       1 },
    //     };
    //     return r.transpose();
    // };
    // auto roty = [](const fp x) constexpr
    // {
    //     const Matrix4 r = {
    //         { cos(x),  0, sin(x), 0 },
    //         { 0,       1, 0,      0 },
    //         { -sin(x), 0, cos(x), 0 },
    //         { 0,       0, 0,      1 },
    //     };
    //     return r.transpose();
    // };
    auto rotz = [](const fp x)
    {
        return Matrix4::from_rows({
            {cos(x),  -sin(x), 0, 0},
            { sin(x), cos(x),  0, 0},
            { 0,      0,       1, 0},
            { 0,      0,       0, 1},
        });
    };

    Vec3f u = normalize(axis.v);
    const Matrix4 step1_translate_away_from_origin =
        homotranslate(axis.o.scale(-1));
    const fp a = u.x, b = u.y, c = u.z;
    const fp d = sqrt(b * b + c * c);
    Matrix4 step2_axis_to_zx = Matrix4::i4();
    // handle cases where axis is already on zx-plane
    {
        if (d == 0)
        {
            step2_axis_to_zx = Matrix4::i4();
        }
        else
        {
            step2_axis_to_zx = Matrix4::from_rows({
                {1,  0,     0,      0},
                { 0, c / d, -b / d, 0},
                { 0, b / d, c / d,  0},
                { 0, 0,     0,      1}
            });
        }
    }
    const fp len = sqrt(a * a + b * b + c * c);
    const Matrix4 step3_axis_to_z = Matrix4::from_rows({
        {sqrt(b * b + c * c) / len, 0, -a / len,                  0},
        { 0,                        1, 0,                         0},
        { a / len,                  0, sqrt(b * b + c * c) / len, 0},
        { 0,                        0, 0,                         1}
    });
    const Matrix4 step4_desired_rotation = rotz(ccw_angle);
    const Matrix4 step5_undo_step3 = step3_axis_to_z.invert();
    const Matrix4 step6_undo_step2 = step2_axis_to_zx.invert();
    const Matrix4 step7_undo_step1 = step1_translate_away_from_origin.invert();
    dprint("step1", step1_translate_away_from_origin);
    dprint("step2", step2_axis_to_zx);
    dprint("step3", step3_axis_to_z);
    dprint("step4", step4_desired_rotation);
    dprint("step5", step5_undo_step3);
    dprint("step6", step6_undo_step2);
    dprint("step7", step7_undo_step1);
    return step7_undo_step1 * step6_undo_step2 * step5_undo_step3 *
           step4_desired_rotation * step3_axis_to_z * step2_axis_to_zx *
           step1_translate_away_from_origin;
}

};   // namespace m
