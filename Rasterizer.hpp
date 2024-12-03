#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cstddef>
using namespace m;

// Exhaustive Specification for Rasterizer
// =======================================
//
// Stage 0: World (input)
// ----------------------
//
// Every object is composed of faces. Every face has its own attributes
// Every face contains vertices.
// Every vertex has its own attributes.
// Everything is fed to Rasterizer in its canonical (worldspace) form
//
// Stage 1: Camera
// ---------------
//
// Calculate camera coordinates of each vertex and store it alongside canonical
// coordinates. Shouldn't be too hard.
//
// Take the time to find Surface Normals. Then, perform BFC.
//
// Stage 2: Device
// ---------------
//
// Calculate bounds of the viewing volume and cull objects that are completely
// outside it. Then, calculate NDC for each vertex and store it alongside the
// others.
//
// Stage 3: Generate Dots and Outlines
// ---------------------------------
//
// Dots are Vec3f on the screen plane. Not fragments yet.
// To generate Fragments, we need to play "connect-the-dots" to create outlines
// of each of the original shapes. Outlines may be clipped using a Polygon
// clipping algorithm.
//
// Every Dot knows the vertex it came from. Every outline knows the face it came
// from.
//
// Stage 4: Generate Fragments
// ---------------------------
//
// Now that you have "connected the dots", you need to fill in the shapes.
// First, draw using midpoint algorithm. Then, if it is required, paint inside
// the outlines using triangles rasterization algorithms PLUS AN EXTRA CHECK to
// see whether you are in-bounds of the canvas. This will also guarentee it is
// within the outline.
//
// Fair bit of interpolation needed. At minimum:
//      - noperspective_ceng477_color
//      - Z values
//
// Every Fragment knows the Outline, Face it came from.
//
// At this stage, every output fragment can be directly written to the output if
// we are going from front to back. If not, generate everything and then perform
// min_Z_search for each fragment and write /that/ result.
//
// Stage 5: Painter's Algorithm
// ----------------------------
//
// Either use painter's algorithm or the Z value carried from
// Vertex->NDC->Dot.z->Frag.z to display the correct fragment



struct Vertex
{
    Vec3f coord;
    VertexAttribute payload;

    constexpr double operator[](size_t i) const
    {
        return coord[i];
    }
};

struct FragmentAttribute
{
    fp depth;
    VertexAttribute interp;
};

// Fragments hold a payload that has data from "birthing" vertices, alongwith
// depth data. Vertex data that is computed and passed on to Fragment comes in
// multiple flavors. See VertexAttribute
// TODO making payload const ... somehow ... to avoid ever editing it
struct Fragment
{
    Vec3f clipspace;
    FragmentAttribute payload;

    double operator[](const size_t i) const;
};

class Rasterizer
{
public:
    Rasterizer() = delete;
};
