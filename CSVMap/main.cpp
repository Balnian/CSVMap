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



using namespace std;

vector<vector<double>> parseCSV(string path, double &min, double & max, int sizeX = 2000, int sizeY = 2000);
vector<double> parseCSV1(string path, double &min, double & max, int& sizeX, int& sizeY);
void partial_parse(std::vector<double> & arr, string_view csvFile, unsigned int begin, unsigned int length);



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
	/*vector<vector<double>>*/ auto zterrain = parseCSV1(source.value(), min, max, x, y);
	cout << "X: " << x << ", Y: " << y << endl;
	/*for (auto &line : zterrain)
	{
		for (auto &z : line)
		{
			cout << z << ", ";
		}
		cout << endl;
	}*/

	/*vector<vector<double>> vec(10, vector<double>(10));
	double i = 0;
	std::for_each(vec.begin(), vec.end(), [&](vector<double> &v2)
	{
		 std::generate(v2.begin(), v2.end(), [&]() {return i++; });

	});
	for (auto &line : vec)
	{
		for (auto &z : line)
		{
			cout << z << ", ";
		}
		cout << endl;
	}
	GrayScaleHeightMap(vec, 0, 99);*/

	/*cout << "Fini1" << endl;
	cout << "Fini1" << endl;
*/
	cout << zterrain[0] << endl;
	auto t1 = std::chrono::high_resolution_clock::now();
	GrayScaleHeightMap(zterrain, min, max, x, y);
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "GPU compute took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";
	cout << zterrain[0] << endl;
	png::image<png::gray_pixel_16> image(x, y);
	long long count = 0;
	for (size_t y = 0; y < image.get_height(); ++y)
	{
		for (size_t x = 0; x < image.get_width(); ++x)
		{
			count += static_cast<png::gray_pixel_16>(zterrain[image.get_width()*y + x]);
			image[y][x] = static_cast<png::gray_pixel_16>(zterrain[image.get_width()*y + x]);
			// non-checking equivalent of image.set_pixel(x, y, ...);
		}
	}
	image.write(output.value().c_str());
	cout << output.value() << endl;
	cout << count << endl;
	getchar();

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
vector<double> parseCSV1(string path, double &min, double & max, int& sizeX, int& sizeY)
{

#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

	vector<double> arr;
	arr.reserve(sizeY*sizeX);
	double minVal = numeric_limits<double>::max();
	double maxVal = numeric_limits<double>::min();
	int y = -1;
	//int x = -1;
	ifstream csvFile(path);
	/*stringstream csvFile;
	auto t1 = std::chrono::high_resolution_clock::now();
	csvFile << ifstream(path, ios::binary).rdbuf();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Loading in memory took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds\n";*/

	auto t3 = std::chrono::high_resolution_clock::now();
	//if (csvFile.is_open())
	{

		double currY = numeric_limits<double>::min();
		string line;
		int charCount = 0;
		int totalOffset = 0;
		getline(csvFile, line);//remove column name
		while (getline(csvFile, line))
		{
			/*size_t zBegin = line.find_last_of(',');
			size_t ybegin = line.find_first_of(',');
			string yVal = line.substr(ybegin + 1, zBegin - ybegin - 1);
			string zVal = line.substr(zBegin + 1, line.size() - 1 - zBegin - 1);*/
			charCount = 0;
			totalOffset = 0;
			double_conversion::StringToDoubleConverter stdc(double_conversion::StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0, 0, "", "");
			double dXVal = stdc.StringToDouble(line.c_str(), line.length(), &charCount); //strtod(line.c_str(), &currEnd);
			totalOffset += charCount+1;
			double dYVal = stdc.StringToDouble(line.c_str() + totalOffset, line.length()-totalOffset, &charCount); // atof(yVal.c_str());
			totalOffset += charCount + 1;
			double dZVal = stdc.StringToDouble(line.c_str() + totalOffset, line.length() - totalOffset, &charCount); // atof(zVal.c_str());
			if (currY != dYVal)
			{
				/*if (y > 0)
					arr[y].shrink_to_fit();

				if (y >= arr.size())
				{
					arr.emplace_back();
					arr.reserve(arr.back().size());
				}*/
				currY = dYVal;
				++y;
				//if (y % 100 == 0)
				//cout << y << endl;

			}
			arr.emplace_back(dZVal);
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
	sizeX = arr.size() / (y + 1);
	sizeY = y + 1;
	return  arr;
}
//
//vector<double> parseCSV2(string path, double &min, double & max, int& sizeX, int& sizeY)
//{
//
//#ifdef max
//#undef max
//#endif // max
//#ifdef min
//#undef min
//#endif // min
//
//	vector<double> arr;
//	arr.reserve(sizeY*sizeX);
//	double minVal = numeric_limits<double>::max();
//	double maxVal = numeric_limits<double>::min();
//	int y = -1;
//	//int x = -1;
//	ifstream csvFile(path);
//	/*stringstream csvFile;*/
//	auto t1 = std::chrono::high_resolution_clock::now();
//	//csvFile << ifstream(path, ios::binary).rdbuf();
//	string data{ (std::istreambuf_iterator<char>(csvFile)), std::istreambuf_iterator<char>() };
//
//	auto t2 = std::chrono::high_resolution_clock::now();
//	std::cout << "Loading in memory took "
//		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
//		<< " milliseconds\n"; */
//
//		auto t3 = std::chrono::high_resolution_clock::now();
//	if (csvFile.is_open())
//	{
//		vector<vector<double>> arrs;
//		arrs.reserve(thread::hardware_concurrency());
//		unsigned int blocSize = data.size() / thread::hardware_concurrency();
//		for (size_t i = 0; i < thread::hardware_concurrency(); i++)
//		{
//			arrs.emplace_back();
//			arrs.reserve(blocSize);
//			partial_parse(arrs.back())
//		}
//		csvFile.seekg(0, ios::end);
//		istream::pos_type size = csvFile.tellg();
//		csvFile.seekg(0, ios::beg);
//		std::filebuf* buf = csvFile.rdbuf();
//		buf.
//
//	}
//
//	auto t4 = std::chrono::high_resolution_clock::now();
//	std::cout << "Parsing CSV took "
//		<< std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count()
//		<< " milliseconds\n";
//	min = minVal;
//	max = maxVal;
//	sizeX = arr.size() / (y + 1);
//	sizeY = y + 1;
//	return  arr;
//}
//
//void partial_parse(std::vector<double> & arr, string_view csvFile, unsigned int begin, unsigned int length)\
//{
//	{
//
//		double currY = numeric_limits<double>::min();
//		string line;
//		char * currEnd = nullptr;
//		getline(csvFile, line);//remove column name
//		while (getline(csvFile, line))
//		{
//			size_t zBegin = line.find_last_of(',');
//			size_t ybegin = line.find_first_of(',');
//			string yVal = line.substr(ybegin + 1, zBegin - ybegin - 1);
//			string zVal = line.substr(zBegin + 1, line.size() - 1 - zBegin - 1);
//			//double dXVal = strtod(line.c_str(), &currEnd);
//			double dYVal = /*strtod(currEnd + 1, &currEnd);*/  atof(yVal.c_str());
//			double dZVal = /*strtod(currEnd + 1, &currEnd);*/  atof(zVal.c_str());
//			if (currY != dYVal)
//			{
//				/*if (y > 0)
//				arr[y].shrink_to_fit();
//
//				if (y >= arr.size())
//				{
//				arr.emplace_back();
//				arr.reserve(arr.back().size());
//				}*/
//				currY = dYVal;
//				++y;
//				//if (y % 100 == 0)
//				//cout << y << endl;
//
//			}
//			arr.emplace_back(dZVal);
//			minVal = std::min(dZVal, minVal);
//			maxVal = std::max(dZVal, maxVal);
//		}
//	}
//}
