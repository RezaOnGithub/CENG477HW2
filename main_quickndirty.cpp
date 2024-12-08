#include "Renderer.hpp"
#include "World.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main()
{
    const auto& w = sample_world();
    const auto& v = sample_view();
    const auto& img = render(w, v);
    unsigned char** data;
    data = (unsigned char**)malloc(v.pixel_grid_rows * sizeof(*data));
    for (size_t i = 0; i < v.pixel_grid_rows; i++)
    {
        data[i] =
            (unsigned char*)malloc(3 * v.pixel_grid_columns * sizeof(**data));
        for (size_t j = 0; j < v.pixel_grid_columns; j++)
        {
            data[i][j + 0] = img[i][j].r;
            data[i][j + 1] = img[i][j].g;
            data[i][j + 2] = img[i][j].b;
        }
    }
    stbi_write_png(v.filename.c_str(), v.pixel_grid_rows, v.pixel_grid_columns,
                   3, data, v.pixel_grid_rows * 3);
    return 0;
}
