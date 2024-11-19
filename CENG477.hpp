#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

constexpr double EPSILON = 0.000000001;

struct Vec3
{
    double x, y, z;
    int colorId;
};

constexpr int NO_COLOR = -1;

class Vec4
{
public:
    double x, y, z, t;
    int colorId;

    // Vec4();
    // Vec4(double x, double y, double z, double t);
    // Vec4(double x, double y, double z, double t, int colorId);
    // Vec4(const Vec4 &other);

    constexpr double getNthComponent(int n)
    {
        switch (n)
        {
        case 0 :
            return this->x;

        case 1 :
            return this->y;

        case 2 :
            return this->z;

        case 3 :
        default :
            return this->t;
        }
    }
};

struct Triangle
{
    int v1, v2, v3;
};

struct Camera
{
    int cameraId, projectionType, horRes,
        verRes;   // projectionType=0 for orthographic
                  // projectionType=1 for perspective
    Vec3 position, gaze, u, v, w;
    double left, right, bottom, top, near, far;
    std::string outputFilename;
};

constexpr int ORTOGRAPHIC_PROJECTION = 0;
constexpr int PERSPECTIVE_PROJECTION = 1;

struct Color
{
    double r, g, b;
};

struct Matrix4
{
    double values[4][4];
};

struct Mesh
{
    int meshId, type, numberOfTransformations,
        numberOfTriangles;   // type=0 for wireframe, type=1 for solid
    std::vector<int> transformationIds;
    std::vector<char> transformationTypes;
    std::vector<Triangle>
        triangles;   // No Please! pass triangles with a pointer too!
};

constexpr int WIREFRAME_MESH = 0;
constexpr int SOLID_MESH = 1;

struct Rotation
{
    int rotationId;
    double angle, ux, uy, uz;
};

struct Translation
{
    int translationId;
    double tx, ty, tz;
};

struct Scaling
{
    int scalingId;
    double sx, sy, sz;
};

class Scene
{
public:
	Color backgroundColor;
	bool cullingEnabled;

	std::vector<std::vector<Color> > image;
	std::vector<Camera *> cameras;
	std::vector<Vec3 *> vertices;
	std::vector<Color *> colorsOfVertices;
	std::vector<Scaling *> scalings;
	std::vector<Rotation *> rotations;
	std::vector<Translation *> translations;
	std::vector<Mesh *> meshes;

	Scene(const char *xmlPath);

	void initializeImage(Camera *camera);
    void forwardRenderingPipeline(Camera *camera);
	int makeBetweenZeroAnd255(double value);
	void writeImageToPPMFile(Camera *camera);
};
