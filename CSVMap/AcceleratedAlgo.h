#pragma once

#include <vector>


void GrayScaleHeightMap(std::vector<double>& zvalues, const double & min, const double & max, int width, int height, unsigned int bits = 16/*int * Src, int * Copy, int width, int height, int stride*/);
