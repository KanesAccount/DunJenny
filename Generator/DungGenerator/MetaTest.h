#pragma once
#include <iostream>
#include "json.hpp"

#include "JsonCast.h"

#include "Person.h"

class Unregistered
{ };

#include "Meta.h"

class MetaTest
{
public:
	std::vector<std::string> strings;
	std::vector<std::string> PrintMetaTest();
};