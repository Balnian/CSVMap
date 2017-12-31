#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <optional>
#include <sstream>
#include <iterator>
#include <chrono>
#include <thread>

#include <double-conversion\double-conversion.h>
#include "AcceleratedAlgo.h"
#define strerror_r(errno,buf,len) strerror_s(buf,len,errno)
#include <png++\png.hpp>

#include "TimeIt.h"


using namespace std;

vector<vector<double>> parseCSV(string path, double &min, double & max, int sizeX = 2000, int sizeY = 2000);
vector<double> parseCSV1(string path, double &min, double & max, int& sizeX, int& sizeY);
void partial_parse(std::vector<double> & arr, string_view csvFile, unsigned int begin, unsigned int length);
void writePNG(const vector<double> & data, string file, int x, int y);
void writeRAW(const vector<double> & data, string file, int x, int y);



int main(int argc, char *argv[])
{
	optional<string> source;
	optional<string> output;
	vector<string> args(argv + 1, argv + argc);
	try
	{


		for (size_t i = 0; i < args.size(); i++)
		{
			string s = args[i];
			if (s == "--out" || s == "-o")
			{
				if (!output)
				{
					if (!((i + 1) >= args.size()))
					{
						++i;
						output = args[i];
					}
					else
					{
						throw runtime_error("Missing argument for parameter: " + s);
					}
				}
				else
				{
					throw runtime_error("Parameter: \"" + s + "\" already specified");
				}


			}
			else
			{
				if (!source)
				{
					source = s;
				}
				else
				{
					throw runtime_error("Too many arguments for Source path");
				}
			}
		}

		if (!source)
		{
			throw runtime_error("Missing argument for Source path");
		}
		else if (!output)
		{
			output = source.value() + ".png";
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		EXIT_FAILURE;
	}

	double min, max;
	int x = 7501, y = 5996;
	auto zterrain = parseCSV1(source.value(), min, max, x, y);
	cout << "X: " << x << ", Y: " << y << endl;


	TimeIt<>::timeIt("GPU Compute", [&]()
	{
		AcceleratedAlgo::LongLatToMap16(zterrain, min, max, x, y);
	});
	/*auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "GPU compute took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";
	cout << zterrain[0] << endl;*/

	/*TimeIt<>::timeIt("Writing PNG", [&]()
	{
		writePNG(zterrain, output.value(), x, y);
	});*/

	TimeIt<>::timeIt("Writing RAW", [&]()
	{
		writeRAW(zterrain, output.value()+".raw",x,y);
	});
	getchar();

}





// Convert number of bytes to human readable size
// if size is -1 it's because the file wasn't open correctly
string print_byte_to_unit(long long && bytes)
{
	int unit = 1024;
	if (bytes < unit) return bytes + " B";
	long exp = (long)(log(bytes) / log(unit));
	string pre = "KMGTPE";
	stringstream sstr;
	sstr.precision(2);
	sstr << bytes / pow(unit, exp) << " " + pre.substr(exp - 1, 1) + "B";
	return sstr.str();


}

// Print File Infos
template <class C>
void Print_file_info(const basic_string<C> &s) {
	// ate signifie at end
	cout << "Opened file : " << s << endl;
	cout << "Size : " << print_byte_to_unit(static_cast<long long>(basic_ifstream<C>(s, ios::ate).tellg())) << endl;
}
vector<vector<double>> parseCSV(string path, double &min, double & max, int sizeX, int sizeY)
{

#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

	vector<vector<double>> arr(sizeY, vector<double>(sizeX));
	double minVal = numeric_limits<double>::max();
	double maxVal = numeric_limits<double>::min();
	int y = -1;
	//int x = -1;
	//ifstream csvFile(path);
	stringstream csvFile;
	auto t1 = std::chrono::high_resolution_clock::now();
	csvFile << ifstream(path, ios::binary).rdbuf();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Loading in memory took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";

	auto t3 = std::chrono::high_resolution_clock::now();
	//if (csvFile.is_open())
	{

		double currY = numeric_limits<double>::min();
		string line;
		getline(csvFile, line);//remove column name
		while (getline(csvFile, line))
		{
			size_t zBegin = line.find_last_of(',');
			size_t ybegin = line.find_first_of(',');
			string yVal = line.substr(ybegin + 1, zBegin - ybegin - 1);
			string zVal = line.substr(zBegin + 1, line.size() - 1 - zBegin - 1);
			double dYVal = atof(yVal.c_str());
			double dZVal = atof(zVal.c_str());
			if (currY != dYVal)
			{
				if (y > 0)
					arr[y].shrink_to_fit();

				if (y >= arr.size())
				{
					arr.emplace_back();
					arr.reserve(arr.back().size());
				}
				currY = dYVal;
				++y;
				//if (y % 100 == 0)
					//cout << y << endl;

			}
			arr.back().emplace_back(dZVal);
			minVal = std::min(dZVal, minVal);
			maxVal = std::max(dZVal, maxVal);
		}
	}
	auto t4 = std::chrono::high_resolution_clock::now();
	std::cout << "Parsing CSV took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count()
		<< " milliseconds\n";
	min = minVal;
	max = maxVal;
	return  arr;
	return {};
}

// Streaming parsing of the file
vector<double> parseCSV1(string path, double &min, double & max, int& sizeX, int& sizeY)
{

#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

	Print_file_info(path);

	vector<double> arr;
	arr.reserve(sizeY*sizeX);
	double minVal = numeric_limits<double>::max();
	double maxVal = numeric_limits<double>::min();
	int y = -1;
	ifstream csvFile(path);

	TimeIt<std::chrono::seconds>::timeIt("Parsing CSV", [&]()
	{
		if (csvFile.is_open())
		{


			double currY = numeric_limits<double>::min();
			string line;
			int charCount = 0;
			int totalOffset = 0;
			double_conversion::StringToDoubleConverter stdc(double_conversion::StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0, 0, "", "");
			getline(csvFile, line);//remove column name
			while (getline(csvFile, line))
			{
				charCount = 0;
				totalOffset = 0;
				for (auto& c : line) { ++totalOffset; if (c == ',')break; }
				double dYVal = stdc.StringToDouble(line.c_str() + totalOffset, line.length() - totalOffset, &charCount); // atof(yVal.c_str());
				totalOffset += charCount + 1;
				double dZVal = stdc.StringToDouble(line.c_str() + totalOffset, line.length() - totalOffset, &charCount); // atof(zVal.c_str());
				if (currY != dYVal)
				{
					currY = dYVal;
					++y;
				}
				arr.emplace_back(dZVal);
				minVal = std::min(dZVal, minVal);
				maxVal = std::max(dZVal, maxVal);
			}
		}
	});

	min = minVal;
	max = maxVal;
	sizeX = arr.size() / (y + 1);
	sizeY = y + 1;
	return  arr;
}

void writePNG(const vector<double> & data, string file, int x, int y)
{
	png::image<png::gray_pixel_16> image(x, y);
	long long count = 0;
	for (size_t y = 0; y < image.get_height(); ++y)
	{
		for (size_t x = 0; x < image.get_width(); ++x)
		{
			count += static_cast<png::gray_pixel_16>(data[image.get_width()*y + x]);
			image[y][x] = static_cast<png::gray_pixel_16>(data[image.get_width()*y + x]);
			// non-checking equivalent of image.set_pixel(x, y, ...);
		}
	}
	image.write(file.c_str());
	cout << "Writing PNG: " << file << endl;

}

void writeRAW(const vector<double> & data, string file, int x, int y)
{
	vector<uint16_t> data16(data.begin(), data.end());
	fstream fout(file, ios::out | ios::binary);

	/*for (size_t i = 0; i < y; i++)
	{
		for (size_t w = 0; w < x; w++)
		{
			fout << data16[x*i + w];
		}
		fout << endl;
	}*/
	for (auto & item : data16)
	{
		fout << item;
	}
	//fout.write((char*)&data16, data16.size() * sizeof(data16));
	fout.close();
}
