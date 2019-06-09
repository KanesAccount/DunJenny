#include "Application.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <utility>
#include <sstream>
#include "NodeEditor.h"
#include "Math2D.h"
#include "Interop.h"
#include "Builders.h"
#include "Widgets.h"
#include "graphBuilder.h"
#include "MetaTest.h"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;
using ax::Widgets::IconType;

static ed::EditorContext* g_Context = nullptr;

static void ShowAppLog(bool* p_open);

graphSys::RuleFactory rf;
GraphBuilder gb(rf);
graphSys::Graph generatedGraph;
graphSys::Rule newRule;
std::vector<std::pair<int, int>> ids;
bool roomAdded = false;
bool startAdded = false;
bool endAdded = false;
bool errorMsg = false;

int maxIters;
int graphMin;
int graphMax;
int xMaxDist;
int xMinDist;
int yMaxDist;
int yMinDist;

int ruleToView;
bool openRuleEdit = false;
bool openRuleBuilder = false;
bool openDataView = false;
bool genFailed = false;
static bool show_app_log = false;

enum class PinType
{
	Flow,
	Key,
	Lock,
	KeyLock,
	Bool,
	Int,
	Float,
	Object,
	Function,
	Delegate,
};

enum class PinKind
{
	Output,
	Input
};

enum class NodeType
{
	Blueprint,
	Simple,
	Tree,
	Comment
};

struct Node;

struct Pin
{
	ed::PinId   ID;
	::Node*     Node;
	std::string Name;
	PinType     Type;
	PinKind     Kind;

	Pin(int id, const char* name, PinType type) :
		ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
	{
	}
};

struct Node
{
	ed::NodeId ID;
	std::string Name;
	std::vector<Pin> Inputs;
	std::vector<Pin> Outputs;
	ImColor Color;
	NodeType Type;
	ImVec2 Size;

	std::string State;
	std::string SavedState;

	Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
		ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
	{
	}
};

struct Link
{
	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	ImColor Color;

	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
	{
	}
};


static const int            s_PinIconSize = 24;
static std::vector<Node>    s_Nodes;
static std::vector<Link>    s_Links;
static ImTextureID          s_HeaderBackground = nullptr;
static ImTextureID          s_SampleImage = nullptr;
static ImTextureID          s_SaveIcon = nullptr;
static ImTextureID          s_RestoreIcon = nullptr;

struct NodeIdLess
{
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
	{
		return lhs.ToPointer() < rhs.ToPointer();
	}
};

static const float          s_TouchTime = 1.0f;
static std::map<ed::NodeId, float, NodeIdLess> s_NodeTouchTime;

static int s_NextId = 1;
static int GetNextId()
{
	return s_NextId++;
}

void setNextId(int idIncrement)
{
	s_NextId = s_NextId + idIncrement;
}

static ed::NodeId GetNextNodeId()
{
	return ed::NodeId(GetNextId());
}

static ed::LinkId GetNextLinkId()
{
	return ed::LinkId(GetNextId());
}

static void TouchNode(ed::NodeId id)
{
	s_NodeTouchTime[id] = s_TouchTime;
}

static float GetTouchProgress(ed::NodeId id)
{
	auto it = s_NodeTouchTime.find(id);
	if (it != s_NodeTouchTime.end() && it->second > 0.0f)
		return (s_TouchTime - it->second) / s_TouchTime;
	else
		return 0.0f;
}

static void UpdateTouch()
{
	const auto deltaTime = ImGui::GetIO().DeltaTime;
	for (auto& entry : s_NodeTouchTime)
	{
		if (entry.second > 0.0f)
			entry.second -= deltaTime;
	}
}

static Node* FindNode(ed::NodeId id)
{
	for (auto& node : s_Nodes)
		if (node.ID == id)
			return &node;

	return nullptr;
}

static Link* FindLink(ed::LinkId id)
{
	for (auto& link : s_Links)
		if (link.ID == id)
			return &link;

	return nullptr;
}

static Pin* FindPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.Inputs)
			if (pin.ID == id)
				return &pin;

		for (auto& pin : node.Outputs)
			if (pin.ID == id)
				return &pin;
	}

	return nullptr;
}

static bool IsPinLinked(ed::PinId id)
{
	if (!id)
		return false;

	for (auto& link : s_Links)
		if (link.StartPinID == id || link.EndPinID == id)
			return true;

	return false;
}

static bool CanCreateLink(Pin* a, Pin* b)
{
	if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
		return false;

	return true;
}

static void BuildNode(Node* node)
{
	for (auto& input : node->Inputs)
	{
		input.Node = node;
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs)
	{
		output.Node = node;
		output.Kind = PinKind::Output;
	}
}

static Node* SpawnComment()
{
	s_Nodes.emplace_back(GetNextId(), "Test Comment");
	s_Nodes.back().Type = NodeType::Comment;
	s_Nodes.back().Size = ImVec2(300, 200);

	return &s_Nodes.back();
}

static Node* SpawnRoomNode()
{
	roomAdded = true;

	s_Nodes.emplace_back(GetNextId(), "T", ImColor(128, 195, 248));
	s_Nodes.back().Type = NodeType::Simple;
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

	s_Nodes.back().Inputs.emplace_back(GetNextId(), "L", PinType::KeyLock);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "K", PinType::KeyLock);
	
	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnStartNode()
{
	startAdded = true;
	s_Nodes.emplace_back(GetNextId(), "Start", ImColor(128, 195, 248));
	s_Nodes.back().Type = NodeType::Simple;
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnEndNode()
{
	endAdded = true;
	s_Nodes.emplace_back(GetNextId(), "End", ImColor(128, 195, 248));
	s_Nodes.back().Type = NodeType::Simple;
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnTreeSequenceNode()
{
	s_Nodes.emplace_back(GetNextId(), "Key/Lock");
	s_Nodes.back().Type = NodeType::Tree;
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::KeyLock);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::KeyLock);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static void ShowHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

struct AppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;
	bool				showGraph;
	bool				showGenStrat;
	bool				rulesLoaded;

	void    Clear() { Buf.clear(); LineOffsets.clear(); }

	void	ShowGraph() { Buf.clear(); LineOffsets.clear(); showGraph = true; }
	void	ShowGenStrat() { Buf.clear(); LineOffsets.clear(); showGenStrat = true; }
	void	ShowRulesLoaded() { Buf.clear(); LineOffsets.clear(); rulesLoaded = true; }

	void    AddLog(const char* fmt, ...) IM_PRINTFARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
		ImGui::Begin(title, p_open);
		if (ImGui::Button("Show Graph")) ShowGraph();					ImGui::SameLine();
		if (ImGui::Button("Show Generation Strategy")) ShowGenStrat();	
		if (ImGui::Button("Show Rules Added")) ShowRulesLoaded();		ImGui::SameLine();

		if (ImGui::Button("Clear")) Clear();							ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy) ImGui::LogToClipboard();

		if (Filter.IsActive())
		{
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != NULL; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
				if (Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : NULL;
			}
		}
		else
		{
			ImGui::TextUnformatted(Buf.begin());
		}

		if (ScrollToBottom)
			ImGui::SetScrollHere(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};

