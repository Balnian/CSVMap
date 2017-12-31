#include "AcceleratedAlgo.h"

#include <vector>
#include <amp.h>
#include <amp_math.h>
#include <iostream>

using std::vector;
using namespace concurrency;

void AcceleratedAlgo::pick_accelerator()
{

	// Get all accelerators known to the C++ AMP runtime
	vector<accelerator> accs = accelerator::get_all();

	// Empty ctor returns the one picked by the runtime by default
	accelerator chosen_one;

	// Choose one; one that isn't emulated, for example
	auto result =
	std::find_if(accs.begin(), accs.end(), [](accelerator acc)
	{
	return !acc.is_emulated; //.supports_double_precision
	});
	if (result != accs.end())
	chosen_one = *(result); // else not shown

	// Output its description (tip: explore the other properties)
	//std::wcout << chosen_one.description << std::endl;

	// Set it as default ... can only call this once per process
	accelerator::set_default(chosen_one.device_path);

	// ... or just return it
	//return chosen_one;
}

void AcceleratedAlgo::LongLatToMap16(vector<double>& zvalues, const double & min, const double & max, int width, int height, unsigned int bits /*= 16*//*int * Src, int * Copy, int width, int height, int stride*/)
{

	

	const double stepsize = (max - min) / (std::pow(2, bits) - 1);

	//std::cout<< stepsize <<std::endl;
	//std::cout<< min <<std::endl;
	//std::cout<< max <<std::endl;

	concurrency::extent<2> image_extent(width, height);
	pick_accelerator();

	array_view<double, 2> texture_src( image_extent, zvalues );
	//array_view<double, 2> texture_cpy( image_extent, zvalues);
	//texture_cpy.discard_data();

	parallel_for_each(image_extent, [=](index<2> idx) restrict(amp) {
	//if (idx[1])

	texture_src[idx] = precise_math::floor((texture_src[idx]-min) / stepsize);

	});
	texture_src.synchronize();
	//texture_cpy.synchronize();

}

