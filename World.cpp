#include "World.hpp"

#include "Base.hpp"
#include "CENG477.hpp"

#include <algorithm>
#include <cstdlib>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

// TODO the initializer list does not feel good. Matrix4 does not have a public
// default constructor yet it works for some reason.
ViewConfig::ViewConfig(const char* name, long rows, long columns,
                       const m::Vec3f& camera_origin,
                       const m::Vec3f& camera_gaze, const m::Vec3f& camera_up,
                       const ViewFrustum& f, const m::Pixel& background_color,
                       bool perspective_correct, bool cull_backface) :
    filename(name),
    bg_color(background_color),
    gaze(camera_gaze),
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
    return m::Matrix4::from_rows({
        {pixel_grid_columns / 2.0, 0,                     0, (pixel_grid_columns - 1) / 2.0},
        { 0,                       pixel_grid_rows / 2.0, 0, (pixel_grid_rows - 1) / 2.0   },
        { 0,                       0,                     1, 0                             },
        { 0,                       0,                     0, 1                             }
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

    // const fp near = -1 * f.near;
    // const fp far = -1 * f.far;
    // return Matrix4::from_rows({
    //     { 2.0 / (f.right - f.left), 0, 0,
    //       -1 * (f.right + f.left) / (f.right - f.left) },
    //     { 0, 2.0 / (f.top - f.bottom), 0,
    //       -1 * (f.top + f.bottom) / (f.top - f.bottom) },
    //     { 0, 0, 2.0 / (f.near - f.far),
    //       -1 * (near + far) / (near - far) },
    //     { 0, 0, 0, 1 }
    // });
}

m::Matrix4 perspective_projection(const ViewFrustum& vf)
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

std::vector<ViewConfig> extract_views(const ceng::Scene& s)
{
    // clamp and cast
    auto color = [&](const ceng::Color& c) -> m::Pixel
    {
        return { static_cast<unsigned char>(c.r > 255 ? 255 : c.r),
                 static_cast<unsigned char>(c.g > 255 ? 255 : c.g),
                 static_cast<unsigned char>(c.b > 255 ? 255 : c.b) };
    };
    std::vector<ViewConfig> result;
    result.reserve(s.cameras.size());
    for (auto* c : s.cameras)
    {
        result.emplace_back(ViewConfig {
            c->outputFilename.c_str(),
            c->verRes,
            c->horRes,
            { c->position.x, c->position.y, c->position.z },
            { c->gaze.x, c->gaze.y, c->gaze.z },
            { c->v.x, c->v.y, c->v.z },
            { c->left, c->right, c->top, c->bottom, c->near, c->far },
            color(s.backgroundColor),
            c->projectionType != 0,
            s.cullingEnabled
        });
    }
    return result;
}

ViewConfig sample_view(m::fp r)
{
    using namespace m;
    Vec3f cam_v { -10, -10, 0 };
    Vec3f cam_o { 10, 10, 0 };
    const Matrix4 rot = homorotate(r, {
                                          {0,  0, 0},
                                          { 0, 1, 0}
    });
    cam_o = (rot * cam_o.homopoint()).dehomogenize();
    cam_v = (rot * cam_v.homopoint()).dehomogenize();
    const ViewFrustum vf { 5, -5, 5, -5, 0.2, 10 };
    return ViewConfig("sample_view.ppm", 800, 600, cam_o, cam_v, { 0, 1, 0 },
                      vf, { 0, 0, 0 }, true, false);
}

World sample_world()
{
    return World({ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 });
}

World::World(m::Vec3f v0, m::Vec3f v1, m::Vec3f v2)
{
    m::Pixel r { 255, 0, 0 };
    fs.push_back({
        {v0,  { r }},
        { v1, { r }},
        { v2, { r }},
        WorldFace::RenderMode::WIREFRAME,
    });
}

// TODO someone review this!
World::World(const ceng::Scene& scene)
{
    // strategy: accumulate into WorldFace collections
    // then accumulate transformations
    // then transform and push onto the flat list of triangles

    // TODO the PDF says there is instancing, but does not explain it
    // and there is no data structure to support it- right?
    // None of the given examples show instancing

    // clamp and cast
    auto color = [&](const ceng::Color& c) -> m::Pixel
    {
        return { static_cast<unsigned char>(c.r > 255 ? 255 : c.r),
                 static_cast<unsigned char>(c.g > 255 ? 255 : c.g),
                 static_cast<unsigned char>(c.b > 255 ? 255 : c.b) };
    };
    auto vec3 = [&](size_t id) -> m::Vec3f
    {
        return { scene.vertices[id - 1]->x, scene.vertices[id - 1]->y,
                 scene.vertices[id - 1]->z };
    };
    auto vec3_color = [&](size_t id) -> m::Pixel
    {
        return color(
            *(scene.colorsOfVertices[scene.vertices[id - 1]->colorId - 1]));
    };

    std::vector<WorldFace> scene_faces {};
    for (auto* m : scene.meshes)
    {
        for (size_t trigi = 0; trigi < m->numberOfTriangles; trigi++)
        {
            m::Vec4f v0h = vec3(m->triangles[trigi].v1).homopoint();
            m::Vec4f v1h = vec3(m->triangles[trigi].v2).homopoint();
            m::Vec4f v2h = vec3(m->triangles[trigi].v3).homopoint();
            m::Pixel c0 = vec3_color(m->triangles[trigi].v1);
            m::Pixel c1 = vec3_color(m->triangles[trigi].v2);
            m::Pixel c2 = vec3_color(m->triangles[trigi].v3);

            // Apply every transformation defined for this mesh on a single
            // triangle
            for (size_t transi = 0; transi < m->numberOfTransformations;
                 transi++)
            {
                const size_t trans_id = m->transformationIds[transi];
                const char trans_type = m->transformationTypes[transi];
                switch (trans_type)
                {
                case 't' : {
                    const auto t = std::find_if(
                        scene.translations.begin(), scene.translations.end(),
                        [trans_id](ceng::Translation* e)
                        {
                            return e->translationId == trans_id;
                        });
                    if (t == scene.translations.end())
                    {
                        throw std::runtime_error(
                            "Couldn't find the translation by id");
                    }
                    const auto t_deref = **t;
                    const auto tranformation = m::homotranslate(
                        { t_deref.tx, t_deref.ty, t_deref.tz });
                    v0h = tranformation * v0h;
                    v1h = tranformation * v1h;
                    v2h = tranformation * v2h;
                    break;
                }
                case 's' : {
                    const auto s = std::find_if(
                        scene.scalings.begin(), scene.scalings.end(),
                        [trans_id](ceng::Scaling* e)
                        {
                            return e->scalingId == trans_id;
                        });
                    if (s == scene.scalings.end())
                    {
                        throw std::runtime_error(
                            "Couldn't find the scaling by id");
                    }
                    const auto s_deref = **s;
                    const auto tranformation =
                        m::homoscale(s_deref.sx, s_deref.sy, s_deref.sz);
                    v0h = tranformation * v0h;
                    v1h = tranformation * v1h;
                    v2h = tranformation * v2h;
                    break;
                }
                case 'r' : {
                    const auto r = std::find_if(
                        scene.rotations.begin(), scene.rotations.end(),
                        [trans_id](ceng::Rotation* e)
                        {
                            return e->rotationId == trans_id;
                        });
                    if (r == scene.rotations.end())
                    {
                        throw std::runtime_error(
                            "Couldn't find the rotation by id");
                    }
                    const auto r_deref = **r;
                    const auto tranformation = m::homorotate(
                        (r_deref.angle * std::numbers::pi) / 180.0,
                        {
                            {0,           0,          0         },
                            { r_deref.ux, r_deref.uy, r_deref.uz}
                    });
                    v0h = tranformation * v0h;
                    v1h = tranformation * v1h;
                    v2h = tranformation * v2h;
                    break;
                }
                default :
                    // TODO should we crash?
                    throw std::runtime_error("Unknown transformation type!");
                }
            }

            WorldFace::RenderMode mode = m->type == ceng::WIREFRAME_MESH ?
                                             WorldFace::RenderMode::WIREFRAME :
                                             WorldFace::RenderMode::SOLID;
            scene_faces.push_back({
                {v0h.dehomogenize(),  c0},
                { v1h.dehomogenize(), c1},
                { v2h.dehomogenize(), c2},
                mode
            });
        }
    }
    this->fs = scene_faces;
}
