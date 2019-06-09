#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <time.h>
#include <random>
#include <Meta.h>

namespace graphSys {
	class Node {
	private:
		//template <>
		//auto meta::registerMembers<Node>();
		int nodeID;
		char nodeLabel;
		std::string nodeType;
		int xPos, yPos;
	public:
		Node();
		Node(int id, char label, std::string type = "room");
		~Node();
		
		inline int getID() { return nodeID; }
		inline void setID(int id) { nodeID = id; }

		inline std::string getType() { return nodeType; }
		inline void setType(std::string type) { nodeType = type; }

		inline char getLabel() { return nodeLabel; }
		inline void setLabel(char l) { nodeLabel = l; }

		inline int getXPos() { return xPos; }
		inline int getYPos() { return yPos; }
		inline void setXPos(int pos) { xPos = pos; }
		inline void setYPos(int pos) { yPos = pos; }

	};
}

namespace meta {

	//template <>
	//inline auto registerMembers<graphSys::Node>()
	//{
	//	return members(
	//		member("id", &graphSys::Node::getID, &graphSys::Node::setID), // access //through //getter/setter only!
	//		member("label", &graphSys::Node::getLabel, &graphSys::Node::setLabel), // //same, //but ref getter/setter
	//		member("type", &graphSys::Node::getType, &graphSys::Node::setType),
	//		member("xPos", &graphSys::Node::getXPos, graphSys::Node::setXPos),
	//		member("yPos", &graphSys::Node::getYPos, graphSys::Node::setYPos)
	//	);
	//}

} // end of namespace meta