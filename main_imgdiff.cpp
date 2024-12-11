#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>

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

const std::string resource_path { CENG477_RESOURCE };

Image cmpimg(Image l, Image r)
{
    std::vector<base::Vec2i> diffcoord;
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
    Image mask = GenImageColor(l.width, l.height, BLANK);
    for (auto d : diffcoord)
    {
        ImageDrawPixel(&mask, d.x, d.y, VIOLET);
    }
    return mask;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        puts("not enough arguments");
        exit(EXIT_SUCCESS);
    }
    const std::string lname = std::string(argv[1]);
    const std::string rname = std::string(argv[2]);
    const Image left = LoadImage(lname.c_str());
    const Image right = LoadImage(rname.c_str());
    const Image diff = cmpimg(left, right);

    // early init required for loading Texture (a GPU-bound object)
    InitWindow(left.width, left.height,
               ((std::string("Renderdiff:  ") + lname).c_str()));
    SetTargetFPS(10);
    SetTraceLogLevel(LOG_FATAL);

    // States needed to make a new frame
    bool show_right = false;
    bool diffmode = false;

    while (!WindowShouldClose())
    {
        show_right = IsKeyDown(KEY_LEFT_SHIFT);
        diffmode = IsKeyDown(KEY_SPACE);

        Image canvas;

        if (not show_right)
        {
            canvas = right;
        }
        else
        {
            canvas = left;
        }

        if (diffmode)
        {
            canvas = diff;
        }
        Texture2D tx = LoadTextureFromImage(canvas);
        BeginDrawing();
        ClearBackground(BLANK);
        DrawTexture(tx, 0, 0, WHITE);
        EndDrawing();
        UnloadTexture(tx);
    }
    CloseWindow();
}
