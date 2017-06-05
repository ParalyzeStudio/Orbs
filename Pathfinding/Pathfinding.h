#pragma once

struct DijkstraNode 
{
	DijkstraNode() : node(NULL), previousNode(NULL), weight(0), removed(false){};

	NodeSphere* node;
	DijkstraNode* previousNode;
	vector<DijkstraNode*> neighbouringNodes;
	float weight;
	bool removed;
};

class Pathfinding
{
public:
	static bool findPath(vector<NodeSphere*> allNodeSpheres, vector<NodeSphere*> &path, NodeSphere* startSphere, NodeSphere* endSphere, bool reorderPath);
	static bool dijkstra(vector<NodeSphere*> allNodes, vector<NodeSphere*> &dijkstraPath, NodeSphere* startNode, NodeSphere* endNode, bool reorderPath);
	static bool dijkstraNodesVectorIsEmpty(const vector<DijkstraNode*> &dijkstraNodes);
	static DijkstraNode* nodeWithMinWeight(const vector<DijkstraNode*> &dijkstraNodes);
	static DijkstraNode* dijkstraNodeForNode(vector<DijkstraNode*> &dijkstraNodes, NodeSphere* nodeSphere);
	static float distanceBetweenDijkstraNodes(DijkstraNode* nodeA, DijkstraNode* nodeB);
};

