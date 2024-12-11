#include "Base.hpp"
#include "CENG477.hpp"
#include "Renderer.hpp"
#include "World.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

void write_image(const char* filename, unsigned char* data, long width,
                 long height);

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

    for (const auto& v : views)
    {
        const auto& fragments = render(w, v);
        const size_t pixel_count = v.pixel_grid_rows * v.pixel_grid_columns;
        auto* data = new unsigned char[pixel_count * 3];
        for (size_t x = 0; x < pixel_count; x++)
        {
            data[x * 3 + 0] = v.bg_color.r;
            data[x * 3 + 1] = v.bg_color.g;
            data[x * 3 + 2] = v.bg_color.b;
        }
        for (const auto& frag : fragments)
        {
            const size_t pixel_index =
                frag.pc.row_from_top * v.pixel_grid_columns +
                frag.pc.column_from_left;
            const m::Pixel c = m::vec2color(frag.a.ceng477_color);
            data[pixel_index * 3 + 0] = c.r;
            data[pixel_index * 3 + 1] = c.g;
            data[pixel_index * 3 + 2] = c.b;
        }
        std::string name = v.filename;
        write_image(name.c_str(), data, v.pixel_grid_columns,
                    v.pixel_grid_rows);
        delete[] data;
    }

    return 0;
}

// NOLINTBEGIN
//  Code copied straight from raytracer
void write_image(const char* filename, unsigned char* data, long width,
                 long height)
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
