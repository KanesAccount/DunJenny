/// \file generationStrategy.h
/// \breif	 
/// \author Kane White 
/// \todo  
#pragma once
//includes
#include "rule.h"
#include "ruleFactory.h"

//header contents
namespace graphSys {
	class GenerationStrategy {
	protected:
		RuleFactory RF;
		RandomGenerator RG;
		std::vector<Rule> rules;
		Graph graph;
		Graph Fail;
		std::vector<std::pair<int, int>> ids;
		std::vector<Node> matchedLeftNodes;
		std::vector<Edge> matchedLeftEdges;
		std::unordered_map<int, Node> matches;
		std::vector<int> mappedIDs;
		Node initialNode;
		Edge initialEdge;

		std::vector<std::pair<std::string, std::string>> graphEdgeMap;
		std::vector<Rule> potentialReplacements;

	public:
		GenerationStrategy();
		GenerationStrategy(RuleFactory rf, Graph startGraph, std::vector<std::pair<int, int>> ids/*, std::vector<Node> nonTerminals, int avgDerivations*/);
		~GenerationStrategy();
		std::vector<std::pair<std::string, std::string>> getGraphEdges();
		void checkLeftNodes(Rule rule, Graph G);
		void checkLeftEdges(Rule rule, Graph G);
		void filterNodes(Rule rule, Graph G);
		Components addProduction(Components rightSide);
		Graph applyRule(Rule rule, Graph graph);
		Graph deriveGraph(Graph G);

		inline std::vector<Rule> getPotentialReplacements() { return potentialReplacements; }
		inline std::vector<Node> getMatches() { return matchedLeftNodes; }
		//inline Graph updateGraph() { return graph; }

		std::pair<int, int> lastPos = std::pair<int, int>(100,100);
		std::vector<std::pair<int, int>> matchingNodes;
		graphSys::Components leftSide;
	};
}
