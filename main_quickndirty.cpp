#include <cstdio>

#include "Renderer.hpp"
#include "World.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ViewConfig simple_view(m::fp r)
{
    using namespace m;
    Vec3f cam_v{ -10, -10, 0 };
    Vec3f cam_o{ 10, 10, 0 };
    const Matrix4 rot = homorotate(r, {
                                       { 0, 0, 0 },
                                       { 0, 1, 0 }
                                   });
    cam_o = (rot * cam_o.homopoint()).dehomogenize();
    cam_v = (rot * cam_v.homopoint()).dehomogenize();
    const ViewFrustum vf{ 100, -100, 100, -100, 0.001, 100 };
    return ViewConfig("simple_view", 800, 600, cam_o, cam_v, { 0, 1, 0 }, vf,
                      { 0, 0, 255 }, false, false);
}

int main()
{
    const auto& w = sample_world();
    const auto& v = simple_view(0);
    const auto& img = render(w, v);
    unsigned char* data = new unsigned char [
        v.pixel_grid_rows * v.pixel_grid_columns * 3];
    for (size_t i = 0; i < v.pixel_grid_rows; i++)
    {
        for (size_t j = 0; j < v.pixel_grid_columns; j++)
        {
            const size_t base_index = (i * v.pixel_grid_columns + j) * 3;
            data[base_index + 0] = img[i][j].r;
            data[base_index + 1] = img[i][j].g;
            data[base_index + 2] = img[i][j].b;
            // printf("index written was %lu\n", base_index);
        }
    }
    stbi_write_png((v.filename + ".png").c_str(), v.pixel_grid_rows,
                   v.pixel_grid_columns,
                   3, data, v.pixel_grid_columns * 3);
    return 0;
}