static void ShowAppLog(bool* p_open)
{
	static AppLog log;
	graphSys::Graph g_Copy = generatedGraph;
	//MetaTest
	//Print graph to log
	if (log.showGraph)
	{
		ids = generatedGraph.getIds();
		std::vector<std::string> eList;

		for (int i = 0; i < s_Links.size(); i++)
		{
			std::string e;
			e = std::to_string(s_Links.at(i).StartPinID.Get() - 2) + " --- " + std::to_string(s_Links.at(i).EndPinID.Get() - 2);
				eList.push_back(e);
		}

		log.AddLog("Graph:\n");
		for (int i = 0; i < eList.size(); i++)
		{
			std::string edge = eList.at(i);
			log.AddLog("%s \n", edge.c_str());
		}

		log.showGraph = false;
	}

	if (log.showGenStrat)
	{
		for (int i = 0; i < g_Copy.getGeneratedRules().size(); i++)
		{
			//Log rule
			log.AddLog("Rule Applied: %s\n", g_Copy.getGeneratedRules().at(i).c_str());
		}
		log.showGenStrat = false;
	}
	
	if (log.rulesLoaded)
	{
		MetaTest MT;
		MT.PrintMetaTest();
		for (int i = 0; i < MT.strings.size(); i++)
		{
			log.AddLog("MetaTest: %s\n", MT.strings.at(i).c_str());
		}
		log.rulesLoaded = false;
	}

	log.Draw("DunJenny: Log", p_open);
}

void BuildNodes()
{
	for (auto& node : s_Nodes)
		BuildNode(&node);
}

void setGraphMinMaxSize(int min, int max, graphSys::Graph G)
{
	G.setTargetSizeMin(min);
	G.setTargetSizeMax(max);
}

void GenerateMap()
{
	ids.clear();
	gb.getGraph().setIds(ids);

	for (int i = 0; i < s_Links.size(); i++)
	{
		auto link = FindLink(s_Links.at(i).ID);
		ed::DeleteLink(link->ID);
	}

	for (int i = 0; i < s_Nodes.size(); i++)
	{
		auto node = FindNode(s_Nodes.at(i).ID);
		ed::DeleteNode(node->ID);
	}

	bool t = true;
	ed::ClearScreen(t);

	ed::BeginDelete();

	ed::LinkId linkId = 0;
	while (ed::QueryDeletedLink(&linkId))
	{
		if (ed::AcceptDeletedItem())
		{
			auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
			if (id != s_Links.end())
				s_Links.erase(id);
		}
	}

	ed::NodeId nodeId = 0;
	while (ed::QueryDeletedNode(&nodeId))
	{
		if (ed::AcceptDeletedItem())
		{
			auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
			if (id != s_Nodes.end())
				s_Nodes.erase(id);
		}
	}

	t = false;
	ed::ClearScreen(t);
	ed::EndDelete();

	graphSys::Graph G = gb.getGraph();
	int nextId = G.getNextNodeId();
	rf = gb.getRF();
	G.setTargetSizeMin(graphMin);
	G.setTargetSizeMax(graphMax);

	G.setMaxIter(maxIters);

	G.setTargetXDistMax(xMaxDist);
	G.setTargetYDistMax(yMaxDist);
	G.setTargetXDistMin(xMinDist);
	G.setTargetYDistMin(yMinDist);

	Node* node;

	if (roomAdded)
	{
		graphSys::Node newNode(GetNextId(), ' ', "room");
		G.addNode(newNode);
		nextId++;
	}

	if (startAdded)
	{
		graphSys::Node startNode(GetNextId(), 's', "start");
		G.addNode(startNode);
		nextId++;
	}

	if (endAdded)
	{
		graphSys::Node endNode(GetNextId(), 'x', "end");
		G.addNode(endNode);
		nextId++;
	}

	roomAdded = false;
	startAdded = false;
	endAdded = false;

	G.setName("Default Name");
	G = gb.onInit(rf.getRules(), G);
	generatedGraph = G;

	if (G.getName() != "FAIL")
	{
		//Generate room nodes & update Ids to match node editor
		for (int i = 0; i < G.getGraphNodes().size(); i++)
		{
			if (G.getGraphNodes().at(i).getType() == "start")
			{
				node = SpawnStartNode();
				ed::SetNodePosition(node->ID, ImVec2(G.getGraphNodes().at(i).getXPos(), G.getGraphNodes().at(i).getYPos()));

				int oldId = G.getGraphNodes().at(i).getID();
				int newId = node->ID.Get();
				ids.push_back(std::pair<int, int>(oldId, newId));
			}
			if (G.getGraphNodes().at(i).getType() == "room")
			{
				node = SpawnRoomNode();
				ed::SetNodePosition(node->ID, ImVec2(G.getGraphNodes().at(i).getXPos(), G.getGraphNodes().at(i).getYPos()));

				int oldId = G.getGraphNodes().at(i).getID();
				int newId = node->ID.Get();
				ids.push_back(std::pair<int, int>(oldId, newId));

			}
			if (G.getGraphNodes().at(i).getType() == "end")
			{
				node = SpawnEndNode();
				ed::SetNodePosition(node->ID, ImVec2(G.getGraphNodes().at(i).getXPos(), G.getGraphNodes().at(i).getYPos()));

				int oldId = G.getGraphNodes().at(i).getID();
				int newId = node->ID.Get();
				ids.push_back(std::pair<int, int>(oldId, newId));
			}
		}

		G.setIds(ids);

		ed::NavigateToContent();

		BuildNodes();

		//Create node links
		for (int i = 0; i < G.getGraphEdges().size(); i++)
		{
			int edgeSID = G.getGraphEdges().at(i).getSrc().getID();
			int edgeTID = G.getGraphEdges().at(i).getTarget().getID();
			int* srcID;
			int* trgID;
			int Splace = 0;
			int Tplace = 0;

			if (G.getGraphEdges().size() > 0)
			{
				ids = G.getIds();
				bool source = false;
				bool target = false;

				auto nodeSrc = std::find_if(ids.begin(), ids.end(), [&](auto& ids)
				{
					if (ids.first == edgeSID)
						source = true;
					return ids.first == edgeSID;
				});
				auto nodeTrg = std::find_if(ids.begin(), ids.end(), [&](auto& ids)
				{
					if (ids.first == edgeTID)
						target = true;
					return ids.first == edgeTID;
				});

				if (source)
					srcID = &nodeSrc->second;
				if (target)
					trgID = &nodeTrg->second;

				for (int j = 0; j < s_Nodes.size(); j++)
				{
					if (source && s_Nodes.at(j).ID.Get() == *srcID)
						Splace = j;
					if (target && s_Nodes.at(j).ID.Get() == *trgID)
						Tplace = j;
				}
			}

			s_Links.push_back(Link(GetNextLinkId(), s_Nodes[Splace].Outputs[0].ID, s_Nodes[Tplace].Inputs[0].ID));
		}

		//Link start node to graph
		if (s_Nodes.size() > 0)
			s_Links.push_back(Link(GetNextLinkId(), s_Nodes[s_Nodes.size() - 2].Outputs[0].ID, s_Nodes[1].Inputs[0].ID));

		generatedGraph = G;

		//Delete initial node from s_Nodes
		if (s_Nodes.size() > 0)
		{
			auto initialNode = FindNode(s_Nodes.at(0).ID);
			ed::DeleteNode(initialNode->ID);
			ed::NodeId nodeId = node->ID;
			while (ed::QueryDeletedNode(&nodeId))
			{
				if (ed::AcceptDeletedItem())
				{
					auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& initialNode) { return initialNode.ID == nodeId; });
					if (id != s_Nodes.end())
						s_Nodes.erase(id);
				}
			}
		}
	}
	else
	{
		genFailed = true;
	}
}

