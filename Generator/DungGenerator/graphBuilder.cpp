#include "graphBuilder.h"

GraphBuilder::GraphBuilder(graphSys::RuleFactory RF)
	: rf(RF)
{}

GraphBuilder::~GraphBuilder()
{}

void GraphBuilder::testRules()
{
	//Setup rules
	//Rule One
	rf.addNode(graphSys::RuleFactory::LEFT);
	graphSys::Node one11 = rf.graphSys::RuleFactory::getLeft().nodes.at(0);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	graphSys::Node one = rf.graphSys::RuleFactory::getRight().nodes.at(0);
	graphSys::Node two5 = rf.graphSys::RuleFactory::getRight().nodes.at(1);
	graphSys::Node two6 = rf.graphSys::RuleFactory::getRight().nodes.at(2);
	graphSys::Node two7 = rf.graphSys::RuleFactory::getRight().nodes.at(3);
	graphSys::Node two8 = rf.graphSys::RuleFactory::getRight().nodes.at(4);

	rf.addEdge(graphSys::RuleFactory::RIGHT, one, two5);
	rf.addEdge(graphSys::RuleFactory::RIGHT, two5, two6);
	rf.addEdge(graphSys::RuleFactory::RIGHT, two6, two7);
	rf.addEdge(graphSys::RuleFactory::RIGHT, two7, two8);

	rf.createRule("RuleOne");

	////Rule Two
	rf.addNode(graphSys::RuleFactory::LEFT);
	rf.addNode(graphSys::RuleFactory::LEFT);
	graphSys::Node one1 = rf.graphSys::RuleFactory::getLeft().nodes.at(0);
	graphSys::Node two1 = rf.graphSys::RuleFactory::getLeft().nodes.at(1);
	rf.addEdge(graphSys::RuleFactory::LEFT, one1, two1);
	
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	graphSys::Node one2 = rf.graphSys::RuleFactory::getRight().nodes.at(0);
	graphSys::Node two2 = rf.graphSys::RuleFactory::getRight().nodes.at(1);
	graphSys::Node three2 = rf.graphSys::RuleFactory::getRight().nodes.at(2);
	
	rf.addEdge(graphSys::RuleFactory::RIGHT, one2, two2);
	rf.addEdge(graphSys::RuleFactory::RIGHT, two2, three2);
	
	rf.createRule("RuleTwo");

	//Rule Three
	//Left Side
	rf.addNode(graphSys::RuleFactory::LEFT);
	rf.addNode(graphSys::RuleFactory::LEFT);
	rf.addNode(graphSys::RuleFactory::LEFT);
	graphSys::Node one3 = rf.graphSys::RuleFactory::getLeft().nodes.at(0);
	graphSys::Node two3 = rf.graphSys::RuleFactory::getLeft().nodes.at(1);
	graphSys::Node three3 = rf.graphSys::RuleFactory::getLeft().nodes.at(2);
	rf.addEdge(graphSys::RuleFactory::LEFT, one3, two3);
	rf.addEdge(graphSys::RuleFactory::LEFT, two3, three3);
	
	//Right Side
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	rf.addNode(graphSys::RuleFactory::RIGHT);
	
	graphSys::Node rightOne = rf.graphSys::RuleFactory::getRight().nodes.at(0);
	graphSys::Node rightTwo = rf.graphSys::RuleFactory::getRight().nodes.at(1);
	graphSys::Node rightThree = rf.graphSys::RuleFactory::getRight().nodes.at(2);
	graphSys::Node rightFour = rf.graphSys::RuleFactory::getRight().nodes.at(3);
	
	rf.addEdge(graphSys::RuleFactory::RIGHT, rightOne, rightTwo);
	rf.addEdge(graphSys::RuleFactory::RIGHT, rightTwo, rightThree);
	rf.addEdge(graphSys::RuleFactory::RIGHT, rightThree, rightFour);
	
	rf.createRule("RuleThree");
}

//Entry point for graph derivation from DunJenny.cpp
graphSys::Graph GraphBuilder::onInit(std::vector<graphSys::Rule> existingRules, graphSys::Graph G)
{
	//Load test rules
	if (firstLoad == true)
	{
		testRules();
		firstLoad = false;
	}
	//Instantiate generation strategy
	graphSys::GenerationStrategy strat(rf, G, G.getIds());

	preGenTime = std::chrono::high_resolution_clock::now();

	if (G.getGraphNodes().size() > 0)
	{
		G = strat.deriveGraph(G);
		postGenTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(postGenTime - preGenTime).count();

		//Store graph derivation variables for algorithm evaluation
		graphGenTime.push_back(duration);
		//G.graphIters.push_back(G.iteration);

		if(G.getName() != "FAIL")
			G.completed = true;

		//Update test variables
		sizes.push_back(G.getGraphNodes().size());
		constraintsMet.push_back(G.completed);
		iterations.push_back(G.iteration);
		graphUpdates.push_back(G);
		return G;
	}
	else
		return G; //deriveGraph sets graph name to "FAIL" if no solution found
} 

void GraphBuilder::initRule(std::string rID)
{
	graphSys::Rule r(left, right);
	
	r.setID(rID);

	rf.addRule(r);
}

graphSys::Node GraphBuilder::addNewNode(char name, int id, std::string type)
{
	nodeNames.push_back(std::pair<char*, int>(&name, id));
	nodeName.setID(id);
	nodeName.setLabel(name);
	nodeName.setType(type);

	return nodeName;
}

graphSys::Edge GraphBuilder::addEdge(graphSys::Rule rule, int side, graphSys::Node src, graphSys::Node trg)
{
	graphSys::Edge edge(src, trg);

	return edge;
}
