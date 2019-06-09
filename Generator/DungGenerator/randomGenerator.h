/// \file randomGenerator.h
/// \breif Pseudo generator for non-deterministic random numbers
/// \adapted from D. Zadravec (2017) 'A Simple C++ Pseudo Random Number Generator
/// \source: https://stackoverflow.com/questions/41875884/how-to-generate-random-numbers-in-c-without-using-time-as-a-seed?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
/// \author Kane White 
/// \todo  
#pragma once
//includes
#include <random>  
#include <algorithm>
#include <vector> 

//header contents
class RandomGenerator
{
public:
	RandomGenerator();
	~RandomGenerator();

	int GenerateUniform(int min, int upperBmaxmaxound);
	int GenerateGaussian(int min, int upperBmaxmaxound);
};
