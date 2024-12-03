#pragma once
#include "Base.hpp"
#include "World.hpp"

#include <cstddef>
#include <utility>
#include <vector>
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
// Dots use VP-Coordinates, NOT pixel coordinates!
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

struct FragmentAttribute;
struct Fragment;
struct RasterTriangle;

using ScreenCoordinate = std::pair<long,long>;

struct FragmentAttribute
{
    m::Pixel noperspective_ceng477_color;
    m::fp depth;
};

struct Fragment
{
    Vec4f ndc;
    Vec3f vp;
    Vec2f scr;
    FragmentAttribute attrib;
};

struct RasterTriangle
{
    Face mother;
    std::vector<Fragment> out;
};

RasterTriangle rasterize(const World& w, const ViewConfig& v, size_t face_index);
