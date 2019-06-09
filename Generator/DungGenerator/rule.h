#pragma once
#include "node.h"
#include "edge.h"

namespace graphSys {

	struct Components {
		std::vector<Node> nodes;
		std::vector<Edge> edges;
	};

	struct Rule {
	private:
		std::string ruleID;
		//left side
		Components leftSide;
		//right side 
		Components rightSide;
	public:
		Rule();
		Rule(Components left, Components right);
		~Rule();

		inline void setLeft(Node node) { this->leftSide.nodes.push_back(node); }
		inline void setLefts(std::vector<Node> nodes) { for(int i = 0; i < nodes.size(); i++) this->leftSide.nodes.push_back(nodes.at(i)); }
		inline void setRight(Node node) { this->rightSide.nodes.push_back(node); }
		inline void setRights(std::vector<Node> nodes) { for (int i = 0; i < nodes.size(); i++) this->rightSide.nodes.push_back(nodes.at(i)); }

		Node getRightNodeAtId(int id);

		//inline void setComponents(Components comp) { this-> = comp.}
		inline void addLeftEdge(Edge edge) { this->leftSide.edges.push_back(edge); }
		inline void addLeftEdges(std::vector<Edge> edges) { for (int i = 0; i < edges.size(); i++) this->leftSide.edges.push_back(edges.at(i)); }
		inline void addRightEdge(Edge edge) { this->rightSide.edges.push_back(edge); }
		inline void addRightEdges(std::vector<Edge> edges) { for (int i = 0; i < edges.size(); i++) this->rightSide.edges.push_back(edges.at(i)); }

		inline Components getLeft() { return this->leftSide; }
		inline Components getRight() { return this->rightSide; }

		inline std::string getID() { return ruleID; }
		inline void setID(std::string id) { ruleID = id; }

		inline int getLeftSize() { return this->leftSide.nodes.size(); }
		inline int getRightSize() { return this->rightSide.nodes.size(); }

		inline int getLeftEdgeSize() { return this->leftSide.edges.size(); }
		inline int getRightEdgeSize() { return this->rightSide.edges.size(); }

		inline void updateRule(Components left, Components right) { leftSide = left; rightSide = right; }
		inline void clear() { leftSide.nodes.clear(); leftSide.edges.clear(); rightSide.nodes.clear(); rightSide.edges.clear(); }
	};
}