void ClearMap()
{
	ids.clear();
	gb.getGraph().setIds(ids);
	gb.newGraph();

	for (int i = 0; i < s_Links.size(); i++)
	{
		auto link = FindLink(s_Links.at(i).ID);
		ed::DeleteLink(link->ID);
	}
	
	for (int i = 0; i < s_Nodes.size(); i++)
	{
		auto node = FindNode(s_Nodes.at(i).ID);
		ed::DeleteNode(node->ID);
	}

	bool t = true;
	ed::ClearScreen(t);

	ed::BeginDelete();
	
	ed::LinkId linkId = 0;
	while (ed::QueryDeletedLink(&linkId))
	{
		if (ed::AcceptDeletedItem())
		{
			auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
			if (id != s_Links.end())
				s_Links.erase(id);
		}
	}
	
	ed::NodeId nodeId = 0;
	while (ed::QueryDeletedNode(&nodeId))
	{
		if (ed::AcceptDeletedItem())
		{
			auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
			if (id != s_Nodes.end())
				s_Nodes.erase(id);
		}
	}

	t = false;
	ed::ClearScreen(t);
	ed::EndDelete();
}

const char* activeEditorFilePath = "";

void LoadMap()
{
	ed::Config config;
	config.SettingsFile = "Simple.json";
	config.LoadNodeSettings	= [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
	{
		auto node = FindNode(nodeId);
		if (!node)
			return 0;

		if (data != nullptr)
			memcpy(data, node->State.data(), node->State.size());
		return node->State.size();
	};
	
	g_Context = ed::CreateEditor(&config);
	ed::SetCurrentEditor(g_Context);
	//config.LoadNodeSettings();
}	
	
void Application_Initialize()
{	
	ed::Config config;
	config.SettingsFile = "Simple.json";
	
	config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
	{
		auto node = FindNode(nodeId);
		if (!node)
			return 0;
	
		if (data != nullptr)
			memcpy(data, node->State.data(), node->State.size());
		return node->State.size();
	};
	
	config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
	{
		auto node = FindNode(nodeId);
		if (!node)
			return false;
	
		node->State.assign(data, size);
	
		TouchNode(nodeId);
	
		return true;
	};

	g_Context = ed::CreateEditor(&config);
	ed::SetCurrentEditor(g_Context);

	srand(time(NULL));

	s_HeaderBackground = Application_LoadTexture("Data/BlueprintBackground.png");
	s_SaveIcon = Application_LoadTexture("Data/ic_save_white_24dp.png");
	s_RestoreIcon = Application_LoadTexture("Data/ic_restore_white_24dp.png");
}

void Application_Finalize()
{
	auto releaseTexture = [](ImTextureID& id)
	{
		if (id)
		{
			Application_DestroyTexture(id);
			id = nullptr;
		}
	};

	releaseTexture(s_RestoreIcon);
	releaseTexture(s_SaveIcon);
	releaseTexture(s_HeaderBackground);

	if (g_Context)
	{
		ed::DestroyEditor(g_Context);
		g_Context = nullptr;
	}
}

static void DrawSplitter(int split_vertically, float thickness, float* size0, float* size1, float min_size0, float min_size1)
{
	ImVec2 backup_pos = ImGui::GetCursorPos();
	if (split_vertically)
		ImGui::SetCursorPosY(backup_pos.y + *size0);
	else
		ImGui::SetCursorPosX(backup_pos.x + *size0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));          // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.10f));
	ImGui::Button("##Splitter", ImVec2(!split_vertically ? thickness : -1.0f, split_vertically ? thickness : -1.0f));
	ImGui::PopStyleColor(3);

	ImGui::SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter.

	if (ImGui::IsItemActive())
	{
		float mouse_delta = split_vertically ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

		// Minimum pane size
		if (mouse_delta < min_size0 - *size0)
			mouse_delta = min_size0 - *size0;
		if (mouse_delta > *size1 - min_size1)
			mouse_delta = *size1 - min_size1;

		// Apply resize
		*size0 += mouse_delta;
		*size1 -= mouse_delta;
	}
	ImGui::SetCursorPos(backup_pos);
}

ImColor GetIconColor(PinType type)
{
	switch (type)
	{
	default:
	case PinType::Flow:     return ImColor(255, 255, 255);
	case PinType::Key:		return ImColor(0, 255, 0);
	case PinType::Lock:		return ImColor(255, 0, 0);
	case PinType::KeyLock:	return ImColor(255, 0, 255);
	case PinType::Bool:     return ImColor(220, 48, 48);
	case PinType::Int:      return ImColor(68, 201, 156);
	case PinType::Float:    return ImColor(147, 226, 74);
	case PinType::Object:   return ImColor(51, 150, 215);
	case PinType::Function: return ImColor(218, 0, 183);
	case PinType::Delegate: return ImColor(255, 48, 48);
	}
};

void DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
	IconType iconType;
	ImColor  color = GetIconColor(pin.Type);
	color.Value.w = alpha / 255.0f;
	switch (pin.Type)
	{
	case PinType::Flow:     iconType = IconType::Flow;   break;
	case PinType::Key:		iconType = IconType::Diamond; break;
	case PinType::Lock:		iconType = IconType::Diamond; break;
	case PinType::KeyLock:	iconType = IconType::Diamond; break;
	case PinType::Bool:     iconType = IconType::Circle; break;
	case PinType::Int:      iconType = IconType::Circle; break;
	case PinType::Float:    iconType = IconType::Circle; break;
	case PinType::Object:   iconType = IconType::Circle; break;
	case PinType::Function: iconType = IconType::Circle; break;
	case PinType::Delegate: iconType = IconType::Square; break;
	default:
		return;
	}

	ax::Widgets::Icon(to_imvec(size(s_PinIconSize, s_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
};

void OpenRuleEditor(std::string rulePath)
{
	ImGui::SetNextWindowPos(ImVec2(418, 75));
	ImGui::Begin("Open Rule");

	// Dummy Json Text For Rule Display
	static char rule1[1024 * 4] = "{\n	'ruleid: ruleone,\n		{'components': \n		{\n			'leftside':\n			{\n			'nodes': 1,\n			'edges' :\n			}\n			'rightside':\n			{\n			'nodes': 2, 3, 4, 5, 6,\n			 'edges ': 2 - 3, 3 - 4, 4 - 5, 5 - 6\n			}\n		}\n	}\n}";
	static char rule2[1024 * 4] = "{\n	'ruleid: ruletwo,\n		{'components': \n		{\n			'leftside':\n			{\n			'nodes': 1,2\n			'edges' : 1-2\n			}\n			'rightside':\n			{\n			'nodes': 4, 5, 6\n			 'edges ': 4 - 5, 5 - 6\n			}\n		}\n	}\n}";
	static char rule3[1024 * 4] = "{\n	'ruleid: rulethree,\n		{'components': \n		{\n			'leftside':\n			{\n			'nodes': 1,2,3\n			'edges' : 1-2, 2-3\n			}\n			'rightside':\n			{\n			'nodes': 4, 5, 6, 7\n			 'edges ': 4 - 5, 5 - 6, 6 - 7\n			}\n		}\n	}\n}";
	
	if (rulePath == "ruleone.json")
	{
		ImGui::PushItemWidth(-1.0f);
		ImGui::InputTextMultiline("##source", rule1, (int)(sizeof(rule1) / sizeof(*rule1)), ImVec2(0.f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine);
		ImGui::PopItemWidth();
	}
	else if (rulePath == "ruletwo.json")
	{
		ImGui::PushItemWidth(-1.0f);
		ImGui::InputTextMultiline("##source", rule2, (int)(sizeof(rule2) / sizeof(*rule2)), ImVec2(0.f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine);
		ImGui::PopItemWidth();
	}
	else if (rulePath == "rulethree.json")
	{
		ImGui::PushItemWidth(-1.0f);
		ImGui::InputTextMultiline("##source", rule3, (int)(sizeof(rule2) / sizeof(*rule2)), ImVec2(0.f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine);
		ImGui::PopItemWidth();
	}

	if (ImGui::Button("Done", ImVec2(50, 25)))
	{
		openRuleEdit = false;
	}

	ImGui::End();
}

void OpenRuleBuilder()
{
	ImGui::SetNextWindowPos(ImVec2(514, 9));
	ImGui::Begin("Rule Builder");
	
	//Instructions modal popup
	if (ImGui::Button("Instructions"))
		ImGui::OpenPopup(" Instructions");

	bool open = true;
	ImGui::SetNextWindowPos(ImVec2(878, 9));
	if (ImGui::BeginPopupModal(" Instructions", &open))
	{
		ImGui::Text("Welcome to the Rule Builder!");
		ImGui::Text("Ensure to use ONLY single characters when nameing rules, nodes and edges!");
		ImGui::Text("First create the left side of the rule filling in ALL required info from top to bottom.\nEnsure to add all rule side nodes in one instance.");
		ImGui::Text("Then create the right side of the rule in the same manner.");
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	 
	//Rule builder vars
	static int selected_rule_side = -1;
	static int selected_room_type = -1;
	static int selected_node = -1;
	bool built = false;
	const char* names[] = { "room", "start", "end"/*, "boss room", "bonus Chest Room", "Health", "Trap" */};
	const char* sides[] = { "LEFT", "RIGHT" };
	
	//Rule Name text box
	static char name[32] = "Rule Name...";
	char buf[64]; sprintf(buf, "Rule Name: %s###Button", name); // ### operator override ID ignoring the preceding label
	ImGui::Button(buf);
	if (ImGui::BeginPopupContextItem(buf))
	{
		ImGui::Text("Edit name:");
		if (ImGui::InputText("##edit", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			gb.ruleName = *name;
			gb.initRule(name);
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Close"))
		{
			gb.ruleName = *name;
			gb.initRule(name);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine(); ImGui::Text("(<-- right-click here)");

	//Rule Side Selector --------------------------------------------------------------------
	if (ImGui::Button("Rule Side"))
	{
		if (name != "Rule Name...")
		{
			ImGui::OpenPopup("sideSelect");
		}
		else
			errorMsg = true;
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(selected_rule_side == -1 ? "<None>" : sides[selected_rule_side]);
	
	if (ImGui::BeginPopup("sideSelect"))
	{
		ImGui::Text("Rule Side");
		ImGui::Separator();
		for (int i = 0; i < IM_ARRAYSIZE(sides); i++)
			if (ImGui::Selectable(sides[i]))
				selected_rule_side = i;
		ImGui::EndPopup();
	}

	//Room type selection -----------------------------------------------------------------------
	if (ImGui::Button("Room Type.."))
	{
		if (name != "Rule Name..." && selected_rule_side != -1)
		{
			ImGui::OpenPopup("select");
		}
		else
			errorMsg = true;
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(selected_room_type == -1 ? "<None>" : names[selected_room_type]);
	if (ImGui::BeginPopup("select"))
	{
		ImGui::Text("Room Type");
		ImGui::Separator();
		for (int i = 0; i < IM_ARRAYSIZE(names); i++)
			if (ImGui::Selectable(names[i]))
				selected_room_type = i;
		ImGui::EndPopup();
	}

	//Add new Node -----------------------------------------------------------------------------
	if (ImGui::Button("Add Node"))
	{
		if (name != "Rule Name...")
		{
			ImGui::OpenPopup(" Node Builder");
		}
		else
			errorMsg = true;
	}
	
	ImGui::SetNextWindowSize(ImVec2(300, 100));
	rf = gb.getRF();
	graphSys::Node n;
	graphSys::Edge e;
	if (ImGui::BeginPopupModal(" Node Builder", &open))
	{
		char name[32] = "Node Name..";
		ImGui::Text("Edit name:");
		
		if (ImGui::InputText("##edit", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			gb.editedName = name;

			n = gb.addNewNode(*gb.editedName, GetNextId(), names[selected_room_type]);
			if (selected_rule_side == 0)
				newRule.setLeft(n);
			if (selected_rule_side == 1)
				newRule.setRight(n);
		}

		ImGui::SameLine();

		if (ImGui::Button("Close"))
		{
			rf.updateRule(rf.ruleAtId(gb.ruleName), newRule, selected_rule_side);
			gb.setRF(rf);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	//Add New Edge -----------------------------------------------------------------------------
	if (ImGui::Button("Add Edge"))
	{
		if (name != "Rule Name..." && selected_rule_side != -1 && selected_room_type != -1)
		{
			ImGui::OpenPopup(" Edge Builder");
		}
		else
			errorMsg = true;
	}
	//
	if (ImGui::BeginPopupModal(" Edge Builder", &open))
	{
		if (selected_rule_side == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 190, 0, 200));
			if (ImGui::Button("Source Node"))
				ImGui::OpenPopup(" Source");
			if (ImGui::BeginPopupModal(" Source"))
			{
				int leftSize = rf.ruleAtId(gb.ruleName).getLeft().nodes.size();

				for (int i = 0; i < leftSize; i++)
				{
					std::stringstream ss;
					std::string lb;
					char old = rf.ruleAtId(gb.ruleName).getLeft().nodes.at(i).getLabel();
					ss << old;
					ss >> lb;

					if (i % 2 == 0)
						ImGui::SameLine();

					if (ImGui::Button(lb.c_str()))
					{
						gb.sidToGet = rf.getRules().back().getLeft().nodes.at(i);

						ImGui::CloseCurrentPopup();
					}
				}
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(190, 0, 0, 200));
			ImGui::SameLine();
			if (ImGui::Button("Target Node"))
				ImGui::OpenPopup(" Target");
			if (ImGui::BeginPopupModal(" Target"))
			{
				int leftSize = rf.ruleAtId(gb.ruleName).getLeft().nodes.size();

				for (int i = 0; i < leftSize; i++)
				{
					std::stringstream ss;
					std::string lb;
					char old = rf.ruleAtId(gb.ruleName).getLeft().nodes.at(i).getLabel();
					ss << old;
					ss >> lb;
					
					if (i % 2 == 0)
						ImGui::SameLine();

					if (ImGui::Button(lb.c_str()))
					{
						gb.tidToGet = rf.getRules().back().getLeft().nodes.at(i);

						ImGui::CloseCurrentPopup();
					}
				}
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();
		}
		else if(selected_rule_side == 1)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 190, 0, 200));
			if (ImGui::Button("Source Node"))
				ImGui::OpenPopup(" Source");
			if (ImGui::BeginPopupModal(" Source"))
			{
				int rightSize = rf.ruleAtId(gb.ruleName).getRight().nodes.size();

				for (int i = 0; i < rightSize; i++)
				{
					std::stringstream ss;
					std::string lb;
					char old = rf.ruleAtId(gb.ruleName).getRight().nodes.at(i).getLabel();
					ss << old;
					ss >> lb;


					if (i % 2 == 0)
						ImGui::SameLine();

					if (ImGui::Button(lb.c_str()))
					{
						gb.sidToGet = rf.getRules().back().getRight().nodes.at(i);

						ImGui::CloseCurrentPopup();
					}
				}
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(190, 0, 0, 200));
			if (ImGui::Button("Target Node"))
				ImGui::OpenPopup(" Target");
			if (ImGui::BeginPopupModal(" Target"))
			{
				int rightSize = rf.ruleAtId(gb.ruleName).getRight().nodes.size();

				for (int i = 0; i < rightSize; i++)
				{
					std::string label = std::to_string(rf.ruleAtId(gb.ruleName).getRight().nodes.at(i).getID());
					std::stringstream ss;
					std::string lb;
					ss << rf.ruleAtId(gb.ruleName).getRight().nodes.at(i).getLabel();
					ss >> lb;

					if (i % 2 == 0)
						ImGui::SameLine();

					if (ImGui::Button(lb.c_str()))
					{
						gb.tidToGet = rf.getRules().back().getRight().nodes.at(i);
						
						ImGui::CloseCurrentPopup();
					}
				}
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();
		}
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 190, 200));
		if (ImGui::Button("Add Edge"))
		{
			if (selected_rule_side == 0)
			{
				e = gb.addEdge(rf.ruleAtId(rf.ruleAtId(gb.ruleName).getID()), 0, gb.sidToGet, gb.tidToGet);
				e.setAutoID(true);
				newRule.addLeftEdge(e);
			}
			else if (selected_rule_side == 1)
			{
				e = gb.addEdge(rf.ruleAtId(rf.ruleAtId(gb.ruleName).getID()), 0, gb.sidToGet, gb.tidToGet);
				e.setAutoID(true);
				newRule.addRightEdge(e);
			}
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			rf.updateRule(rf.ruleAtId(gb.ruleName), newRule, selected_rule_side);
			gb.setRF(rf);
			newRule.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();
	ImGui::Text("Info");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Take care when setting edge source and targets to prevent unusable rules.");

	if (ImGui::Button("Create Rule"))
	{		
		if (name != "Rule Name..." && selected_rule_side != -1 && selected_room_type != -1)
		{
			//Rule is already created and filled, clear values for next rule build and close popup
			gb.nodes.clear();
			gb.edges.clear();

			gb.left.nodes.clear();
			gb.left.edges.clear();
			gb.right.nodes.clear();
			gb.right.edges.clear();

			gb.ruleName.clear();
			gb.editedName = " ";

			rf.setRules(rf.getRules());
		}
		else
			errorMsg = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Done", ImVec2(50, 25)))
	{
		openRuleBuilder = false;
	}
	ImGui::End();
}

void OpenDataViewer(graphSys::Graph G)
{
	ImGui::SetNextWindowPos(ImVec2(880, 9));
	ImGui::SetNextWindowSize(ImVec2(820, 1000));
	ImGui::Begin("Data Viewer");

	ImGui::Columns(7, "datacolumns"); // 4-ways, with border
	ImGui::Separator();

	ImGui::Text("Graph #"); ImGui::NextColumn();
	ImGui::Text("Rule Set"); ImGui::NextColumn();
	ImGui::Text("Time"); ImGui::NextColumn();
	ImGui::Text("Iters"); ImGui::NextColumn();
	ImGui::Text("Success"); ImGui::NextColumn();
	ImGui::Text("Size"); ImGui::NextColumn();
	ImGui::Text("Constraints"); ImGui::NextColumn();
	ImGui::Separator();

	for (int i = 0; i < gb.getGraphUpdates().size(); i++)
	{
		//Convert variables into chars to use with imgui::text
		long long genTimes = gb.graphGenTime.at(i);
		int gt = (int)genTimes;
		std::string gts = std::to_string(gt);
		const char* t = gts.c_str();

		int it = gb.iterations.at(i);
		std::string its = std::to_string(it);
		const char* s = its.c_str();

		const char* cons = "False";

		if (gb.constraintsMet.at(i))
			cons = "True";

		int gsz = gb.sizes.at(i);
		std::string szs = std::to_string(gsz);
		const char* sz = szs.c_str();

		//Set graph num to start from 1
		std::string n = std::to_string(i + 1);
		const char* num = n.c_str();

		ImGui::Text(num); ImGui::NextColumn();
		ImGui::Text("1,2,3"); ImGui::NextColumn();
		ImGui::Text(t); ImGui::NextColumn();
		ImGui::Text(s); ImGui::NextColumn();
		ImGui::Text(cons); ImGui::NextColumn();
		ImGui::Text(sz); ImGui::NextColumn();
		ImGui::Text("100 | 20/50 | -1k/1k/-1k/1k "); ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::Separator();
	
	
	if (ImGui::Button("Done", ImVec2(50, 25)))
	{
		openDataView = false;
	}
	ImGui::End();
}

void ShowLeftPane(float paneWidth)
{
	auto& io = ImGui::GetIO();
	ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

	paneWidth = ImGui::GetContentRegionAvailWidth();

	//DunJenny Title
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("DunJenny");

	//Rules Title
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Available Rules");

	ImGui::Indent();
	if (gb.getRF().getRules().size() > 0)
	{
		for (int i = 0; i < gb.getRF().getRules().size(); i++)
		{
			ImGui::Text(gb.getRF().getRules().at(i).getID().c_str());

			if (ImGui::IsItemClicked() && gb.getRF().getRules().size() > 0)
			{
				openRuleEdit = true;
				ruleToView = i;
			}
		}
	}
	ImGui::Unindent();

	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;
	selectedNodes.resize(ed::GetSelectedObjectCount());
	selectedLinks.resize(ed::GetSelectedObjectCount());
	
	int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
	int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));
	
	selectedNodes.resize(nodeCount);
	selectedLinks.resize(linkCount);
	
	int saveIconWidth = Application_GetTextureWidth(s_SaveIcon);
	int saveIconHeight = Application_GetTextureWidth(s_SaveIcon);
	int restoreIconWidth = Application_GetTextureWidth(s_RestoreIcon);
	int restoreIconHeight = Application_GetTextureWidth(s_RestoreIcon);

	ImGui::Spacing(); ImGui::SameLine();
	if (ImGui::CollapsingHeader("Generated Nodes"))
	{
		for (auto& node : s_Nodes)
		{
			ImGui::PushID(node.ID.ToPointer());
			auto start = ImGui::GetCursorScreenPos();

			if (const auto progress = GetTouchProgress(node.ID))
			{
				ImGui::GetWindowDrawList()->AddLine(
					start + ImVec2(-8, 0),
					start + ImVec2(-8, ImGui::GetTextLineHeight()),
					IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
			}

			bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();
			if (ImGui::Selectable((node.Name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.ToPointer()))).c_str(), &isSelected))
			{
				if (io.KeyCtrl)
				{
					if (isSelected)
						ed::SelectNode(node.ID, true);
					else
						ed::DeselectNode(node.ID);
				}
				else
					ed::SelectNode(node.ID, false);

				ed::NavigateToSelection();
			}
			if (ImGui::IsItemHovered() && !node.State.empty())
				ImGui::SetTooltip("State: %s", node.State.c_str());

			auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node.ID.ToPointer())) + ")";
			auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
			auto iconPanelPos = start + ImVec2(
				paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - saveIconWidth - restoreIconWidth - ImGui::GetStyle().ItemInnerSpacing.x * 1,
				(ImGui::GetTextLineHeight() - saveIconHeight) / 2);
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(iconPanelPos.x - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y),
				IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

			auto drawList = ImGui::GetWindowDrawList();
			ImGui::SetCursorScreenPos(iconPanelPos);
			ImGui::SetItemAllowOverlap();
			if (node.SavedState.empty())
			{
				if (ImGui::InvisibleButton("save", ImVec2((float)saveIconWidth, (float)saveIconHeight)))
					node.SavedState = node.State;

				if (ImGui::IsItemActive())
					drawList->AddImage(s_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
				else if (ImGui::IsItemHovered())
					drawList->AddImage(s_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
				else
					drawList->AddImage(s_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
			}
			else
			{
				ImGui::Dummy(ImVec2((float)saveIconWidth, (float)saveIconHeight));
				drawList->AddImage(s_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
			}

			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::SetItemAllowOverlap();
			if (!node.SavedState.empty())
			{
				if (ImGui::InvisibleButton("restore", ImVec2((float)restoreIconWidth, (float)restoreIconHeight)))
				{
					node.State = node.SavedState;
					ed::RestoreNodeState(node.ID);
					node.SavedState.clear();
				}

				if (ImGui::IsItemActive())
					drawList->AddImage(s_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
				else if (ImGui::IsItemHovered())
					drawList->AddImage(s_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
				else
					drawList->AddImage(s_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
			}
			else
			{
				ImGui::Dummy(ImVec2((float)restoreIconWidth, (float)restoreIconHeight));
				drawList->AddImage(s_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
			}

			ImGui::SameLine(0, 0);
			ImGui::SetItemAllowOverlap();
			ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));

			ImGui::PopID();
		}
	}

	static int changeCount = 0;

	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Selection");

	ImGui::BeginHorizontal("Selection Stats", ImVec2(paneWidth, 0));
	ImGui::Text("Changed %d time%s", changeCount, changeCount > 1 ? "s" : "");
	ImGui::Spring();
	if (ImGui::Button("Deselect All"))
		ed::ClearSelection();
	ImGui::EndHorizontal();
	ImGui::Indent();
	for (int i = 0; i < nodeCount; ++i) ImGui::Text("Node (%d)", selectedNodes[i]);
	for (int i = 0; i < linkCount; ++i) ImGui::Text("Link (%d)", selectedLinks[i]);
	ImGui::Unindent();

	ImGui::Separator();
	//Sliders -------------------------------------------------------------
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Generation Variables");
	//Max iterations Slider
	static int maxIT = 100;
	ImGui::DragInt("Iterations", &maxIT, 5, 1, 1000, "Max: %.0f");
	maxIters = maxIT;
	
	//Max & Min graph size slider
	static int tMin = 10;
	static int tMax = 50;
	ImGui::DragIntRange2("Target Graph Size", &tMin, &tMax, 1.0f, 1, 50, "Min: %.0f", "Max: %.0f");
	graphMin = tMin;
	graphMax = tMax;

	//Distance 
	static int dMinX = -1000;
	static int dMaxX = 1000;

	static int dMinY = -1000;
	static int dMaxY = 1000;

	ImGui::DragIntRange2("Target Graph X Distance", &dMinX, &dMaxX, 1.0f, -1000, 1000, "Min: %.0f", "Max: %.0f");
	ImGui::DragIntRange2("Target Graph Y Distance", &dMinY, &dMaxY, 1.0f, -1000, 1000, "Min: %.0f", "Max: %.0f");

	xMaxDist = dMaxX;
	xMinDist = dMinX;
	yMaxDist = dMaxY;
	yMinDist = dMinY;

	//Variable Display -------------------------------------------------------------
	//Graph size
	ImGui::Separator();
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Graph Data");

	ImGui::Text("Iterations to Generate: %d", generatedGraph.iteration);

	ImGui::Text("Graph Size: %d", s_Nodes.size());
	
	//Graph Max Dists
	ImGui::Text("Max Distance (x , y): %d , %d", generatedGraph.calcDistances().first, generatedGraph.calcDistances().second);

	//Directional flow shortcut
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
		for (auto& link : s_Links)
			ed::Flow(link.ID);

	if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)) && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Enter)))
	{
		//Map regen shortcut
		generatedGraph.clearGeneratedRules();
		GenerateMap();
	}

	if (ed::HasSelectionChanged())
		++changeCount;

	ImGui::EndChild();
}

void Application_Frame()
{
	UpdateTouch();
	auto& io = ImGui::GetIO();
	//Menu Code----------------------------------------------------------------------
	auto editorWindowSize = ImVec2(io.DisplaySize.x, 50);
	const char* activeEditorFilePath = "";
	//const char *mapName = activeEditorFilePath.empty() ? "Unnamed map" : activeEditorFilePath.c_str();

	ImGui::SetNextWindowPos(ImVec2(418, 9));
	ImGui::Begin("Menu", false, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Generate New map"))
		{
			ClearMap();
			generatedGraph.clearGeneratedRules();
			gb.setFirstLoad(true);
			GenerateMap();
			activeEditorFilePath = "";
		}
		if (ImGui::MenuItem("Load map.../obsolete/"))
		{
			//activeEditorFilePath = "Simple.json";
			//LoadMap();
			//showOpenDialog = true;
			//firstTimeOpenDialog = true;
		}
		if (ImGui::MenuItem("Save map/obsolete/"))
		{
			//showSaveDialog = activeEditorFilePath.empty();
			//if (!showSaveDialog)
			//	SaveMap(activeEditorFilePath.c_str());
			//else
			//	firstTimeSaveDialog = true;
		}
		if (ImGui::MenuItem("Save map as.../obsolete/"))
		{
			//showSaveDialog = firstTimeSaveDialog = true;
		}
		
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Tools"))
	{
		if (ImGui::MenuItem("Open Rule Builder"))
		{
			openRuleBuilder = true;
		}
		if (ImGui::MenuItem("Open Log"))
		{
			show_app_log = true;
		}
		if (ImGui::MenuItem("Regenerate Map"))
		{
			generatedGraph.clearGeneratedRules();
			GenerateMap();
		}
		if (ImGui::MenuItem("Open Data Viewer"))
		{
			openDataView = true;
		}

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	ImGui::End();

	//------------------------------------------------------------------------------- 

	ed::SetCurrentEditor(g_Context);

	static ed::NodeId contextNodeId = 0;
	static ed::LinkId contextLinkId = 0;
	static ed::PinId  contextPinId = 0;
	static bool createNewNode = false;
	static Pin* newNodeLinkPin = nullptr;
	static Pin* newLinkPin = nullptr;

	static float leftPaneWidth = 400.0f;
	static float rightPaneWidth = 800.0f;
	DrawSplitter(0, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

	ShowLeftPane(leftPaneWidth);

	ImGui::SameLine();

	//Main Editor Code --------------------------------------------------------------
	ed::Begin("Node editor");
	{
		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		util::BlueprintNodeBuilder builder(s_HeaderBackground, Application_GetTextureWidth(s_HeaderBackground), Application_GetTextureHeight(s_HeaderBackground));

		for (auto& node : s_Nodes)
		{
			if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
				continue;

			const auto isSimple = node.Type == NodeType::Simple;

			bool hasOutputDelegates = false;
			for (auto& output : node.Outputs)
				if (output.Type == PinType::Delegate)
					hasOutputDelegates = true;

			builder.Begin(node.ID);
			if (!isSimple)
			{
				builder.Header(node.Color);
				ImGui::Spring(0);
				ImGui::TextUnformatted(node.Name.c_str());
				ImGui::Spring(1);
				ImGui::Dummy(ImVec2(0, 28));
				if (hasOutputDelegates)
				{
					ImGui::BeginVertical("delegates", ImVec2(0, 28));
					ImGui::Spring(1, 0);
					for (auto& output : node.Outputs)
					{
						if (output.Type != PinType::Delegate)
							continue;

						auto alpha = ImGui::GetStyle().Alpha;
						if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
							alpha = alpha * (48.0f / 255.0f);

						ed::BeginPin(output.ID, ed::PinKind::Output);
						ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
						ed::PinPivotSize(ImVec2(0, 0));
						ImGui::BeginHorizontal(output.ID.ToPointer());
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
						if (!output.Name.empty())
						{
							ImGui::TextUnformatted(output.Name.c_str());
							ImGui::Spring(0);
						}
						DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
						ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
						ImGui::EndHorizontal();
						ImGui::PopStyleVar();
						ed::EndPin();

					}
					ImGui::Spring(1, 0);
					ImGui::EndVertical();
					ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
				}
				else
					ImGui::Spring(0);
				builder.EndHeader();
			}
		
			for (auto& input : node.Inputs)
			{
				auto alpha = ImGui::GetStyle().Alpha;
				if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
					alpha = alpha * (48.0f / 255.0f);

				builder.Input(input.ID);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
				DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
				ImGui::Spring(0);
				if (!input.Name.empty())
				{
					ImGui::TextUnformatted(input.Name.c_str());
					ImGui::Spring(0);
				}
				if (input.Type == PinType::Bool)
				{
					ImGui::Button("Hello");
					ImGui::Spring(0);
				}				
				//Asseert random key on creation of KeyLock Pin
				if (input.Type == PinType::KeyLock)
				{
					ImGui::TextUnformatted("k", "L");
					ImGui::Spring(0);
				}

				ImGui::PopStyleVar();
				builder.EndInput();
			}

			if (isSimple)
			{
				builder.Middle();

				ImGui::Spring(1, 0);
				ImGui::TextUnformatted(node.Name.c_str());
				ImGui::Spring(1, 0);
			}

			for (auto& output : node.Outputs)
			{
				if (!isSimple && output.Type == PinType::Delegate)
					continue;

				auto alpha = ImGui::GetStyle().Alpha;
				if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
					alpha = alpha * (48.0f / 255.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
				builder.Output(output.ID);
				if (!output.Name.empty())
				{
					ImGui::Spring(0);
					ImGui::TextUnformatted(output.Name.c_str());
				}
				ImGui::Spring(0);
				DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
				ImGui::PopStyleVar();
				builder.EndOutput();
			}

			builder.End();
		}

		for (auto& node : s_Nodes)
		{
			if (node.Type != NodeType::Tree)
				continue;

			const float rounding = 10.0f;
			const float padding = 12.0f;

			const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

			ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
			ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
			ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

			ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
			ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
			ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
			ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
			ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
			ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
			ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
			ed::BeginNode(node.ID);

			ImGui::BeginVertical(node.ID.ToPointer());
			ImGui::BeginHorizontal("inputs");
			ImGui::Spring(0, padding * 2);

			rect inputsRect;
			int inputAlpha = 200;
			if (!node.Inputs.empty())
			{
				auto& pin = node.Inputs[0];
				ImGui::Dummy(ImVec2(0, padding));
				ImGui::Spring(1, 0);
				inputsRect = ImGui_GetItemRect();

				ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
				ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
				ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
				ed::BeginPin(pin.ID, ed::PinKind::Input);
				ed::PinPivotRect(to_imvec(inputsRect.top_left()), to_imvec(inputsRect.bottom_right()));
				ed::PinRect(to_imvec(inputsRect.top_left()), to_imvec(inputsRect.bottom_right()));
				ed::EndPin();
				ed::PopStyleVar(3);

				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
			}
			else
				ImGui::Dummy(ImVec2(0, padding));

			ImGui::Spring(0, padding * 2);
			ImGui::EndHorizontal();

			ImGui::BeginHorizontal("content_frame");
			ImGui::Spring(1, padding);

			ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
			ImGui::Dummy(ImVec2(160, 0));
			ImGui::Spring(1);
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::Spring(1);
			ImGui::EndVertical();
			auto contentRect = ImGui_GetItemRect();

			ImGui::Spring(1, padding);
			ImGui::EndHorizontal();

			ImGui::BeginHorizontal("outputs");
			ImGui::Spring(0, padding * 2);

			rect outputsRect;
			int outputAlpha = 200;
			if (!node.Outputs.empty())
			{
				auto& pin = node.Outputs[0];
				ImGui::Dummy(ImVec2(0, padding));
				ImGui::Spring(1, 0);
				outputsRect = ImGui_GetItemRect();

				ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
				ed::BeginPin(pin.ID, ed::PinKind::Output);
				ed::PinPivotRect(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()));
				ed::PinRect(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()));
				ed::EndPin();
				ed::PopStyleVar();

				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
			}
			else
				ImGui::Dummy(ImVec2(0, padding));

			ImGui::Spring(0, padding * 2);
			ImGui::EndHorizontal();

			ImGui::EndVertical();

			ed::EndNode();
			ed::PopStyleVar(7);
			ed::PopStyleColor(4);

			auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

			drawList->AddRectFilled(to_imvec(inputsRect.top_left()) + ImVec2(0, 1), to_imvec(inputsRect.bottom_right()),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
			ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
			drawList->AddRect(to_imvec(inputsRect.top_left()) + ImVec2(0, 1), to_imvec(inputsRect.bottom_right()),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
			ImGui::PopStyleVar();
			drawList->AddRectFilled(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()) - ImVec2(0, 1),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
			ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
			drawList->AddRect(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()) - ImVec2(0, 1),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
			ImGui::PopStyleVar();
			drawList->AddRectFilled(to_imvec(contentRect.top_left()), to_imvec(contentRect.bottom_right()), IM_COL32(24, 64, 128, 200), 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
			drawList->AddRect(
				to_imvec(contentRect.top_left()),
				to_imvec(contentRect.bottom_right()),
				IM_COL32(48, 128, 255, 100), 0.0f);
			ImGui::PopStyleVar();
		}

		for (auto& node : s_Nodes)
		{
			if (node.Type != NodeType::Comment)
				continue;

			const float commentAlpha = 0.5f;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
			ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
			ed::BeginNode(node.ID);
			ImGui::BeginVertical("content");
			ImGui::BeginHorizontal("horizontal");
			ImGui::Spring(1);
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::Spring(1);
			ImGui::EndHorizontal();
			ed::Group(node.Size);
			ImGui::EndVertical();
			ed::EndNode();
			ed::PopStyleColor(2);
			ImGui::PopStyleVar();

			if (ed::BeginGroupHint(node.ID))
			{
				auto alpha = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);

				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

				auto min = ed::GetGroupMin();
				//auto max = ed::GetGroupMax();

				ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
				ImGui::BeginGroup();
				ImGui::TextUnformatted(node.Name.c_str());
				ImGui::EndGroup();

				auto drawList = ed::GetHintBackgroundDrawList();

				auto hintBounds = ImGui_GetItemRect();
				auto hintFrameBounds = hintBounds.expanded(8, 4);

				drawList->AddRectFilled(
					to_imvec(hintFrameBounds.top_left()),
					to_imvec(hintFrameBounds.bottom_right()),
					IM_COL32(255, 255, 255, 64 * alpha / 255), 4.0f);

				drawList->AddRect(
					to_imvec(hintFrameBounds.top_left()),
					to_imvec(hintFrameBounds.bottom_right()),
					IM_COL32(255, 255, 255, 128 * alpha / 255), 4.0f);

				ImGui::PopStyleVar();
			}
			ed::EndGroupHint();
		}

		//Create links between nodes
		for (auto& link : s_Links)
			ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

		if (!createNewNode)
		{
			if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
			{
				auto showLabel = [](const char* label, ImColor color)
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
					auto size = ImGui::CalcTextSize(label);

					auto padding = ImGui::GetStyle().FramePadding;
					auto spacing = ImGui::GetStyle().ItemSpacing;

					ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

					auto rectMin = ImGui::GetCursorScreenPos() - padding;
					auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

					auto drawList = ImGui::GetWindowDrawList();
					drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
					ImGui::TextUnformatted(label);
				};

				ed::PinId startPinId = 0, endPinId = 0;
				if (ed::QueryNewLink(&startPinId, &endPinId))
				{
					auto startPin = FindPin(startPinId);
					auto endPin = FindPin(endPinId);

					newLinkPin = startPin ? startPin : endPin;

					if (startPin->Kind == PinKind::Input)
					{
						std::swap(startPin, endPin);
						std::swap(startPinId, endPinId);
					}

					if (startPin && endPin)
					{
						if (endPin == startPin)
						{
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->Kind == startPin->Kind)
						{
							showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->Type != startPin->Type)
						{
							showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
						}
						else
						{
							showLabel("+ Create Link", ImColor(32, 45, 32, 180));
							if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
							{
								s_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
								s_Links.back().Color = GetIconColor(startPin->Type);
							}
						}
					}
				}

				ed::PinId pinId = 0;
				if (ed::QueryNewNode(&pinId))
				{
					newLinkPin = FindPin(pinId);
					if (newLinkPin)
						showLabel("+ Create Node", ImColor(32, 45, 32, 180));

					if (ed::AcceptNewItem())
					{
						createNewNode = true;
						newNodeLinkPin = FindPin(pinId);
						newLinkPin = nullptr;
						ImGui::OpenPopup("Create New Node");
					}
				}
			}
			else
				newLinkPin = nullptr;

			ed::EndCreate();

			if (ed::BeginDelete())
			{
				ed::LinkId linkId = 0;
				while (ed::QueryDeletedLink(&linkId))
				{
					if (ed::AcceptDeletedItem())
					{
						auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
						if (id != s_Links.end())
							s_Links.erase(id);
					}
				}

				ed::NodeId nodeId = 0;
				while (ed::QueryDeletedNode(&nodeId))
				{
					if (ed::AcceptDeletedItem())
					{
						auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
						if (id != s_Nodes.end())
							s_Nodes.erase(id);
					}
				}
			}
			ed::EndDelete();
		}
		ImGui::SetCursorScreenPos(cursorTopLeft);
	}

	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId))
		ImGui::OpenPopup("Pin Context Menu");
	else if (ed::ShowLinkContextMenu(&contextLinkId))
		ImGui::OpenPopup("Link Context Menu");
	else if (ed::ShowBackgroundContextMenu())
	{
		ImGui::OpenPopup("Create New Node");
		newNodeLinkPin = nullptr;
	}

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Node Context Menu"))
	{
		auto node = FindNode(contextNodeId);

		ImGui::TextUnformatted("Node Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %d", node->ID);
			ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
		}
		else
			ImGui::Text("Unknown node: %d", contextNodeId);
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteNode(contextNodeId);
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Pin Context Menu"))
	{
		auto pin = FindPin(contextPinId);

		ImGui::TextUnformatted("Pin Context Menu");
		ImGui::Separator();
		if (pin)
		{
			ImGui::Text("ID: %d", pin->ID);
			if (pin->Node)
				ImGui::Text("Node: %d", pin->Node->ID);
			else
				ImGui::Text("Node: %s", "<none>");
		}
		else
			ImGui::Text("Unknown pin: %d", contextPinId);

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Link Context Menu"))
	{
		auto link = FindLink(contextLinkId);

		ImGui::TextUnformatted("Link Context Menu");
		ImGui::Separator();
		if (link)
		{
			ImGui::Text("ID: %d", link->ID);
			ImGui::Text("From: %d", link->StartPinID);
			ImGui::Text("To: %d", link->EndPinID);
		}
		else
			ImGui::Text("Unknown link: %d", contextLinkId);
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteLink(contextLinkId);
		ImGui::EndPopup();
	}

	//Right Click node adding menu within graph area
	if (ImGui::BeginPopup("Create New Node"))
	{
		auto newNodePostion = ImGui::GetMousePosOnOpeningCurrentPopup();
		
		//Node Adders for individual node types
		Node* node = nullptr;
		if (ImGui::MenuItem("Start"))
		{
			node = SpawnStartNode();
			graphSys::Node startNode(GetNextId(), 's', "start");
			gb.getGraph().addNode(startNode);
		}
		if (ImGui::MenuItem("End"))
		{
			graphSys::Node endNode(GetNextId(), 'x', "end");
			gb.getGraph().addNode(endNode);
			node = SpawnEndNode();
		}
		if (ImGui::MenuItem("Room"))
		{
			node = SpawnRoomNode();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Key/Lock"))
			node = SpawnTreeSequenceNode();

		ImGui::Separator();

		if (node)
		{
			createNewNode = false;

			ed::SetNodePosition(node->ID, newNodePostion);

			if (auto startPin = newNodeLinkPin)
			{
				auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

				for (auto& pin : pins)
				{
					if (CanCreateLink(startPin, &pin))
					{
						auto endPin = &pin;
						if (startPin->Kind == PinKind::Input)
							std::swap(startPin, endPin);

						s_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
						s_Links.back().Color = GetIconColor(startPin->Type);

						break;
					}
				}
			}
		}

		ImGui::EndPopup();
	}
	else
		createNewNode = false;

	if (openRuleEdit)
	{
		std::string rule;
		if(ruleToView == 0)
			rule = "ruleone.json";
		else if(ruleToView == 1)
			rule = "ruletwo.json";
		else if (ruleToView == 2)
			rule = "rulethree.json";

		OpenRuleEditor(rule);
	}

	if (openRuleBuilder)
		OpenRuleBuilder();

	if (openDataView)
		OpenDataViewer(generatedGraph);

	if (show_app_log)
	{
		ImGui::SetNextWindowPos(ImVec2(1200, 9));
		ShowAppLog(&show_app_log);
	}

	if (genFailed)
	{
		ImGui::OpenPopup(" Generation Failed!");
	}

	ImGui::SetNextWindowPos(ImVec2(420, 420));
	if (ImGui::BeginPopupModal(" Generation Failed!"))
	{
		ImGui::Text(" Graph Generation Failed! Exceeded Max Iterations! Try tweaking the min/max distance and size variables to generate a valid graph.");
		ImGui::SameLine();
		if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
		{
			genFailed = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (errorMsg)
	{
		ImGui::OpenPopup(" Error!");
	}

	if (ImGui::BeginPopupModal(" Error!", 0 , ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Error! Ensure all preceding rule variables are filled.");
		ImGui::SameLine();
		if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
		{
			errorMsg = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();

	//-----------------------------------------------------------------------------

	ed::Resume();
	ed::End();
}

