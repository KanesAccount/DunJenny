#include "ruleFactory.h"

namespace graphSys {

	RuleFactory::RuleFactory()
	{
	}

	RuleFactory::~RuleFactory()
	{
	}

	void RuleFactory::addNode(RuleSide s, std::string type)
	{
		
		int uniqueID = rg.GenerateUniform(1,499);

		char defaultLabel = ' ';
		Node newNode(uniqueID, defaultLabel, type);
		if (s == LEFT)
		{
			leftSide.nodes.push_back(newNode);
		}
		else
		{
			rightSide.nodes.push_back(newNode);
		}
	}

	void RuleFactory::addEdge(RuleSide s, Node& src, Node& target)
	{
		Edge newEdge(src, target);
		if (s == LEFT)
		{
			leftSide.edges.push_back(newEdge);
		}
		else
		{
			rightSide.edges.push_back(newEdge);
		}
	}

	void RuleFactory::createRule(std::string ruleID)
	{
		Rule newRule(leftSide, rightSide);
		newRule.setID(ruleID);
		ruleList.push_back(newRule);

		//Clear leftSide & rightSide when added to list
		leftSide.nodes.clear();
		rightSide.nodes.clear();
		leftSide.edges.clear();
		rightSide.edges.clear();
	}	

	Rule RuleFactory::ruleAtId(std::string id)
	{		
		for (int i = 0; i < ruleList.size(); i++)
		{
			if (id == ruleList.at(i).getID())
			{
				return ruleList.at(i);
			}
		}
 	}

	Rule RuleFactory::generateNewIds(Rule r, Graph G)
	{
		Rule updatedRule;

		int nextId = rg.GenerateUniform(499,998);

		int increment = 0;
		std::vector<std::pair<int, int>> idPairs;

		for (int i = 0; i < r.getRight().nodes.size(); i++)
		{
			idPairs.push_back(std::pair<int, int>(r.getRight().nodes.at(i).getID(), nextId));
			Node n(nextId, r.getRight().nodes.at(i).getLabel(), r.getRight().nodes.at(i).getType());

			updatedRule.setRight(n);
			nextId++;
			increment++;
		}

		for (int i = 0; i < r.getRight().edges.size(); i++)
		{
			Edge newEdge;

			bool srcFound = false;
			bool trgFound = false;
			bool srcAdded = false;
			bool trgAdded = false;
			int it = i;

			while (!srcFound || !trgFound)
			{
				if (r.getRight().nodes.size() == 1)
				{
					srcFound = true;
					srcAdded = true;
					trgFound = true;
					trgAdded = true;
				}
				std::vector<std::pair<int, int>>::iterator itSrc;
				std::vector<std::pair<int, int>>::iterator itTrg;

				if (!srcFound)
				{
					itSrc = std::find_if(idPairs.begin(), idPairs.end(), [&](std::pair<int, int>) mutable { 
						if (i < idPairs.size() && idPairs.at(i).first == r.getRight().edges.at(it).getSrc().getID())
						{
							srcFound = true;
							newEdge.setSrc(updatedRule.getRightNodeAtId(idPairs.at(i).second));
							srcAdded = true;
							return idPairs.at(i).first == r.getRight().edges.at(it).getSrc().getID();
						}
					});
				}

				if (srcFound && !srcAdded)
				{
					newEdge.setSrc(updatedRule.getRightNodeAtId(itSrc->second));
					srcAdded = true;
				}

				if (!trgFound)
				{
					itTrg = std::find_if(idPairs.begin(), idPairs.end(), [&](std::pair<int, int>) mutable {
						if (i < idPairs.size() && idPairs.at(i).first == r.getRight().edges.at(it).getTarget().getID())
						{
							trgFound = true;
							newEdge.setTarget(updatedRule.getRightNodeAtId(idPairs.at(i).second));
							trgAdded = true;
						}
						else if(i < idPairs.size())
						{
							return idPairs.at(i).first == r.getRight().edges.at(it).getTarget().getID();
						}});
				}

				if (trgFound && !trgAdded)
				{
					newEdge.setTarget(updatedRule.getRightNodeAtId(itTrg->second));
					trgAdded = true;
				}

				i++;
			}
			i = it;

			updatedRule.addRightEdge(newEdge);
		}

		return updatedRule;
	}

