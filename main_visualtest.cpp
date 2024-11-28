#include "Base.hpp"

#include "raylib.h"

#include <cstdlib>
#include <iostream>

#include "Renderer.hpp"

constexpr double w = 600;
constexpr double h = 600;

static bool killme;

void teardown()
{
    if (killme)
        exit(EXIT_SUCCESS);
}

// TODO come back to this after you have implemented barycentric coordinates
void ColorInterpolation()
{
    const m::Pixel top = { 255, 0, 0 };
    const m::Pixel left = { 0, 255, 0 };
    const m::Pixel right = { 0, 0, 255 };

    Image c = GenImageColor(w, h, BLANK);
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            if (i > j)
            {
                continue;
            }
            m::Pixel p = m::lerp(top, left, i / w);
            p = m::lerp(p, right, j / h);
            ImageDrawPixel(&c, i, j, { p.r, p.g, p.b, 255 });
        }
    }
    Texture t = LoadTextureFromImage(c);
    while (not(killme = WindowShouldClose()))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            std::cout << GetMousePosition().x << "\t" << GetMousePosition().y
                      << std::endl;
        }

        BeginDrawing();
        DrawTexture(t, 0, 0, WHITE);
        EndDrawing();
    }
}

// void LineEquation() {
//     Image c = GenImageColor(w, h, BLANK);
//     for (int i = 0; i < w; i++)
//     {
//         for (int j = 0; j < h; j++)
//         {
//             if (i > j)
//             {
//                 continue;
//             }
//             m::Pixel p = m::lerp(top, left, i / w);
//             p = m::lerp(p, right, j / h);
//             ImageDrawPixel(&c, i, j, { p.r, p.g, p.b, 255 });
//         }
//     }
// }

int main()
{
    InitWindow(600, 600, "Color Interpolation Test");
    ColorInterpolation();
}
