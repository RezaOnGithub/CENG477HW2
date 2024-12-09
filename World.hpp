#pragma once
#include "Base.hpp"
#include "CENG477.hpp"

#include <cstddef>
#include <string>
#include <vector>

// Cook everything into a list of faces with attributes
// Apply all modeling transformations, handle instancing transparently

struct WorldVertex;
struct WorldFace;
// complemeted by FragmentAttribteFinal in rasterizer and renderer
struct VertexAttributeInterp;
struct FaceAttributeUniform;
struct World;
struct ViewConfig;

struct ViewFrustum
{
    m::fp left, right, top, bottom, near, far;
};

m::Matrix4 viewport_transformation(long pixel_grid_rows,
                                   long pixel_grid_columns);
m::Matrix4 orthographic_projection(ViewFrustum f);
m::Matrix4 perspective_projection(const ViewFrustum& vf);
m::Matrix4 camera_transformation(m::Vec3f cam_o, m::Vec3f gaze, m::Vec3f up);

class ViewConfig
{
public:
    ViewConfig() = delete;
    ViewConfig(const char* filename, long rows, long columns,
               const m::Vec3f& camera_origin, const m::Vec3f& camera_gaze,
               const m::Vec3f& camera_up, const ViewFrustum& frustum,
               const m::Pixel& background_color, bool perspective_correct,
               bool cull_backface);
    std::string filename;
    m::Pixel bg_color;
    m::Vec3f gaze;
    m::Matrix4 t_viewport;
    m::Matrix4 t_projection;
    m::Matrix4 t_camera;
    long pixel_grid_rows;
    long pixel_grid_columns;
    bool cull_backface;
};

// complemented by World(const ceng::Scene &s)
std::vector<ViewConfig> extract_views(const ceng::Scene& s);

// sample view looking at (0,0,0) from (1,1,1); optionally spin around y
ViewConfig sample_view(m::fp rot = 0);
// sample world with a single triangle at (1,0,0) (0,1,0) (0,0,1)
World sample_world();

struct WorldVertex
{
    m::Vec3f wc;
    m::Pixel ceng477_color;
};

struct WorldFace
{
    WorldVertex v0, v1, v2;
    enum class RenderMode
    {
        WIREFRAME,
        SOLID,
        // ALPHA
    } mode;
};

struct World
{
private:
    std::vector<WorldFace> fs;
public:
    World(const ceng::Scene& s);
    World(m::Vec3f v0, m::Vec3f v1, m::Vec3f v2);

    [[nodiscard]] inline WorldFace get_face(size_t i) const
    {
        return fs[i];
    }

    [[nodiscard]] inline size_t face_count() const
    {
        return fs.size();
    }
};