	void RuleFactory::clearRules()
	{
		ruleList.clear();
	}

	void RuleFactory::updateRule(Rule oldRule, Rule newRule, int side)
	{
		for (int i = 0; i < ruleList.size(); i++)
		{
			graphSys::Rule currentRule = ruleList.at(i);
			if (oldRule.getID() == currentRule.getID())
			{
				if (side == 0)
				{
					ruleList.at(i).getLeft().nodes.clear();
					ruleList.at(i).getLeft().edges.clear();

					for (int j = 0; j < newRule.getLeft().nodes.size(); j++)
					{	//Check that the node hasnt already been added by Node builder
						if (currentRule.getLeftSize() > 0 && currentRule.getLeft().nodes.at(j).getID() == newRule.getLeft().nodes.at(j).getID())
							continue;
						else
							ruleList.at(i).setLeft(newRule.getLeft().nodes.at(j));
					}
					for (int l = 0; l < newRule.getLeft().edges.size(); l++)
					{	//Check that the edge hasnt already been added by edge builder
						//if (currentRule.getLeftEdgeSize() > 0 && currentRule.getLeft().edges.at(l).getSrc().getID//() != newRule.getLeft().edges.at(l).getSrc().getID())
						//	continue;
						//else
							ruleList.at(i).addLeftEdge(newRule.getLeft().edges.at(l));
					}
				}
				else if (side == 1)
				{
					ruleList.at(i).getLeft().nodes.clear();
					ruleList.at(i).getLeft().edges.clear();

					for (int k = 0; k < newRule.getRight().nodes.size(); k++)
					{
						if (currentRule.getRightSize() > 0 && currentRule.getRight().nodes.at(k).getID() == newRule.getRight().nodes.at(k).getID())
							continue;
						else
							ruleList.at(i).setRight(newRule.getRight().nodes.at(k));
					}
					for (int m = 0; m < newRule.getRight().edges.size(); m++)
					{
							ruleList.at(i).addRightEdge(newRule.getRight().edges.at(m));
					}
				}
			}
		}
	}

	void RuleFactory::printRule(Rule r)
	{
		//Def strings
		std::vector<std::string> nodeStringLeft;
		std::vector<std::string> nodeStringRight;
		std::vector<std::string> edgeStringLeft;
		std::vector<std::string> edgeStringRight;

		//Get left side rule nodes
		for (int i = 0; i < r.getLeft().nodes.size(); i++)
		{
			int id = r.getLeft().nodes[i].getID();
			std::string idString = std::to_string(id);
			nodeStringLeft.push_back("Node" + idString);
		}
		//Get right side rule nodes
		for (int i = 0; i < r.getRight().nodes.size(); i++)
		{
			int id = r.getRight().nodes[i].getID();
			std::string idString = std::to_string(id);
			nodeStringRight.push_back("Node" + idString);
		}

		for (int i = 0; i < r.getLeft().edges.size(); i++)
		{
			Node src = r.getLeft().edges[i].getSrc();
			Node target = r.getLeft().edges[i].getTarget();

			std::string srcStr = std::to_string(src.getID());
			std::string targetStr = std::to_string(target.getID());

			edgeStringLeft.push_back(srcStr + "---" + targetStr);
		}

		for (int i = 0; i < r.getRight().edges.size(); i++)
		{
			Node src = r.getRight().edges[i].getSrc();
			Node target = r.getRight().edges[i].getTarget();

			std::string srcStr = std::to_string(src.getID());
			std::string targetStr = std::to_string(target.getID());

			edgeStringRight.push_back(srcStr + "---" + targetStr);
		}
	}

	void RuleFactory::ruleBuilder()
	{

	}
}