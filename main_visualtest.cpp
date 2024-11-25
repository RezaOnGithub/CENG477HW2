#include "Base.hpp"
#include "raylib.h"

#include <iostream>

const Pixel top = {255,0,0};
const Pixel left = {0,255,0};
const Pixel right = {0,0,255};


// TODO come back to this after you have implemented barycentric coordinates
int main()
{
    InitWindow(600, 600, "Color Interpolation Test");
    Image c = GenImageColor(600, 600, BLANK);
    for (int i = 0; i < 800; i++)
    {
        for (int j = 0; j < 600; j++)
        {
            if (i > j)
            {
                continue;
            }
            Pixel p = lerp(top, left, i / 600.);
            p = lerp(p, right, j/600.);
            ImageDrawPixel(&c, i, j, { p.r, p.g, p.b, 255 });
        }
    }
    Texture t = LoadTextureFromImage(c);
    while (not WindowShouldClose())
    {
        BeginDrawing();
        DrawTexture(t, 0, 0, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            std::cout << GetMousePosition().x << "\t" << GetMousePosition().y
                      << std::endl;
        }
        EndDrawing();
    }
}
