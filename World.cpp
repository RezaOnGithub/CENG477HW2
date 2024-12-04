#include "World.hpp"

#include "Base.hpp"
#include "CENG477.hpp"

#include <cstdlib>

static m::Matrix4 camera_transformation(m::Vec3f cam_o, m::Vec3f gaze,
                                        m::Vec3f up)
{
    using namespace m;
    const Vec3f w = gaze.scale(-1);
    const Vec3f v = up;
    const Vec3f u = cross(v, w);
    return Matrix4::from_columns({ u.homovector(), v.homovector(),
                                   w.homovector(), cam_o.homopoint() })
        .invert();
}

struct ViewFrustum
{
    m::fp left, right, top, bottom, near, far;
};

static m::Matrix4 orthographic_projection(ViewFrustum f)
{
    // do NOT trust the user in passing values correctly-signed
    using namespace m;
    using std::abs;
    const fp width = abs(abs(f.left) - abs(f.right));
    const fp height = abs(abs(f.top) - abs(f.bottom));
    const fp distance = abs(abs(f.near) - abs(f.far));
    return Matrix4::from_rows({
        {2.0 / width, 0,            0,              abs(f.right + f.left) / width },
        { 0,          2.0 / height, 0,              abs(f.top + f.bottom) / height},
        { 0,          0,            2.0 / distance, abs(f.near + f.far) / distance},
        { 0,          0,            0,              1                             }
    });
}

static m::Matrix4 perspective_projection(ViewFrustum fr)
{
    // do NOT trust the user in passing values correctly-signed
    using namespace m;
    using std::abs;
    const fp n = -abs(fr.near);
    const fp f = -abs(fr.far);
    const Matrix4 orthographic = orthographic_projection(fr);
    const Matrix4 perspective = Matrix4::from_rows({
        {n,  0, 0,     0     },
        { 0, n, 0,     0     },
        { 0, 0, n + f, -f * n},
        { 0, 0, 1,     0     }
    });
    return orthographic * perspective;
}

ViewConfig sample_view(m::fp r)
{
    using namespace m;
    const Vec3f cam_v { -1, -1, -1 };
    const Vec3f cam_o { 1, 1, 1 };
    const Matrix4 rot = homorotate(r, {
                                          {0,  0, 0},
                                          { 0, 1, 0}
    });
    const Matrix4 tcam = camera_transformation(
        (rot * (cam_o.homopoint())).dehomogenize(),
        (rot * (cam_v.homovector())).dehomogenize(), { 0, 1, 0 });
    return {
        "sample_view", { 0, 0, 0 },
        tcam,          perspective_projection({ 5, -5, 5, -5, 0.2, 10 }
         ),
        800,           600,
        false,
    };
}

World sample_world()
{
    return World({ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 });
}

World::World(m::Vec3f v0, m::Vec3f v1, m::Vec3f v2)
{
    m::Pixel r { 255, 0, 0 };
    fs.push_back({
        { v0, { r } },
        { v1, { r } },
        { v2, { r } },
        Face::RenderMode::WIREFRAME,
        { m::surface_normal(v0, v1, v2) }
    });
}

// TODO Implement
World::World(const ceng::Scene& s)
{
}
