#pragma once

#include <random>

class Random
{
public:
	static void Init()
	{
		RandomGenerator.seed(std::random_device()());
	}

	static float Float()
	{

		return (float)Distribution(RandomGenerator) / (float)std::numeric_limits<uint32_t>::max();
	}

private:
	static std::mt19937 RandomGenerator;
	static std::uniform_int_distribution<std::mt19937::result_type> Distribution;
};