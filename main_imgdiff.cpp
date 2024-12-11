// Single-file utility to compare two PNGs
// does not need anything other than raylib

#include "raylib.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

const std::string left = std::string(CENG477_RESOURCE) + "";
const std::string right = std::string(CENG477_RESOURCE) + "../build/";

namespace base
{
struct Pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct Vec2i
{
    int x;
    int y;
};
}   // namespace base

std::vector<base::Vec2i> cmpimg(Image l, Image r)
{
    std::vector<base::Vec2i> diffcoord;
    assert(l.width == r.width && l.height == r.height);
    for (int i = 0; i < l.width; i++)
    {
        for (int j = 0; j < l.height; j++)
        {
            Color lc = GetImageColor(l, i, j);
            Color rc = GetImageColor(r, i, j);
            if (ColorToInt(lc) != ColorToInt(rc))
            {
                diffcoord.push_back({ i, j });
            }
        }
    }
    return diffcoord;
}

Image diff_mask(std::vector<base::Vec2i> diffcoord, size_t w, size_t h)
{
    Image mask = GenImageColor(w, h, BLANK);
    for (auto d : diffcoord)
    {
        ImageDrawPixel(&mask, d.x, d.y, VIOLET);
    }
    return mask;
}

int main(int argc, char** argv)
{
    using namespace std;

    Image l = LoadImage(left.c_str());
    Image r = LoadImage(right.c_str());

    if (l.width != r.width || l.height != r.height)
    {
        fprintf(stderr, "image size mismatch! cannot compare");
        exit(EXIT_FAILURE);
    }

    vector<base::Vec2i> diffcoord = cmpimg(l, r);
    Image diffmask = diff_mask(diffcoord, l.width, l.height);

    for (size_t i = 0; i < l.width; i++)
    {
        for (size_t j = 0; j < l.height; j++)
        {
            Color lc = GetImageColor(l, i, j);
            Color rc = GetImageColor(r, i, j);
            if (ColorToInt(lc) != ColorToInt(rc))
            {
                ImageDrawPixel(&diffmask, i, j, VIOLET);
                diffcoord.push_back(
                    { static_cast<int>(i), static_cast<int>(j) });
            }
        }
    }

    // early init required for texture loading
    InitWindow(l.width * 2, l.height, "ImgDiff");
    SetTargetFPS(60);

    Texture2D lt = LoadTextureFromImage(l);
    Texture2D rt = LoadTextureFromImage(diffmask);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        DrawTexture(lt, 0, 0, WHITE);
        DrawTexture(rt, l.width + 1, 0, WHITE);

        EndDrawing();
    }
    CloseWindow();
}
