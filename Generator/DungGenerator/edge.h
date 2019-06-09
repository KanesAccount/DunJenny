/// \file edge.h
/// \breif	 
/// \author Kane White 
/// \todo  
#pragma once
//includes
#include "node.h"

//header contents
namespace graphSys {
	class Edge {
	private:
		Node srcNode;
		Node targetNode;
		std::string edgeType;
		bool autoIdGen = false;
	public:
		Edge();
		Edge(Node srcNode, Node targetNode, std::string edgeType = "default");
		~Edge();

		inline Node getSrc() { return srcNode; }
		inline void setSrc(Node& source) { srcNode = source; }
		inline Node getTarget() { return targetNode; }
		inline void setTarget(Node& target) { targetNode = target; }
		inline std::string getType() { return edgeType; }
		inline void setType(std::string type) { edgeType = type; }
		inline void setAutoID(bool gen) { autoIdGen = gen; }
		inline bool getAutoID() { return autoIdGen; }
	};
}
