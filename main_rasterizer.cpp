#include "CENG477.hpp"
#include "Renderer.hpp"
#include "World.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

void write_image(const char* filename, unsigned char* data, int width,
                 int height);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Please run the rasterizer as:" << std::endl
                  << "\t" << argv[0] << " <input_file_name>" << std::endl;
        return 1;
    }

    const char* xmlPath = argv[1];
    const ceng::Scene& scene = ceng::Scene(xmlPath);
    const World& w = World(scene);
    const std::vector<ViewConfig>& views = extract_views(scene);

    for (auto v : views)
    {
        auto img = render(w, v);
        auto* data =
            new unsigned char[v.pixel_grid_rows * v.pixel_grid_columns * 3];
        for (size_t i = 0; i < v.pixel_grid_rows; i++)
        {
            for (size_t j = 0; j < v.pixel_grid_columns; j++)
            {
                const size_t base_index = (i * v.pixel_grid_columns + j) * 3;
                data[base_index + 0] = img[i][j].r;
                data[base_index + 1] = img[i][j].g;
                data[base_index + 2] = img[i][j].b;
            }
        }
        std::string name = v.filename;
        write_image(name.c_str(), data, v.pixel_grid_columns,
                    v.pixel_grid_rows);
    }

    return 0;
}

// NOLINTBEGIN
//  Code copied straight from raytracer
void write_image(const char* filename, unsigned char* data, int width,
                 int height)
{
    FILE* outfile;

    if ((outfile = fopen(filename, "w")) == NULL)
    {
        throw std::runtime_error(
            "Error: The ppm file cannot be opened for writing.");
    }

    (void)fprintf(outfile, "P3\n%d %d\n255\n", width, height);

    unsigned char color;
    for (size_t j = 0, idx = 0; j < height; ++j)
    {
        for (size_t i = 0; i < width; ++i)
        {
            for (size_t c = 0; c < 3; ++c, ++idx)
            {
                color = data[idx];

                if (i == width - 1 && c == 2)
                {
                    (void)fprintf(outfile, "%d", color);
                }
                else
                {
                    (void)fprintf(outfile, "%d ", color);
                }
            }
        }

        (void)fprintf(outfile, "\n");
    }

    (void)fclose(outfile);
}

// NOLINTEND
