#include "rule.h"

namespace graphSys {

	Rule::Rule()
	{}

	Rule::Rule(Components left, Components right)
		: leftSide(left), rightSide(right)
	{
	}

	Rule::~Rule()
	{
	}

	Node Rule::getRightNodeAtId(int id)
	{
		for (int i = 0; i < rightSide.nodes.size(); i++)
		{
			if (rightSide.nodes.at(i).getID() == id)
				return rightSide.nodes.at(i);
		}
	}
}