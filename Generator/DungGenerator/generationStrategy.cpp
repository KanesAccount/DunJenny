#include "generationStrategy.h"

namespace graphSys {

	GenerationStrategy::GenerationStrategy()
	{
	}

	GenerationStrategy::GenerationStrategy(RuleFactory rf, Graph startGraph, std::vector<std::pair<int, int>> ids)
		: RF(rf), rules(rf.getRules()), graph(startGraph), ids(ids)
	{
	}

	GenerationStrategy::~GenerationStrategy()
	{
	}

	void GenerationStrategy::checkLeftNodes(Rule rule, Graph graph)
	{
		if (graph.getGraphNodes().size() == 1)
			initialNode = graph.getGraphNodes().at(0);

		if (graph.getGraphNodes().size() >= rule.getLeftSize()) 
		{
			int matchSize = 0;
			std::vector<std::pair<int, int>> matchesToUse;

			//for each node (starting from 0) of left hand of rule check for matches in mainGraph
			for (int i = 0; i < rule.getLeft().nodes.size(); i++)
			{
				//For each node in the mainGraph check for a type match and add to list if true
				for (int j = 0; j < graph.getGraphNodes().size(); j++)
				{
					if (rule.getLeft().nodes.at(i).getType() == graph.getGraphNodes().at(j).getType())
					{
						matchesToUse.push_back(std::pair<int, int>(rule.getLeft().nodes.at(i).getID(), graph.getGraphNodes().at(j).getID()));

						matchSize++;
					}
				}
			}

			int toUse = 0;
			if (matchesToUse.size() > 0)
			{
				while (matchingNodes.size() < rule.getLeft().nodes.size())
				{
					if (matchesToUse.size() > 2 && matchesToUse.size() % 2 == 0)
					{
						matchingNodes.push_back(matchesToUse.at(toUse));
						toUse += matchSize / rule.getLeft().nodes.size() + 1;
					}
					else if (matchingNodes.size() == 2)
					{
						matchingNodes.push_back(matchesToUse.at(toUse));
						toUse += matchSize / rule.getLeft().nodes.size();
					}
					else
						matchingNodes.push_back(matchesToUse.at(toUse));
				}
			}
		}
	}

	void GenerationStrategy::checkLeftEdges(Rule rule, Graph graph)
	{
		//find all edges in left side of rule
		std::vector<Edge> ruleEdges = rule.getLeft().edges;
		graphSys::Components leftSideReplacement;

		if (matchingNodes.size() > 1 && rule.getLeft().nodes.size() == 1)
		{
			//if left side had only one node choose a random node to replace 
			int randNode = rand() % matchingNodes.size();
			leftSideReplacement.nodes.push_back(graph.nodeAtID(matchingNodes.at(randNode).second));
		}
		else if (matchingNodes.size() > 1 && rule.getLeft().nodes.size() > 1)
		{
			//Add replacement nodes 
			for (int i = 0; i < matchingNodes.size(); i++)
			{
				leftSideReplacement.nodes.push_back(graph.nodeAtID(matchingNodes.at(i).second));
			}
		}
		else if (matchingNodes.size() == 1)
		{
				leftSideReplacement.nodes.push_back(graph.nodeAtID(matchingNodes.at(0).second));
		}

		leftSide = leftSideReplacement;
	}

	void GenerationStrategy::filterNodes(Rule rule, Graph graph)
	{
		//filter node list based on present edges in left hand of rule
		checkLeftNodes(rule, graph);
		checkLeftEdges(rule, graph);

		//Check each edge to see if its src and target nodes have been mapped to the left side rule
		graphSys::Components leftSideReplacement = leftSide;
		graphSys::Components rightSide = rule.getRight();
		Rule newRule;

		if (leftSide.nodes.size() > 0)
		{
			newRule.setLefts(leftSideReplacement.nodes);
			newRule.setRights(rightSide.nodes);
			newRule.addRightEdges(rightSide.edges);
			potentialReplacements.push_back(newRule);
		}
	}

