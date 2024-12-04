#include "World.hpp"

#include "Base.hpp"
#include "CENG477.hpp"
#include "Rasterizer.hpp"

#include <cstdlib>
#include <string>

ViewConfig::ViewConfig(const char* name, long rows, long columns,
                       const m::Vec3f& camera_origin,
                       const m::Vec3f& camera_gaze, const m::Vec3f& camera_up,
                       const ViewFrustum& f, const m::Pixel& background_color,
                       bool perspective_correct, bool cull_backface) :
    filename(name),
    bg_color(background_color),
    t_viewport(viewport_transformation(rows, columns)),
    t_projection(perspective_correct ? perspective_projection(f) :
                                       orthographic_projection(f)),
    t_camera(camera_transformation(camera_origin, camera_gaze, camera_up)),
    pixel_grid_rows(rows),
    pixel_grid_columns(columns),
    cull_backface(cull_backface)
{
}

m::Matrix4 viewport_transformation(long pixel_grid_rows,
                                   long pixel_grid_columns)
{
    return Matrix4::from_rows({
        {pixel_grid_rows / 2.0, 0,                        0, (pixel_grid_rows - 1) / 2.0   },
        { 0,                    pixel_grid_columns / 2.0, 0, (pixel_grid_columns - 1) / 2.0},
        { 0,                    0,                        1, 0                             },
        { 0,                    0,                        0, 1                             }
    });
}

m::Matrix4 camera_transformation(m::Vec3f cam_o, m::Vec3f gaze, m::Vec3f up)
{
    using namespace m;
    const Vec3f w = gaze.scale(-1);
    const Vec3f v = up;
    const Vec3f u = cross(v, w);
    return Matrix4::from_columns({ u.homovector(), v.homovector(),
                                   w.homovector(), cam_o.homopoint() })
        .invert();
}

m::Matrix4 orthographic_projection(ViewFrustum f)
{
    // do NOT trust the user in passing values correctly-signed
    using namespace m;
    using std::abs;
    const fp width = abs(f.left - f.right);
    const fp height = abs(f.top - f.bottom);
    const fp distance = abs(f.near - f.far);
    return Matrix4::from_rows({
        {2.0 / width, 0,            0,              abs(f.right + f.left) / width },
        { 0,          2.0 / height, 0,              abs(f.top + f.bottom) / height},
        { 0,          0,            2.0 / distance, abs(f.near + f.far) / distance},
        { 0,          0,            0,              1                             }
    });
}

m::Matrix4 perspective_projection(ViewFrustum vf)
{
    // do NOT trust the user in passing values correctly-signed
    using namespace m;
    using std::abs;
    const fp n = -abs(vf.near);
    const fp f = -abs(vf.far);
    const Matrix4 orthographic = orthographic_projection(vf);
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
    Vec3f cam_v { -1, -1, -1 };
    Vec3f cam_o { 1, 1, 1 };
    const Matrix4 rot = homorotate(r, {
                                          {0,  0, 0},
                                          { 0, 1, 0}
    });
    cam_o = (rot * cam_o.homopoint()).dehomogenize();
    cam_v = (rot * cam_v.homopoint()).dehomogenize();
    const ViewFrustum vf { 5, -5, 5, -5, 0.2, 10 };
    return ViewConfig("sample view", 800, 600, cam_o, cam_v, { 0, 1, 0 }, vf,
                      { 0, 0, 0 }, true, false);
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
