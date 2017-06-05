#include "OrbsPluginPCH.h"
#include "Pathfinding.h"
#include "GameManager.h"

bool Pathfinding::findPath(vector<NodeSphere*> allNodeSpheres, vector<NodeSphere*> &path, NodeSphere* startSphere, NodeSphere* endSphere, bool reorderPath)
{
	return dijkstra(allNodeSpheres, path, startSphere, endSphere, reorderPath);
}

bool Pathfinding::dijkstra(vector<NodeSphere*> allNodes, vector<NodeSphere*> &dijkstraPath, NodeSphere* startNode, NodeSphere* endNode, bool reorderPath)
{
	//Create the dijsktra nodes with infinite weight at start except the first one
	int dijkstraNodesCount = (int) allNodes.size();
	vector<DijkstraNode*> dijkstraNodes;
	dijkstraNodes.reserve(dijkstraNodesCount);
	for (int allNodesIndex = 0; allNodesIndex != dijkstraNodesCount; allNodesIndex++)
	{
		NodeSphere* node = allNodes[allNodesIndex];
		DijkstraNode* dijkstraNode = new DijkstraNode();
		dijkstraNode->node = node;
		dijkstraNode->previousNode = NULL;
		dijkstraNode->removed = false;
		dijkstraNode->weight = (node == startNode) ? 0.0f : (float) INT_MAX;
		dijkstraNodes.push_back(dijkstraNode);
	}

	//Duplicate the neighbouring nodes
	for (int allNodesIndex = 0; allNodesIndex != dijkstraNodesCount; allNodesIndex++)
	{
		NodeSphere* pNodeSphere = allNodes[allNodesIndex];
		vector<NodeSphere*> neighbouringNodes = pNodeSphere->getNeighbouringSpheres();
		vector<DijkstraNode*> neighbouringDijkstraNodes;
		neighbouringDijkstraNodes.reserve(neighbouringNodes.size());
		for (int neighbouringNodesIndex = 0; neighbouringNodesIndex != neighbouringNodes.size(); neighbouringNodesIndex++)
		{
			NodeSphere* pNeighbouringSphere = neighbouringNodes[neighbouringNodesIndex];
			Bridge* pBridge = GameManager::GlobalManager().findBridge(pNodeSphere, pNeighbouringSphere);
			if (pBridge && pBridge->isRevealed())
				neighbouringDijkstraNodes.push_back(dijkstraNodeForNode(dijkstraNodes, neighbouringNodes[neighbouringNodesIndex]));
		}
		dijkstraNodes[allNodesIndex]->neighbouringNodes = neighbouringDijkstraNodes;
	}

	//Loop over the nodes until this vector is empty
	int index = 0;
	while (!dijkstraNodesVectorIsEmpty(dijkstraNodes))
	{
		DijkstraNode* minWeightNode = nodeWithMinWeight(dijkstraNodes);
		if (!minWeightNode) // one or more spheres are disconnected from the main network
			break;
		minWeightNode->removed = true;
		vector<DijkstraNode*> neighbouringDjikstraNodes = minWeightNode->neighbouringNodes;
		for (int neighbouringDjikstraNodesIndex = 0; neighbouringDjikstraNodesIndex != neighbouringDjikstraNodes.size(); neighbouringDjikstraNodesIndex++)
		{
			DijkstraNode* neighbouringDijkstraNode = neighbouringDjikstraNodes[neighbouringDjikstraNodesIndex];
			float newNodeWeight = minWeightNode->weight + distanceBetweenDijkstraNodes(minWeightNode, neighbouringDijkstraNode);
			if (neighbouringDijkstraNode->weight > newNodeWeight)
			{
				neighbouringDijkstraNode->weight = newNodeWeight;
				neighbouringDijkstraNode->previousNode = minWeightNode;
			}
		}
		index++;
	}

	//Rebuild the path with stored 'previous' nodes
	vector<NodeSphere*> path;
	DijkstraNode* startDijkstraNode = dijkstraNodeForNode(dijkstraNodes, startNode);
	DijkstraNode* endDijkstraNode = dijkstraNodeForNode(dijkstraNodes, endNode);
	DijkstraNode* pathNode = endDijkstraNode;
	path.push_back(pathNode->node);
	while (pathNode != startDijkstraNode)
	{
		if (!pathNode->previousNode) //isolated node sphere, break the process
			return false;
		path.push_back(pathNode->previousNode->node);
		pathNode = pathNode->previousNode;
	}

	if (path.size() == 0)
		return false;

	if (reorderPath)
	{
		vector<NodeSphere*> reversedArray(path.rbegin(), path.rend());
		path = reversedArray;
	}

	//Clear the dijkstraNodes array
	for (int dijkstraNodesIndex = 0; dijkstraNodesIndex != dijkstraNodesCount; dijkstraNodesIndex++)
	{
		DijkstraNode* toBeDelete = dijkstraNodes[dijkstraNodesIndex];
		toBeDelete->node = NULL;
		toBeDelete->previousNode = NULL;
		delete toBeDelete;
	}

	dijkstraPath = path;
	return true;
}

bool Pathfinding::dijkstraNodesVectorIsEmpty(const vector<DijkstraNode*> &dijkstraNodes)
{
	for (int i = 0; i != dijkstraNodes.size(); i++)
	{
		if (!dijkstraNodes[i]->removed)
			return false;
	}
	return true;
}

DijkstraNode* Pathfinding::nodeWithMinWeight(const vector<DijkstraNode*> &dijkstraNodes)
{
	float minWeight = (float) INT_MAX;
	DijkstraNode* nodeWithMinWeight = NULL;
	for (int i = 0; i != dijkstraNodes.size(); i++)
	{
		DijkstraNode* node = dijkstraNodes[i];
		if (!node->removed && node->weight < minWeight)
		{
			minWeight = node->weight;
			nodeWithMinWeight = node;
		}
	}
	return nodeWithMinWeight;
}

DijkstraNode* Pathfinding::dijkstraNodeForNode(vector<DijkstraNode*> &dijkstraNodes, NodeSphere* nodeSphere)
{
	for (int i = 0; i != dijkstraNodes.size(); i++)
	{
		if (dijkstraNodes[i]->node == nodeSphere)
			return dijkstraNodes[i];
	}
	return NULL;
}

float Pathfinding::distanceBetweenDijkstraNodes(DijkstraNode* nodeA, DijkstraNode* nodeB)
{
	hkvVec3 diff = nodeA->node->GetPosition() - nodeB->node->GetPosition();
	return diff.getLength();
}