#include "Base.hpp"

#include <cmath>

namespace m
{

Vec3f normalize(const Vec3f& a)   // TODO: maybe pass by value for performance?
{
    fp len = sqrt(dot(a, a));
    return { a.x / len, a.y / len, a.z / len };
}

Matrix4 homotranslate(const Vec3f& disp)
{
    // disp stands for "displacement"
    constexpr Matrix4 t = {
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
    auto u = normalize(axis.v);
    const Matrix4 step1_translate_away_from_origin = homotranslate(axis.o.scale(-1));
    const Matrix4 step2_
    const Matrix4 translate_back_to_origin = homotranslate(axis.o);
}

}   // namespace m
