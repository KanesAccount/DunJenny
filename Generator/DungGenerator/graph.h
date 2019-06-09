/// \file graph.h
/// \breif	 
/// \author Kane White 
/// \todo  
#pragma once
//includes
#include "rule.h"

//header contents
namespace graphSys {
	class Graph {
	private:
		std::string name;
		std::vector<Node> nodes;
		std::vector<Edge> edges;

		std::vector<std::string> nodeList;
		std::vector<std::string> edgeList;

		std::vector<std::pair<int, int>> ids;
		std::vector<std::pair<int, int>> distances;

		int targetSizeMin = 10;
		int targetSizeMax = 50;

		int targetXDistMin = -1000;
		int targetYDistMin = -1000;
		int targetXDistMax = 1000;
		int targetYDistMax = 1000;

		int minXDist = -1000;
		int minYDist = -1000;
		int maxXDist = 1000;
		int maxYDist = 1000;
		
		int currentNextNodeId;

		Rule updatedRule;
		std::vector<std::string> rulesApplied;

	public:
		int iteration;
		int maxIterations = 100;

		Node nullNode;
		bool completed = false;
		Graph();
		~Graph();

		void addNode(Node n);
		void delNode(std::vector<Node>& nodeVec, size_t pos);
		void delNode(std::vector<Node>& nodeVec);
		void addEdge(Edge e);
		void delEdge(std::vector<Edge>&	 edgeVec);
		std::vector<Edge> getConnectedEdges(Node n);
		bool hasSource(Node n, Graph G);
		bool hasTarget(Node n, Graph G);
		bool containsNode(Node n);
		void clearGraph();
		std::vector<std::string> printGraph(std::vector<std::pair<int, int>> ids);
		std::vector<std::string> printGraphNodes(std::vector < std::pair<int, int>> ids);
		Node nodeAtID(int id);
		Node nodeWithLabel(char label);
		bool matchEdge(Edge* one);
		Node randomMatch(Node n, Graph G);
		
		inline void updateRule(Rule r) { updatedRule = r; }
		inline Rule getUpdatedRule() { return updatedRule; }
		inline void addRuleApplied(std::string rule) { rulesApplied.push_back(rule); }
		inline std::vector<std::string> getGeneratedRules() { return rulesApplied; }
		inline void clearGeneratedRules() { rulesApplied.clear(); }

		inline std::vector<Node> getGraphNodes() { return nodes; }
		inline std::vector<Edge> getGraphEdges() { return edges; }
		inline std::vector<std::string> getNodeList() { return nodeList; }
		inline std::vector<std::string> getEdgeList() { return edgeList; }

		inline int* getTargetSizeMin() { return &targetSizeMin; };
		inline int* getTargetSizeMax() { return &targetSizeMax; };
		inline void setTargetSizeMin(int tSize) { targetSizeMin = tSize; }
		inline void setTargetSizeMax(int tSize) { targetSizeMax = tSize; }

		inline int* getTargetXDistMin() { return &targetXDistMin; };
		inline int* getTargetXDistMax() { return &targetXDistMax; };

		inline int* getTargetYDistMin() { return &targetYDistMin; };
		inline int* getTargetYDistMax() { return &targetYDistMax; };

		inline void setTargetXDistMin(int tXMin) { targetXDistMin = tXMin; }
		inline void setTargetXDistMax(int tXMax) { targetXDistMax = tXMax; }

		inline void setTargetYDistMin(int tYMin) { targetYDistMin = tYMin; }
		inline void setTargetYDistMax(int tYMax) { targetYDistMax = tYMax; }

		std::pair<int, int> calcDistances();

		inline void setDistances(std::pair<int, int> d) { distances.push_back(d); }
		inline std::vector<std::pair<int, int>> getDistances() { return distances; }

		inline void setNextNodeId(int id) { currentNextNodeId = id; }
		inline int getNextNodeId() { return currentNextNodeId; }
		inline void setIds(std::vector<std::pair<int, int>> newIds) { ids = newIds; }
		inline std::vector<std::pair<int, int>> getIds() { return ids; }

		inline void setMaxIter(int max) { maxIterations = max; }

		inline std::string getName() { return name; }
		inline void setName(std::string n) { name = n; }

	};
}