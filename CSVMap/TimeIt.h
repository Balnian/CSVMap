#pragma once
#include <chrono>
#include <string>

template< typename unit = std::chrono::milliseconds>
struct TimeIt
{


	template<class T>
	static void timeIt(std::string description, T &&func)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		func();
		auto t2 = std::chrono::high_resolution_clock::now();
		std::cout << description << " took: "
			<< std::chrono::duration_cast<unit>(t2 - t1).count()
			<< " " << durationToString<unit>() << endl;;
	}

	template<class unit>
	static  std::string durationToString()
	{
		return "Unknown Unit";
	}

	template<>
	static  std::string durationToString<std::chrono::nanoseconds>()
	{
		return "nanoseconds";
	}

	template<>
	static  std::string durationToString<std::chrono::microseconds>()
	{
		return "microseconds";
	}

	template<>
	static std::string durationToString<std::chrono::milliseconds>()
	{
		return "milliseconds";
	}

	template<>
	static std::string durationToString<std::chrono::seconds>()
	{
		return "seconds";
	}

	template<>
	static std::string durationToString<std::chrono::minutes>()
	{
		return "minutes";
	}

	template<>
	static std::string durationToString<std::chrono::hours>()
	{
		return "hours";
	}
};