#include "edge.h"

namespace graphSys {

	Edge::Edge()
	{
	}

	Edge::Edge(Node srcNode, Node targetNode, std::string edgeType)
		: srcNode(srcNode), targetNode(targetNode)
	{
	}

	Edge::~Edge()
	{
	}
}