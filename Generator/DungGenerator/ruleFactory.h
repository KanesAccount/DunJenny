#pragma once
#include "graph.h"
#include "randomGenerator.h"

namespace graphSys {
	class RuleFactory {
	private:
		Components leftSide;
		Components rightSide;
		RandomGenerator rg;
		std::vector<Rule> ruleList;
		std::vector<std::pair<int, int>> idPairs;
	public:
		enum RuleSide {
			LEFT = 0,
			RIGHT = 1
		};
	public:
		RuleFactory();
		~RuleFactory();
		void addNode(RuleSide s, std::string type = "room");
		void addEdge(RuleSide s, Node& src, Node& target);

		inline Components getLeft() { return leftSide; }
		inline Components getRight() { return rightSide; }
		inline void addRule(Rule r) { ruleList.push_back(r); }
		inline void setRules(std::vector<Rule> newRules) { ruleList = newRules; }
		void updateRule(Rule ruleToUpdate, Rule newRule, int side);


		inline std::vector<std::pair<int, int>> getNewIds() { return idPairs; }
		inline void clearIdPairs() { idPairs.clear(); }
		Rule ruleAtId(std::string id);
		Rule generateNewIds(Rule r, Graph G);
		void createRule(std::string ruleID);
		void clearRules();
		void printRule(Rule r);
		void ruleBuilder();

		inline std::vector<Rule> getRules() { return ruleList; }

		char ruleStr[512];
	};
}