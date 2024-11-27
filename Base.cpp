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
    const Matrix4 t = {
        {}
    };
    return t.transpose();
}

Matrix4 homorotate(double ccw_angle, const Ray& axis)
{
    auto u = normalize(axis.v);
    Matrix4 translate_away_from_origin = homotranslate(axis.o.scale(-1));
    Matrix4 translate_back_to_origin = homotranslate(axis.o);
    
}

}   // namespace m
