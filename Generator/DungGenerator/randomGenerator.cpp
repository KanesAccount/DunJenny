#include "randomGenerator.h"

RandomGenerator::RandomGenerator()
{
}

RandomGenerator::~RandomGenerator()
{}

int RandomGenerator::GenerateUniform(int min, int max)
{
	//validate input
	if (min > max)
		throw std::invalid_argument("Invalid generation range!");

	int randomValue;
	//Return a random value using the merseene twister
	std::random_device rd;
	std::mt19937 engine(rd());

	//using uniform distrobution when generating between min and max
	std::uniform_int_distribution<> dist(min, max);

	//Generate value
	randomValue = dist(engine);

	return randomValue;
}

int RandomGenerator::GenerateGaussian(int min, int max)
{
	//validate input
	if (min > max)
		throw std::invalid_argument("Invalid generation range!");

	int randomValue;
	//Return a random value using the merseene twister
	std::random_device rd;
	std::mt19937 engine(rd());

	//using uniform distrobution when generating between min and max
	std::normal_distribution<> dist(min, max);

	//Generate value
	randomValue = dist(engine);

	return randomValue;
}