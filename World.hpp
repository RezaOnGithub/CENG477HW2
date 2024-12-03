#pragma once
#include "Base.hpp"
#include "CENG477.hpp"

#include <cstddef>
#include <string>
#include <vector>

// Cook everything into a list of faces with attributes
// Apply all modeling transformations, handle instancing transparently

// Attribute Type Qualifiers
// - uniform: constant accross the face
// - smooth: perspective-correct interpolation
// - noperspective: affine/linear interpolation
// Unimplemented: flat ("provoking" vertex), closest (would-be-nice)

struct Vertex;
struct VertexAttributeInterp;
struct Face;
struct FaceAttributeUniform;
struct World;
struct ViewConfig;
std::vector<ViewConfig> extract_views(const ceng::Scene& s);

struct VertexAttributeInterp
{
    m::Pixel noperspective_ceng477_color;
    m::Vec3f noperspective_vertex_normal;
};

struct FaceAttributeUniform
{
    m::Vec3f uniform_surface_normal;
};

struct Vertex
{
    m::Vec3f coord;

    // TODO for when rasterizer is ready
    // VertexAttributeInterp interp;

    [[nodiscard]] inline m::fp operator[](size_t i) const
    {
        return coord[i];
    }

    [[nodiscard]] inline m::fp row(size_t i) const
    {
        return coord.row(i);
    }
};

struct Face
{
    Vertex v0, v1, v2;
    enum class Mode
    {
        WIREFRAME,
        SOLID
    } mode;
    FaceAttributeUniform uniform;

    inline Vertex operator[](size_t i) const
    {
        switch (i)
        {
        case 0 :
            return v0;
        case 1 :
            return v1;
        case 2 :
            return v2;
        default :
            throw std::string("Invalid Index!");
        }
    }
};

struct ViewConfig
{
    std::string filename;
    m::Matrix4 t_camera;
    m::Matrix4 t_projection;
    long pixel_grid_rows;
    long pixel_grid_columns;
};

static const ViewConfig sample_view { "sample_view.ppm",
                                      m::Matrix4::from_rows({
                                        m::Vec3f({}).homovector(),

                                      }).invert(),
                                      m::Matrix4::from_rows({}), 800, 600 };

struct World
{
public:
    // TODO implement
    // World(ceng::Scene s);
    inline World(m::Vec3f v0, m::Vec3f v1, m::Vec3f v2) :
        bg_color({ 125, 125, 125 })
    {
        fs.push_back({
            {
             v0, },
            {
             v1, },
            { v2 },
            Face::Mode::WIREFRAME,
            { m::surface_normal(v0, v1, v2) }
        });
    }

    [[nodiscard]] inline Face get_face(size_t i) const
    {
        return fs[0];
    }

    size_t face_count(size_t i);
    m::Pixel get_bg_color();
    [[nodiscard]] m::Vec4f canonical_to_camera(m::Vec4f i) const;
    [[nodiscard]] m::Matrix4 t_camera() const;
    [[nodiscard]] m::Matrix4 t_proj() const;
private:
    m::Pixel bg_color;
    std::vector<Face> fs;
};
