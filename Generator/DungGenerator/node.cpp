#include "node.h"

namespace graphSys {
	//Default constructor
	Node::Node()
	{}

	Node::Node(int id, char label, std::string type)
		: nodeID(id), nodeLabel(label), nodeType(type)
	{
	}

	Node::~Node()
	{
	}
}