	std::vector<std::pair<std::string, std::string>> GenerationStrategy::getGraphEdges()
	{
		graphEdgeMap.clear();
		for (int i = 0; i < graph.getGraphEdges().size(); i++)
		{
			std::string edgeSrc = graph.getGraphEdges().at(i).getSrc().getType();
			std::string edgeTrg = graph.getGraphEdges().at(i).getTarget().getType();
			graphEdgeMap.push_back(std::pair<std::string, std::string>(edgeSrc, edgeTrg));
		}
		return graphEdgeMap;
	}

	Components GenerationStrategy::addProduction(Components rightSide)
	{
		Components newProduction;

		for (int i = 0; i < rightSide.nodes.size(); i++)	//Add all nodes from rule right to new production
		{
			newProduction.nodes.push_back(rightSide.nodes[i]);
		}

		for (int i = 0; i < rightSide.edges.size(); i++)	//Add all edges from rule right to new production
		{
			newProduction.edges.push_back(rightSide.edges[i]);
		}

		return newProduction;
	}


	Graph GenerationStrategy::applyRule(Rule rule, Graph graph)
	{
		filterNodes(rule, graph);
		
		if (potentialReplacements.size() > 0)
		{
			Components production;
			graphSys::Rule replacement = rule;


			//Update rule right ids to prevent duplicated nodes & edges
			replacement = RF.generateNewIds(replacement, graph);
			replacement.setLefts(leftSide.nodes);
			replacement.setID(rule.getID());
			//Update id map held in graph so that s_Nodes & s_Edges can be generated correctly
			graph.setIds(RF.getNewIds());
			RF.clearIdPairs();
			graph.updateRule(replacement);
			
			//save temp oldSrc and temp oldTarget if they have
			Node tempSrc, tempTarget, randNode;
			std::vector<graphSys::Edge> srcConnections;
			std::vector<graphSys::Edge> trgConnections;

			//Get any sources and taraget edges currently connected to node for replacement
		
			Node firstNode = replacement.getLeft().nodes.at(0);
			if (graph.getConnectedEdges(firstNode).size() > 0)
			{
				if (graph.hasSource(firstNode, graph))
				{
					std::vector<Edge> connections = graph.getConnectedEdges(firstNode);
					Edge sEdge;
					
					for (int i = 0; i < connections.size(); i++)
					{
						if (connections.at(i).getSrc().getID() == firstNode.getID())
							continue;
						else
							tempSrc = connections.at(i).getSrc();
					}

					sEdge.setSrc(tempSrc);
					srcConnections.push_back(sEdge);
				}
			}

			Node lastNode = replacement.getLeft().nodes.back();
			if (graph.getConnectedEdges(lastNode).size() > 0)
			{
				if (graph.hasTarget(lastNode, graph))
				{
					std::vector<Edge> connections = graph.getConnectedEdges(lastNode);
					Edge tEdge;

					for (int i = 0; i < connections.size(); i++)
					{
						if (connections.at(i).getTarget().getID() == lastNode.getID())
							continue;
						else
							tempTarget = connections.at(i).getTarget();
					}

					tEdge.setTarget(tempTarget);
					trgConnections.push_back(tEdge);					
				}
			}

			std::vector<graphSys::Node> danglingNodes;
			std::vector<graphSys::Edge> danglingEdges;

			for (int i = 0; i < replacement.getLeft().nodes.size(); i++)
			{
				danglingNodes.push_back(replacement.getLeft().nodes.at(i));
			}
			//remove dangling nodes	
			if (danglingNodes.size() > 0)
					graph.delNode(danglingNodes);

			//If graph does not contain nodes that the edge is connected to add to dangling edges
			for (int i = 0; i < graph.getGraphEdges().size(); i++)
			{
				if (!graph.containsNode(graph.getGraphEdges().at(i).getSrc()))
					danglingEdges.push_back(graph.getGraphEdges().at(i));
				if(!graph.containsNode(graph.getGraphEdges().at(i).getTarget()))
					danglingEdges.push_back(graph.getGraphEdges().at(i));
			}

			//remove dangling edges
			if (danglingEdges.size() > 0)
				graph.delEdge(danglingEdges);

			//add right side of rule to production
			if (production.nodes.size() == 0)
				production = addProduction(replacement.getRight());

			//set old src / targets to start and end of rule right
			for (int i = 0; i < srcConnections.size(); i++)
			{
				srcConnections.at(i).setTarget(production.nodes.at(0));
				production.edges.push_back(srcConnections.at(i));
			}

			for (int i = 0; i < trgConnections.size(); i++)
			{
				trgConnections.at(i).setSrc(production.nodes.back());
				production.edges.push_back(trgConnections.at(i));
			}

			//add nodes to graph
			for (int i = 0; i < production.nodes.size(); i++)
			{
				production.nodes[i].setXPos(RG.GenerateGaussian(0, *graph.getTargetSizeMin() * 50));
				production.nodes[i].setYPos(RG.GenerateGaussian(0, *graph.getTargetSizeMin() * 50));

				graph.setDistances(std::pair<int, int>(production.nodes[i].getXPos(), production.nodes[i].getYPos()));

				graph.addNode(production.nodes[i]);
			}

			//add edges to graph
			for (int i = 0; i < production.edges.size(); i++)
			{
				graph.addEdge(production.edges[i]);
			}

			//Cleanup
			matchingNodes.clear();
			potentialReplacements.clear();
			srcConnections.clear();
			trgConnections.clear();
			graph.iteration++;
		}
		return graph;
	}

