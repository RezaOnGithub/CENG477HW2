#include "Base.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
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
        throw std::runtime_error("Invalid index");
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
    return { w * x, w * y, w * z, w };
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
        throw std::runtime_error("Invalid index");
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
        throw std::runtime_error("Invalid index");
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
        throw std::runtime_error("Invalid index");
    }
}

fp Matrix3::det() const
{
    const auto r = row(0);
    // dprint("the chosen row", r);
    // dprint("minor #1 ", minor({ 0, 0 }));
    // dprint("minor #2 ", minor({ 0, 1 }));
    // dprint("minor #3 ", minor({ 0, 2 }));

    return r[0] * minor({ 0, 0 }) - r[1] * minor({ 0, 1 }) +
           r[2] * minor({ 0, 2 });
}

fp Matrix3::minor(IndexPair x) const
{
    std::vector<IndexPair> p {};
    for (size_t i = 0; i < 3; i++)
    {
        if (not(i == x.row))
        {
            for (size_t j = 0; j < 3; j++)
            {
                if (not(j == x.column))
                {
                    // printf("\t\t%lu %lu\n", i, j);
                    p.emplace_back(IndexPair { i, j });
                }
            }
        }
    }
    const auto reduced = Matrix2::from_rows({
        {rc(p[0]),  rc(p[1])},
        { rc(p[2]), rc(p[3])}
    });
    // dprint("reduced matrix for the current minor ", reduced);
    return reduced.det();
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
        throw std::runtime_error("Invalid Index!");
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
    });
}

Vec4f Matrix4::operator*(const Vec4f& rhs) const
{
    // because operator[] does not return a reference, can't write it in
    // loop form
    return {
        dot(row(0), rhs),
        dot(row(1), rhs),
        dot(row(2), rhs),
        dot(row(3), rhs),
    };
}

fp Matrix4::minor(IndexPair x) const
{
    std::vector<IndexPair> p {};
    for (size_t i = 0; i < 4; ++i)
    {
        if (i == x.row)
        {
            continue;
        }
        for (size_t j = 0; j < 4; j++)
        {
            if (j == x.column)
            {
                continue;
            }
            const IndexPair pair { i, j };
            p.emplace_back(pair);
        }
    }

    return Matrix3::from_rows({
                                  {rc(p[0]),  rc(p[1]), rc(p[2])},
                                  { rc(p[3]), rc(p[4]), rc(p[5])},
                                  { rc(p[6]), rc(p[7]), rc(p[8])},
    })
        .det();
}

fp Matrix4::det() const
{
    const auto primary = row(0);
    return primary[0] * minor({ 0, 0 }) - primary[1] * minor({ 0, 1 }) +
           primary[2] * minor({ 0, 2 }) - primary[3] * minor({ 0, 3 });
}

Matrix4 Matrix4::scale(fp fp) const
{
    return { c0.scale(fp), c1.scale(fp), c2.scale(fp), c3.scale(fp) };
}

Matrix4 Matrix4::invert() const
{
    std::vector<Vec4f> rows_cofactors {};
    for (size_t i = 0; i < 4; i++)
    {
        const fp s = (i % 2) != 0u ? -1 : 1;
        const Vec4f sign = { s, -s, s, -s };
        const Vec4f x { minor({ i, 0 }), minor({ i, 1 }), minor({ i, 2 }),
                        minor({ i, 3 }) };
        rows_cofactors.push_back(pointwise(x, sign));
    }
    // dprint("matrix of cofactors ", Matrix4::from_rows(rows_cofactors));
    // dprint("matrix determinant is ", det());
    return Matrix4::from_rows(rows_cofactors).transpose().scale(1 / det());
}

/*****************************************************************************/
// Functions - Misc
/*****************************************************************************/

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

Vec3f barycentric(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2,
                  const Vec2f& r)
{
    // const Matrix2 solve = Matrix2::from_rows({
    //                                              {b.x - a.x,  c.x - a.x},
    //                                              { b.y - a.y, c.y - a.y}
    // })
    //                           .invert();

    // const auto [beta, gamma] = solve * Vec2f({p.x-a.x, p.y-a.y});
    // return { 1 - beta - gamma, beta, gamma };
    const Matrix2 solve = Matrix2::from_rows({
                                                 {v0.x - v2.x,  v1.x - v2.x},
                                                 { v0.y - v2.y, v1.y - v2.y}
    })
                              .invert();

    const auto [alpha, beta] = solve * Vec2f({ r.x - v2.x, r.y - v2.y });
    return { alpha, beta, 1 - alpha - beta };
}

