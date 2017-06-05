/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef PLUGINMANAGER_H_INCLUDED
#define PLUGINMANAGER_H_INCLUDED

#include "OrbsPluginModule.h"

//#define MAKE_ALL_SCENE_ELEMENTS_VISIBLE //macro to reveal everything for debugging purpose

class GameManager : public VBaseObject, public IVisCallbackHandler_cl
{
public:
	GameManager() :  m_bPlayingTheGame(false),
		gameStarted(false),
		mainCamera(NULL),
		inputEventsBlocked(false),
		pickedNodeSphere(NULL),
		actionTargetNodeSphere(NULL)
#ifdef SUPPORTS_MOUSE
		,onPressHandledByGUI(false),
		onPressHandledByGame(false),
		lastPointerLocation(false),
		onPressTouchLocation(false),
		onMoveActive(false)
#endif
	{};

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData) HKV_OVERRIDE;

	// called when plugin is loaded/unloaded
	void OneTimeInit();
	void OneTimeDeInit();

	// switch to play-the-game mode. When not in vForge, this is default
	void SetPlayTheGame(bool bStatus);

	// access one global instance of the Game manager
	static GameManager& GlobalManager() {return g_GameManager;};

	//clear all variables
	void clearVariables();

	//init/deinit
	void preloadShadersLibs();
	void initNodeSpheres();
	void clearNodeSpheresVisibilityStatus();
	void deInitNodeSpheres();
	void initNodeSpheresVisibilityStatus();
	void initTravelingOrbs();
	vector<Orb*>& getTravelingOrbsForTeam(Team* team);
	int findNodeSphereIndex(NodeSphere* nodeSphere);
	void initCamera();

	//start level by unlocking input events, start orb generation and start timer
	void startLevel();
	//end level and launch the display of the end screen
	void endLevel(bool victory);

	//effects
	void initOverlays();
	void displaySelectionHalo(NodeSphere* nodeSphere); //common selection halo after clicking on a sphere
	void displayRingWave(); //ring wave that appears after clicking a action button
	void removeSelectionHalo();
	void removeRingWave();

	//sounds/music
	void initMusicPlayback();

	//2D overlays
	//void updatePathOverlays(float dt);

	//teams
	void setUpTeams();
	void releaseTeams();

	//input map
	void initInputEvents();
	void processInputEvents(float dt);
	hkvVec2 getCursorPosition();
	bool processPointerEvent(hkvVec2 pointerLocation, int eventType);
	void OnPickSphere();
	void OnPickNothing(bool unpickLastNodeSphere);
	void lockInputEvents(){this->inputEventsBlocked = true;};
	void unlockInputEvents(){this->inputEventsBlocked = false;};
	bool areInputEventsLocked(){return this->inputEventsBlocked;};

	//bridges
	void initBridges();
	void deInitBridges();
	void addBridge(Bridge* bridge);
	bool isBridgeAlreadyBuilt(NodeSphere* startSphere, NodeSphere* endSphere);
	void addTravelingOrb(Orb* orb);
	void removeTravelingOrb(Orb* orb);
	void destroyTravelingOrbsOnDeInit();
	Bridge* findBridge(NodeSphere* startSphere, NodeSphere* endSphere);

	//actions
	void performMoveAction();
	void performRallyAction();
	bool performBuildBridgesAction();
	void performSacrificeAction(NodeSphere* actionSphere, Team* team);

	//pathfinding calculations
	void makeOrbTravel(Orb* orb, vector<NodeSphere*> nodesPath); //single orb
	vector<Orb*> makeOrbsTravel(Team* team, NodeSphere* startSphere, NodeSphere* endSphere, int number); //random orbs among node sphere child orbs
	bool findNodesPath(NodeSphere* startSphere, NodeSphere* endSphere, vector<NodeSphere*> &path);

	//sacrified orbs
	int getSacrifiedOrbsCount(int teamId);

	//release function
	void unloadLevelElements();

	//is playing the game
	bool isPlayingTheGame(){return this->m_bPlayingTheGame;};

	//reset the game
	void resetGame();

	//getters and setters
	MainCamera* getMainCamera(){return this->mainCamera;};
	vector<NodeSphere*> &getNodeSpheres(){return this->nodeSpheres;};
	NodeSphere* getPickedNodeSphere(){return this->pickedNodeSphere;};
	void setPickedNodeSphere(NodeSphere* nodeSphere){this->pickedNodeSphere = nodeSphere;};
	NodeSphere* getActionTargetNodeSphere(){return this->actionTargetNodeSphere;};
	void setActionTargetNodeSphere(NodeSphere* nodeSphere){this->actionTargetNodeSphere = nodeSphere;};
	NodeSphere* findNodeSphereBy3DPosition(hkvVec3 position);
	vector<Bridge*> &getBridges(){return this->bridges;};
	vector<TeamedOrbs> &getTravelingOrbs(){return this->travelingOrbs;};
	void setGameStarted(bool gameStared){this->gameStarted = gameStarted;};
	bool hasGameStarted(){return this->gameStarted;};

	static bool s_tmp_start_level_once;
	static bool s_first_launch;

private:
	bool m_bPlayingTheGame;
	static GameManager g_GameManager;

	bool gameStarted;

	VSmartPtr<VInputMap> m_pInputMap;

	MainCamera* mainCamera;

	//input
	bool inputEventsBlocked;

	//Node Spheres
	vector<NodeSphere*> nodeSpheres;
	NodeSphere* pickedNodeSphere;
	NodeSphere* actionTargetNodeSphere; //the node sphere used as a target for move, rally point, build action or artifacts

	//bridges
	vector<Bridge*> bridges;

	//traveling orbs
	vector<TeamedOrbs> travelingOrbs; //orbs in transit on a bridge (e.g with no parent node sphere set)

	//debug tmp win32
#ifdef SUPPORTS_MOUSE
	bool onPressHandledByGUI;
	bool onPressHandledByGame;
	hkvVec2 lastPointerLocation;
	hkvVec2 onPressTouchLocation;
	bool onMoveActive;
#endif
};


#endif // PLUGINMANAGER_H_INCLUDED