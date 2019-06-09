/// \file graphBuilder.h
/// \breif	 
/// \author Kane White 
/// \todo  
#pragma once
//includes
#include "ruleFactory.h"
#include "generationStrategy.h"
#include <chrono>

//header contents
class GraphBuilder 
{
private:
	graphSys::Graph G;
	graphSys::RuleFactory rf;
	std::vector<graphSys::Graph> graphUpdates;


	std::vector<std::pair<char*, int>> nodeNames;
	bool firstLoad = true;
	std::vector<std::string> rulesApplied;

	std::chrono::high_resolution_clock::time_point preGenTime;
	std::chrono::high_resolution_clock::time_point postGenTime;

public:
	GraphBuilder(graphSys::RuleFactory RF);
	~GraphBuilder();

	inline graphSys::Graph getGraph() { return G; }
	inline graphSys::RuleFactory getRF() { return rf; }
	inline void setRF(graphSys::RuleFactory nrf) { rf = nrf; }
	inline std::vector<graphSys::Graph> getGraphUpdates() { return graphUpdates; }
	inline std::vector<std::pair<char*, int>> getNodeNames() { return nodeNames; }

	void testRules();
	graphSys::Graph onInit(std::vector<graphSys::Rule> existingRules, graphSys::Graph G);

	inline void newGraph() { G.clearGraph(); rf.clearRules(); }
	inline void setFirstLoad(bool t) { firstLoad = t; }
	inline void clearGeneratedRules() { rulesApplied.clear(); }

	void initRule(std::string rID);
	graphSys::Node addNewNode(char newNode, int id, std::string type);
	graphSys::Edge addEdge(graphSys::Rule rule, int side, graphSys::Node src, graphSys::Node trg);

	graphSys::Node edgeSrc;
	graphSys::Node edgeTrg;

	std::vector<graphSys::Node> nodes;
	std::vector<graphSys::Edge> edges;

	graphSys::Components left{ nodes, edges };
	graphSys::Components right{ nodes, edges };

	graphSys::Node nodeName;
	std::string ruleName;
	char* editedName = " ";
	graphSys::Node sidToGet;
	graphSys::Node tidToGet;

	//Test variables
	std::vector<int> sizes;
	std::vector<int> iterations;
	std::vector<long long> graphGenTime;
	std::vector<bool> constraintsMet;

};