// TODO I have tried my best not to dehomogenize and hope for correctness
// TODO FIXME after making sure it's correct, eliminate dehomogenization!
Clip clip_aa_inner(const Vec3f& normal, const HomoLine& l, fp epsilon)
{
    const Vec4f start = l.cohomogenize().start;
    const Vec4f end = l.cohomogenize().end;
    const fp w = start.w;
    const Vec3f direction =
        Vec4f(end.x - start.x, end.y - start.y, end.z - start.z, w)
            .dehomogenize();
    Vec3f point_on_plane = normal.scale(-1);

    auto line_eq = [&normal, &point_on_plane](Vec3f p)
    {
        return dot(normal, { p.x - point_on_plane.x, p.y - point_on_plane.y,
                             p.z - point_on_plane.z });
    };

    const int limiting_coordinate_index = normal[0] ? 0 : (normal[1] ? 1 : 2);
    std::vector<int> free_coordinate_indices;
    for (int i = 0; i < 3; i++)
    {
        if (i == limiting_coordinate_index)
        {
            continue;
        }
        free_coordinate_indices.push_back(i);
    }

    // const fp cc_bound = -normal[constant_coordinate_index];
    // // reminder that these are cohomo
    // const fp start_vc[2] { start[variable_coordinate_indices[0]],
    //                        start[variable_coordinate_indices[1]] };
    // const fp start_cc = start[constant_coordinate_index];

    // const fp end_vc[2] { end[variable_coordinate_indices[0]],
    //                      end[variable_coordinate_indices[1]] };
    const fp end_cc = end[limiting_coordinate_index];

    const fp start_subs = line_eq(start.dehomogenize());
    const fp end_subs = line_eq(end.dehomogenize());

    // Epsilon Policy: Be Liberal in What You Accept

    // Trivial Success: both points are on the "positive" side of the plane
    if (start_subs > -ceng_epsilon and end_subs > -ceng_epsilon)
    {
        // puts("Trivial Success!");
        return { l, Clip::ClipType::NoCut };
    }

    // Trivial Failure: both points on the "negative" side
    if (start_subs < -ceng_epsilon and end_subs < -ceng_epsilon)
    {
        puts("Trivial Failure!");
        return { {}, Clip::ClipType::NonExistant };
    }

    // It is guarenteed that if line is intersecting the plane it is not
    // parallel to it.

    // Line is intersecting
    Vec3f point_on_line = start.dehomogenize();
    Vec3f point_on_plane_minus_point_on_line {
        point_on_plane.x - point_on_line.x, point_on_plane.y - point_on_line.y,
        point_on_plane.z - point_on_line.z
    };
    fp point_on_plane_minus_point_on_line_dot_normal =
        dot(point_on_plane_minus_point_on_line, normal);
    fp direction_dot_normal = dot(direction, normal);
    fp d = point_on_plane_minus_point_on_line_dot_normal / direction_dot_normal;
    Vec3f plane_intersection { point_on_line.x + d * direction.x,
                               point_on_line.y + d * direction.y,
                               point_on_line.z + d * direction.z };
    if (within(plane_intersection[free_coordinate_indices[0]], -1, 1) and
        within(plane_intersection[free_coordinate_indices[1]], -1, 1))
    {
        // puts("Found an intersection on AA Rectangle!");
        // dprint("intersection", plane_intersection);
        if (d > 0)
        {
            // starting point is inside, ending is outside
            return {
                {l.start, plane_intersection.homopoint()},
                Clip::ClipType::CutTail
            };
        }
        return {
            {plane_intersection.homopoint(), l.end},
            Clip::ClipType::CutTail
        };
    }

    // TODO delete this once things seem OK
    throw std::runtime_error(
        "Bug in clipping: line was supposed to intersect!");
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
    const fp a = u.x;
    const fp b = u.y;
    const fp c = u.z;
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
    // dprint("step1", step1_translate_away_from_origin);
    // dprint("step2", step2_axis_to_zx);
    // dprint("step3", step3_axis_to_z);
    // dprint("step4", step4_desired_rotation);
    // dprint("step5", step5_undo_step3);
    // dprint("step6", step6_undo_step2);
    // dprint("step7", step7_undo_step1);
    return step7_undo_step1 * step6_undo_step2 * step5_undo_step3 *
           step4_desired_rotation * step3_axis_to_z * step2_axis_to_zx *
           step1_translate_away_from_origin;
}

Matrix4 homoscale(fp sx, fp sy, fp sz)
{
    return Matrix4::from_rows({
        {sx, 0,  0,  0},
        { 0, sy, 0,  0},
        { 0, 0,  sz, 0},
        { 0, 0,  0,  1}
    });
}
};   // namespace m