	Graph GenerationStrategy::deriveGraph(Graph G)
	{
		//Get a copy of the production rules 
		std::vector<Rule> rulesCpy = rules;

		int result = 0;
		do
		{
			int graphSize = G.getGraphNodes().size();
			int randN;
			graphSys::Rule rule;

			if (rules.size() != 0)
			{
				randN = RG.GenerateUniform(0, rules.size());
				//Get position of rule if not 0
				if (randN > 0)
					randN--;
				rule = rules.at(randN);
			}
			else
				result = 1;

			Graph G_Copy = G;

			G_Copy = applyRule(rule, G_Copy);

			int graphCopySize = G_Copy.getGraphNodes().size();
			std::pair<int, int> currentMaxDist = G_Copy.calcDistances();

			int targetSizeMin = *G.getTargetSizeMin();
			int targetSizeMax = *G.getTargetSizeMax();

			int targetXDistMax = *G.getTargetXDistMax();
			int targetYDistMax = *G.getTargetYDistMax();

			int targetXDistMin = *G.getTargetXDistMin();
			int targetYDistMin = *G.getTargetYDistMin();

			if (graphCopySize > targetSizeMin + RG.GenerateUniform(0, targetSizeMax - targetSizeMin) && graphCopySize < targetSizeMax &&
				currentMaxDist.first < targetXDistMax && currentMaxDist.second < targetYDistMax && 
				currentMaxDist.first > targetXDistMin && currentMaxDist.second > targetYDistMin)
			{
				G = G_Copy;
				G.addRuleApplied(rule.getID());
				result = 1;
			}
			else if (graphCopySize > graphSize)
			{
				G = G_Copy;
				G.addRuleApplied(rule.getID());
				rules.erase(rules.begin() + randN);
				rules.push_back(G.getUpdatedRule());
			}
			else
			{
				G_Copy.clearGraph();
				if(rules.size() > 0)
					rules.erase(rules.begin() + randN);
			}
			G.iteration++;
		} while (result == 0 && G.iteration < G.maxIterations);
		
		if (G.iteration < G.maxIterations)
		{
			//Delete inital node from graph
			std::vector<Node> node;
			node.push_back(initialNode);
			G.delNode(node);

			//Add start and end nodes to graph
			Node start(0, 's', std::string("start"));
			Node end(999, 'e', std::string("end"));
			Node sTrg = G.nodeAtID(G.getGraphNodes().at(0).getID());
			Node eSrc = G.nodeAtID(G.getGraphNodes().back().getID());
			Edge sEdge, eEdge;

			start.setXPos(sTrg.getXPos() - 200);
			start.setYPos(sTrg.getYPos() - 200);

			end.setXPos(eSrc.getXPos() - 200);
			end.setYPos(eSrc.getYPos() - 200);

			//sEdge.setSrc(start);
			//sEdge.setTarget(sTrg);
			eEdge.setSrc(eSrc);
			eEdge.setTarget(end);

			G.addNode(start);
			G.addNode(end);
			G.addEdge(sEdge);
			G.addEdge(eEdge);

			return G;
		}
		else
		{
			Fail.setName("FAIL");
			return Fail;
		}
	}
}