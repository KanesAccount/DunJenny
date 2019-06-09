#include "graph.h"

namespace graphSys {

	Graph::Graph()
	{
		iteration = 0;
	}

	Graph::~Graph()
	{}

	void Graph::addNode(Node n)
	{
		nodes.push_back(n);
	}
	
	void Graph::delNode(std::vector<Node>& nodeVec)
	{
		int nodePos;

		for (int i = 0; i < nodeVec.size(); i++)
		{
			for (int j = 0; j < nodes.size(); j++)
			{
				if (nodes.at(j).getID() == nodeVec.at(i).getID())
					nodePos = j;
				else
					nodePos = 0;
			}
			nodes.erase(nodes.begin() + nodePos);
		}
	}

	void Graph::delNode(std::vector<Node>& nodeVec, size_t pos)
	{
		if (nodeVec.size() > 0)
		{
			if (pos == 0)
				nodes.erase(nodes.begin());
			else
				nodes.erase(nodes.begin() + pos);
		}
	}

	void Graph::addEdge(Edge e)
	{
		edges.push_back(e);
	}

	void Graph::delEdge(std::vector<Edge>& edgeVec)
	{
		int edgePos; 

		for (int i = 0; i < edgeVec.size(); i++)
		{
			for (int j = 0; j < edges.size(); j++)
			{
				if (edges.at(j).getSrc().getID() == edgeVec.at(i).getSrc().getID())
					edgePos = j;
				else
					edgePos = 0;
			}
			if(edges.size() > 0 )
				edges.erase(edges.begin() + edgePos);
		}
	}

	std::vector<Edge> Graph::getConnectedEdges(Node n)
	{
		std::vector<Edge> connections;
		for (int i = 0; i < edges.size(); i++)
		{
			if (edges.at(i).getSrc().getID() == n.getID() 
				|| edges.at(i).getTarget().getID() == n.getID())
			{
				connections.push_back(edges.at(i));
			}
		}
		return connections;
	}

	bool Graph::hasSource(Node n, Graph G)
	{
		for (int i = 0; i < G.getGraphEdges().size(); i++)
		{
			if (n.getID() == G.getGraphEdges().at(i).getTarget().getID())
				return true;
		}
		return false;
	}

	bool Graph::hasTarget(Node n, Graph G)
	{
		for (int i = 0; i < G.getGraphEdges().size(); i++)
		{
			if (n.getID() == G.getGraphEdges().at(i).getSrc().getID())
				return true;
		}
		return false;
	}

	bool Graph::containsNode(Node n)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes.at(i).getID() == n.getID())
				return true;
		}
		return false;
	}

	Node Graph::nodeAtID(int id)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			if (id == nodes.at(i).getID())
			{
				return nodes.at(i);
			}
		}		
		return nullNode;
	}

	Node Graph::nodeWithLabel(char label)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			if (label == nodes.at(i).getLabel())
			{
				return nodes.at(i);
			}
		}
		return nullNode;
	}

	bool Graph::matchEdge(Edge* one)
	{
		Edge empty;
		empty.setType("empty");
		//Check if both edges share the same source and target types
		if (one->getSrc().getType() != empty.getSrc().getType())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	Node Graph::randomMatch(Node n, Graph g)
	{
		int node;

		if (g.nodes.size() > 0)
		{
			do
			{
				node = rand() % g.nodes.size();
			} while (g.nodes.at(node).getType() != n.getType());
		}
		return g.nodes.at(node);
	}

	void Graph::clearGraph()
	{
		edges.clear();
		nodes.clear();
	}

	std::vector<std::string> Graph::printGraphNodes(std::vector<std::pair<int, int>> ids)
	{
		//Print nodes
		for (int i = 0; i < nodes.size(); i++)
		{
			int id = nodes[i].getID();
			std::string idStr = std::to_string(id);

			auto node = std::find_if(ids.begin(), ids.end(), [=](auto& ids) 
			{ if(ids.first == id)
				return ids.first == id; });

			int nodeID = node->second;

			nodeList.push_back(std::to_string(nodeID));
		}
		return nodeList;
	}

	std::vector<std::string> Graph::printGraph(std::vector<std::pair<int, int>> ids)
	{
		for (int i = 0; i < edges.size(); i++)
		{
			int src = edges[i].getSrc().getID();
			int trg = edges[i].getTarget().getID();
			int* srcID;
			int* trgID;
			std::string srcStr;
			std::string targetStr;
			bool source = false;
			bool target = false;

			if (!getGraphEdges().at(i).getAutoID())
			{
				auto nodeSrc = std::find_if(ids.begin(), ids.end(), [&](auto& ids)
				{
					if (ids.first == src)
						source = true;
					return ids.first == src;
				});
				auto nodeTrg = std::find_if(ids.begin(), ids.end(), [&](auto& ids)
				{
					if (ids.first == trg)
						target = true;
					return ids.first == trg ;
				});

				if (source)
				{
					srcID = &nodeSrc->second;
					srcStr = std::to_string(*srcID);
				}
				if (target)
				{
					trgID = &nodeTrg->second;
					targetStr = std::to_string(*trgID);
				}
			}
			
			const char* srcChar = srcStr.c_str();
			const char* trgChar = targetStr.c_str();

			char buffer[256]; // <- danger, only storage for 256 characters.
			strncpy(buffer, srcChar, sizeof(buffer));
			strncat(buffer, "---", sizeof(buffer));
			strncat(buffer, trgChar, sizeof(buffer));

			std::string edge = (buffer);

			if(source & target)
				edgeList.push_back(edge);
		}
		return edgeList;
	}

	std::pair<int, int> Graph::calcDistances()
	{
		int xDist1 = 0, xDist2 = 0, yDist1 = 0, yDist2 = 0;

		int tempXMax = 0, tempYMax = 0;
		int maxXDist = 0, maxYDist = 0;
		int minXDist = 0, minYDist = 0;

		std::vector<std::pair<int, int>> realDists;
		std::pair<int, int> mDist = std::pair<int, int>(0, 0);

		if (distances.size() > 0)
		{
			for (int i = 0; i < distances.size() - 1; i++)
			{
				xDist1 = distances.at(i).first;
				xDist2 = distances.at(i + 1).first;
				yDist1 = distances.at(i).second;
				yDist2 = distances.at(i + 1).second;

				realDists.push_back(std::pair<int, int>((xDist2 - xDist1), (yDist2 - yDist1)));  
			}

			for (int i = 0; i < realDists.size(); i++)
			{
				if (distances.at(i).first > tempXMax || distances.at(i).second > tempYMax)
				{
					tempXMax = distances.at(i).first;
					tempYMax = distances.at(i).second;
				}
			}
			mDist = std::pair<int, int>(tempXMax, tempYMax);
		}
		return mDist;
	}
}