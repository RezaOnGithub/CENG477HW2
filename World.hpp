#include <vector>
#include "Base.hpp"

// Cook everything into a list of faces with attributes

using namespace m;

// Attribute Type Qualifiers
// - uniform: constant accross the face
// - smooth: perspective-correct interpolation
// - noperspective: affine/linear interpolation
// Unimplemented: flat ("provoking" vertex), closest (would-be-nice)

struct Vertex;
struct VertexAttributeInterp;
struct Face;
struct FaceAttributeUniform;
struct CombinedAttribute;
struct World;

struct VertexAttributeInterp
{
    Pixel noperspective_ceng477_color;
    Vec3f noperspective_normal;
};

struct FaceAttributeUniform
{
    Vec3f uniform_surfacenorma;
};

struct Vertex
{
    Vec3f coord;
    VertexAttributeInterp interp;
    const FaceAttributeUniform uniform;
    inline fp operator[](size_t i) const
    {
        return coord[i];
    }
};

struct Face
{
    Vertex v0,v1,v2;
    FaceAttributeUniform uniform;
};

struct World;
