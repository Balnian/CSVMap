#pragma once

#include <vector>
#include <amp.h>
#include <amp_math.h>
struct AcceleratedAlgo
{
private:
	static void pick_accelerator();
public:
	static void LongLatToMap16(std::vector<double>& zvalues, const double & min, const double & max, int width, int height, unsigned int bits = 16/*int * Src, int * Copy, int width, int height, int stride*/);
	
	/*template<typename T>
	static std::vector<T> convert(std::vector<double>& zvalues, int width, int height);*/


};

