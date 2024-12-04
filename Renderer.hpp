#pragma once
#include "Base.hpp"
#include "World.hpp"
#include "Rasterizer.hpp"
#include <vector>

// Rasterize every triangle, perform depth test, return an output buffer
std::vector<std::vector<Pixel>> render(const World &w, const ViewConfig &v);