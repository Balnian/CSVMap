#include <vector>
#include <amp.h>
#include <amp_math.h>

using std::vector;
using namespace concurrency;

void pick_accelerator()
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

void GrayScaleHeightMap(vector<vector<double>>& zvalues, const double & min, const double & max, unsigned int bits = 16)
{



	const double stepsize = (max - min) / (std::pow(2, bits) - 1);


	concurrency::extent<2> image_extent(zvalues.size(), zvalues[0].size());
	pick_accelerator();

	array_view<double, 2> texture_src{ image_extent, zvalues };


	parallel_for_each(image_extent, [=](index<2> idx) restrict(amp) {


		texture_src[idx] = precise_math::floor(texture_src[idx] / stepsize);

	});
	texture_src.synchronize();


}

int main()
{
	vector<vector<double>> vec(10, vector<double>(10));
	double i = 0;
	std::for_each(vec.begin(), vec.end(), [&](vector<double> &v2)
	{
		std::generate(v2.begin(), v2.end(), [&]() {return i++; });

	});
	GrayScaleHeightMap(vec, 0, 99);
}