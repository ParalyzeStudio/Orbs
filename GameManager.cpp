/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include "OrbsPluginPCH.h"
#include "GameManager.h"

#include "time.h"

// one global static instance
GameManager GameManager::g_GameManager;

bool GameManager::s_tmp_start_level_once = true;
bool GameManager::s_first_launch = true;

void GameManager::OneTimeInit()
{
	// the game manager should listen to the following callbacks:
	Vision::Callbacks.OnEditorModeChanged += this;
	Vision::Callbacks.OnBeforeSceneLoaded += this;
	Vision::Callbacks.OnAfterSceneLoaded += this;
	Vision::Callbacks.OnBeforeSceneUnloaded += this;
	Vision::Callbacks.OnAfterSceneUnloaded += this;
	Vision::Callbacks.OnUpdateSceneBegin += this;
	Vision::Callbacks.OnWorldDeInit += this;
	Vision::Callbacks.OnRenderHook += this;
}

void GameManager::OneTimeDeInit()
{
	Vision::Callbacks.OnEditorModeChanged -= this;
	Vision::Callbacks.OnBeforeSceneLoaded -= this;
	Vision::Callbacks.OnAfterSceneLoaded -= this;
	Vision::Callbacks.OnBeforeSceneUnloaded -= this;
	Vision::Callbacks.OnAfterSceneUnloaded -= this;
	Vision::Callbacks.OnUpdateSceneBegin -= this;
	Vision::Callbacks.OnWorldDeInit -= this;
	Vision::Callbacks.OnRenderHook -= this;
}

void GameManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender==&Vision::Callbacks.OnUpdateSceneBegin)
	{
		//This callback will be triggered at the beginning of every frame
		//You can add your own per frame logic here
		// [...]
		if (m_bPlayingTheGame)
		{
			float dt = Vision::GetTimer()->GetTimeDifference();
			processInputEvents(dt);
			//ScreenMaskManager::sharedInstance()->updateOverlays(dt);

			Level* currentLevel = LevelManager::sharedInstance().getCurrentLevel();
			if (currentLevel)
			{
				ObjectivesChecker::sharedInstance().checkObjectives(currentLevel);
			}
		}

		return;
	}

	if (pData->m_pSender==&Vision::Callbacks.OnEditorModeChanged)
	{
		// when vForge switches back from EDITORMODE_PLAYING_IN_GAME, turn off our play the game mode
		if (((VisEditorModeChangedDataObject_cl *)pData)->m_eNewMode != VisEditorManager_cl::EDITORMODE_PLAYING_IN_GAME)
			SetPlayTheGame(false);
		return;
	}

	if (pData->m_pSender==&Vision::Callbacks.OnBeforeSceneLoaded)
	{
		hkvLog::Error(">>>>>>>>>OnBeforeSceneLoaded");
		//here you can add you specific code before the scene is loaded
		clearVariables();

		if (SceneManager::sharedInstance().isNextSceneLevel())
		{
			ResourcePreloader::preloadGameTextures();
			AudioManager::sharedInstance().preloadGameSounds();
		}
		else
		{
			ResourcePreloader::preloadMenusTextures();
			AudioManager::sharedInstance().preloadMenuSounds();
		}
		
		return;
	}

	if (pData->m_pSender==&Vision::Callbacks.OnAfterSceneLoaded)
	{
		hkvLog::Error(">>>>>>>>>>>OnAfterSceneLoaded");
		//gets triggered when the play-the-game vForge is started or outside vForge after loading the scene
		if (Vision::Editor.IsPlayingTheGame()) 
			SetPlayTheGame(true);
		return;
	}

	if (pData->m_pSender==&Vision::Callbacks.OnBeforeSceneUnloaded)
	{
		hkvLog::Error(">>>>>>>>>>>OnBeforeSceneUnloaded");
		SetPlayTheGame(false);
		if (SceneManager::sharedInstance().isCurrentSceneLevel())
		{
			GameTimer::sharedInstance().OneTimeDeInit();
			AIManager::sharedInstance().OneTimeDeInit();
			unloadLevelElements();
		}
	}

	if (pData->m_pSender==&Vision::Callbacks.OnAfterSceneUnloaded)
	{
		hkvLog::Error(">>>>>>>>>>OnAfterSceneUnloaded");
	}

	if (pData->m_pSender==&Vision::Callbacks.OnWorldDeInit) 
	{
		hkvLog::Error(">>>>>>>>>>OnWorldDeInit");
		// this is important when running outside vForge
		SetPlayTheGame(false);

		return;
	}
}


// switch to play-the-game mode
void GameManager::SetPlayTheGame(bool bStatus)
{
	if (m_bPlayingTheGame==bStatus)
		return;

	m_bPlayingTheGame = bStatus;

	if (m_bPlayingTheGame)
	{
		//preload some resources
		preloadShadersLibs();
		LevelManager::sharedInstance().preloadStringsFiles();
		AchievementManager::sharedInstance().preloadStringsFiles();
		TutorialsHandler::sharedInstance().preloadStringsFiles();
		StringsXmlHelper::sharedInstance().preloadStringsFiles();
		AudioManager::sharedInstance().preloadStringsFiles();
		FontManager::loadFonts();

		if (SceneManager::sharedInstance().isNextSceneLevel()) //in-game level
		{
			//tmp
			if (GameManager::s_tmp_start_level_once)
			{
				AchievementManager::sharedInstance().buildAchievementsFromXML();
				LevelManager &levelManager = LevelManager::sharedInstance();
				levelManager.buildLevelsFromXML();
				Level* firstLevel = levelManager.getLevels()[3];
				levelManager.setCurrentLevel(firstLevel); //tmp

				GameManager::s_first_launch = false;
				GameManager::s_tmp_start_level_once = false;
			}
			
			GameTimer::sharedInstance().OneTimeInit();
			AIManager::sharedInstance().OneTimeInit();
			this->gameStarted = false;
			GameTimer::sharedInstance().start();
			GameManager::GlobalManager().lockInputEvents();
			
			//build tutorials if level 1
			int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
			if (currentLevelNumber == 1)
				TutorialsHandler::sharedInstance().buildTutorialsFromXML();

			//Init the GUI context
			GameDialogManager* gameDialogManager = GameDialogManager::sharedInstance();
			gameDialogManager->initGUIContext();

			initCamera();
			setUpTeams();

			initNodeSpheres();
			clearNodeSpheresVisibilityStatus();
			initNodeSpheresVisibilityStatus();
			mainCamera->updateCameraBoundaries();
			initTravelingOrbs();
			initBridges();
			initOverlays();
			
			gameDialogManager->showHUD();

			//show veil at start of a level
			Veil* veil = new Veil();
			veil->init(V_RGBA_BLACK, 1.0f);
			veil->SetOrder(-10);
			veil->fadeTo(0.0f, 5.0f, 0.0f);

			//show intro sentences (1st group)
			/*if (currentLevelNumber == 1)
			{
				TutorialsHandler::sharedInstance().showTutorialGroup1();
			}
			else
			{*/
				this->inputEventsBlocked = true;
				MessageHandler::sharedInstance().queueAndShowIntroMessages(0, 2.0f);
			//}

		}
		else //menus
		{
			//builds levels
			LevelManager::sharedInstance().setCurrentLevel(NULL);
			LevelManager::sharedInstance().buildLevelsFromXML();

			//builds menus
			MenuManager* menuManager = MenuManager::sharedInstance();
			menuManager->initGUIContext();

			if (GameManager::s_first_launch)
			{
				menuManager->showSplashScreens();
				GameManager::s_first_launch = false;
			}
			else	
				menuManager->showIntroMenu();
			//menuManager->showLevelsMenu();
		}

		initMusicPlayback();
		initInputEvents();
	} 
	else
	{
		//DeInit the GUI context previously created
		if (GameDialogManager::sharedInstance()->isContextActive())
		{
			GameDialogManager* gameDialogManager = GameDialogManager::sharedInstance();
			gameDialogManager->deInitGUIContext();
		}
		else if (MenuManager::sharedInstance()->isContextActive())
		{
			MenuManager* menuManager = MenuManager::sharedInstance();
			menuManager->deInitGUIContext();
		}
	}
}

void GameManager::clearVariables()
{
	m_bPlayingTheGame = false;
	gameStarted = false;
	mainCamera = NULL;
	inputEventsBlocked = false;
	pickedNodeSphere = NULL;
	actionTargetNodeSphere = NULL;

#ifdef SUPPORTS_MOUSE
	onPressHandledByGUI = false;
	onPressHandledByGame = false;
	lastPointerLocation = hkvVec2(0,0);
	onPressTouchLocation = hkvVec2(0,0);
	onMoveActive = false;
#endif
}

void GameManager::preloadShadersLibs()
{
	Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
}

void GameManager::initNodeSpheres()
{
	DynArray_cl<VisBaseEntity_cl*> nodeSphereEntities;
	Vision::Game.SearchEntity("NodeSphere", &nodeSphereEntities);
	for (int i = 0; i != nodeSphereEntities.GetSize(); i++)
	{
		VisBaseEntity_cl* nodeSphereEntity = nodeSphereEntities.Get(i);

		NodeSphere* nodeSphere = (NodeSphere*) nodeSphereEntities.Get(i);
		nodeSphere->init();
		nodeSpheres.push_back(nodeSphere);
	}
	this->pickedNodeSphere = NULL;
	this->actionTargetNodeSphere = NULL;
}

void GameManager::deInitNodeSpheres()
{
	//remove related objects
	for (int nodeSphereIndex = 0; nodeSphereIndex != nodeSpheres.size(); nodeSphereIndex++)
	{
		NodeSphere* nodeSphere = nodeSpheres[nodeSphereIndex];
		nodeSphere->destroyOrbsOnDeInit();
		//nodeSphere->removePortals();
	}

	this->destroyTravelingOrbsOnDeInit();

	
	for (int nodeSphereIndex = 0; nodeSphereIndex != nodeSpheres.size(); nodeSphereIndex++)
	{
		NodeSphere* nodeSphere = nodeSpheres[nodeSphereIndex];

		//release particle effects
		nodeSphere->purgeParticleEffects();

		//remove node spheres with their meta orbs
		nodeSphere->removeMetaOrbs();
		Vision::Game.RemoveEntity(nodeSphere);
	}

	this->nodeSpheres.clear();
}

void GameManager::clearNodeSpheresVisibilityStatus()
{
	for (int nodeSphereIndex = 0; nodeSphereIndex != nodeSpheres.size(); nodeSphereIndex++)
	{
		nodeSpheres[nodeSphereIndex]->clearVisibilityStatus();
	}
}

void GameManager::initNodeSpheresVisibilityStatus()
{
	for (int nodeSphereIndex = 0; nodeSphereIndex != nodeSpheres.size(); nodeSphereIndex++)
	{
		nodeSpheres[nodeSphereIndex]->initVisibilityStatus();
	}
}

void GameManager::initTravelingOrbs()
{
	//init the childOrbs vector by pushing an element for each team
	vector<Team*> allTeams = TeamManager::sharedInstance().getAllTeams();
	unsigned int teamsCount = allTeams.size();
	travelingOrbs.reserve(teamsCount);
	for (int teamIndex = 0; teamIndex != teamsCount; teamIndex++)
	{
		Team* team = allTeams[teamIndex];
		TeamedOrbs teamedOrbs;
		teamedOrbs.team = team;
		vector<Orb*> orbs; //push_back empty vector
		teamedOrbs.orbs = orbs;
		travelingOrbs.push_back(teamedOrbs);
	}
}

vector<Orb*>& GameManager::getTravelingOrbsForTeam(Team* team)
 {
	 for (int travelingOrbsIndex = 0; travelingOrbsIndex != travelingOrbs.size(); travelingOrbsIndex++)
	 {
		 TeamedOrbs &teamedTravelingOrbs = travelingOrbs[travelingOrbsIndex];
		 if (teamedTravelingOrbs.team->getID() == team->getID())
			return teamedTravelingOrbs.orbs;
	 }

	 VASSERT_MSG(false, "Invalid team");
 }

int GameManager::findNodeSphereIndex(NodeSphere* nodeSphere)
{
	for (int index = 0; index != nodeSpheres.size(); index++)
	{
		if (nodeSphere == nodeSpheres[index])
			return index;
	}
	return -1;
}

void GameManager::initCamera()
{
	VisBaseEntity_cl* cameraEntity = Vision::Game.SearchEntity("MainCamera");
	
	if (vdynamic_cast<MainCamera*>(cameraEntity))
		mainCamera = (MainCamera*) cameraEntity;
	else 
		mainCamera = NULL;

	if (mainCamera)
		MultiTouchManager::sharedInstance().registerCallback(mainCamera);
}

void GameManager::startLevel()
{
	if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
	{
		this->unlockInputEvents();
		this->mainCamera->unblockCamera();
	}
	this->gameStarted = true;
}

void GameManager::endLevel(bool victory)
{
	GameDialogManager::sharedInstance()->showDefeatVictoryMenu(victory, 5.0f);
}

void GameManager::initMusicPlayback()
{
	bool musicActive = PersistentDataManager::sharedInstance().isMusicActive();
	if (musicActive) //call this only when the game starts
	{
		AudioManager::sharedInstance().queueRandomMusic(0.0f);
	}
}

void GameManager::initInputEvents()
{
	m_pInputMap = new VInputMap(GENERIC_NUM_INPUTS + CAMERA_NUM_INPUTS, INPUT_NUM_ALTERNATIVE);

#ifdef SUPPORTS_MOUSE
	//mouse
	m_pInputMap->MapTrigger(POINTER_DOWN, V_MOUSE, CT_MOUSE_LEFT_BUTTON, VInputOptions::Once(ONCE_ON_PRESS));
	m_pInputMap->MapTrigger(POINTER_UP, V_MOUSE, CT_MOUSE_LEFT_BUTTON, VInputOptions::Once(ONCE_ON_RELEASE));
	m_pInputMap->MapTrigger(POINTER_MOVE, V_MOUSE, CT_MOUSE_LEFT_BUTTON);
	m_pInputMap->MapTrigger(POINTER_LOCATION_X, V_MOUSE, CT_MOUSE_ABS_X);
	m_pInputMap->MapTrigger(POINTER_LOCATION_Y, V_MOUSE, CT_MOUSE_ABS_Y);
#endif

#ifdef SUPPORTS_KEYBOARD
	//camera
	m_pInputMap->MapTrigger(CAMERA_MOVE_FORWARD,  V_KEYBOARD, CT_KB_Z);
	m_pInputMap->MapTrigger(CAMERA_MOVE_BACKWARD, V_KEYBOARD, CT_KB_S);
	m_pInputMap->MapTrigger(CAMERA_MOVE_LEFT,     V_KEYBOARD, CT_KB_Q);
	m_pInputMap->MapTrigger(CAMERA_MOVE_RIGHT,    V_KEYBOARD, CT_KB_D);
	m_pInputMap->MapTrigger(CAMERA_TURN_LEFT,    V_KEYBOARD, CT_KB_A);
	m_pInputMap->MapTrigger(CAMERA_TURN_RIGHT,    V_KEYBOARD, CT_KB_E);
	m_pInputMap->MapTrigger(CAMERA_ZOOM_IN,    V_KEYBOARD, CT_KB_KP_PLUS);
	m_pInputMap->MapTrigger(CAMERA_ZOOM_OUT,    V_KEYBOARD, CT_KB_KP_MINUS);
#endif

#ifdef _VISION_ANDROID
	m_pInputMap = new VInputMap(GENERIC_NUM_INPUTS, INPUT_NUM_ALTERNATIVE);

	float const priority = -950.0f;
	int const width = Vision::Video.GetXRes();
	int const height = Vision::Video.GetYRes();

	VTouchArea *touchScreen = new VTouchArea(VInputManager::GetTouchScreen(), VRectanglef(0.0f, 0.0f, width, height), priority);
	m_pInputMap->MapTrigger(POINTER_DOWN, touchScreen, CT_TOUCH_ANY, VInputOptions::Once(ONCE_ON_PRESS));
	m_pInputMap->MapTrigger(POINTER_UP, touchScreen, CT_TOUCH_ANY, VInputOptions::Once(ONCE_ON_RELEASE));
	m_pInputMap->MapTrigger(POINTER_MOVE, touchScreen, CT_TOUCH_ANY);
	m_pInputMap->MapTrigger(POINTER_LOCATION_X, touchScreen, CT_TOUCH_ABS_X);
	m_pInputMap->MapTrigger(POINTER_LOCATION_Y, touchScreen, CT_TOUCH_ABS_Y);
#endif
}

void GameManager::initOverlays()
{
	////halos
	////fxSelectionHalo = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\ring_wave_small_halo.xml");
	//selectionHalo = NULL;

	//fxTravelRingWave = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\green_ring_wave.xml");
	//fxRallyRingWave = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\yellow_ring_wave.xml");
	//largeRingWave = NULL;

	//fxSmallRingWave = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\ring_wave_small_halo.xml");	

	////beams
	//fxBeamLink = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\beam.xml");
}

void GameManager::displaySelectionHalo(NodeSphere* nodeSphere)
{
	//if (!selectionHalo)
	//{
	//	//build a new one
	//	selectionHalo = new SelectionHalo();
	//	selectionHalo->init(nodeSphere);
	//}
	//else
	//	selectionHalo->updateParentNodeSphere(nodeSphere);

	//selectionHalo->updateActionMode(ACTION_NORMAL_MODE);
	////selectionHalo = fxSelectionHalo->CreateParticleEffectInstance(nodeSphere->GetPosition(), hkvVec3(0.f, 0.f, 0.f), 1.2f);
	////selectionHalo->SetVisibleBitmask(0xffffffff);
}

void GameManager::displayRingWave()
{
	//int mode = this->getMouseActionMode();
	//if (mode == ACTION_TRAVEL_MODE)
	//{
	//	largeRingWave = fxTravelRingWave->CreateParticleEffectInstance(pickedNodeSphere->GetPosition(), hkvVec3(0.f, 0.f, 0.f), 1.2f);
	//	//largeRingWave->SetVisibleBitmask(0xffffffff);
	//}
	//else if (mode == ACTION_RALLY_POINT_MODE)
	//{
	//	largeRingWave = fxRallyRingWave->CreateParticleEffectInstance(pickedNodeSphere->GetPosition(), hkvVec3(0.f, 0.f, 0.f), 1.2f);
	//	//largeRingWave->SetVisibleBitmask(0xffffffff);
	//}
}

void GameManager::removeSelectionHalo()
{
	//if (selectionHalo)
	//	selectionHalo->destroySelf();
	////selectionHalo->SetFinished();
	////selectionHalo = NULL;
}

void GameManager::removeRingWave()
{
	//largeRingWave->SetFinished();
	//largeRingWave = NULL;
}


//void GameManager::updatePathOverlays(float dt)
//{
//	vector<FadeScreenMask*>::iterator pathOverlaysIt;
//
//	int pathOverlaysSize = pathOverlays.size();
//	for (int pathOverlayIndex = 0; pathOverlayIndex != pathOverlaysSize; pathOverlayIndex++)
//	{
//		FadeScreenMask* pathOverlay = pathOverlays[pathOverlayIndex];
//		if (!pathOverlay->getCanDispose())
//			pathOverlay->updateSelf(dt);
//		else
//		{
//			pathOverlays.erase(pathOverlays.begin() + pathOverlayIndex);
//			pathOverlayIndex--;
//			pathOverlaysSize--;
//			pathOverlay->DisposeObject();
//		}
//	}
//}


void GameManager::setUpTeams()
{
	Level* currentLevel = LevelManager::sharedInstance().getCurrentLevel();
	vector<TeamParty*> &levelParties = currentLevel->getParties();

	//copy levelParties into new objects sent to TeamManager
	vector<TeamParty*> copiedParties (levelParties.size() + 1);
	for (size_t partyIndex = 0; partyIndex < levelParties.size(); partyIndex++)
	{
		TeamParty* levelParty = levelParties[partyIndex];
		copiedParties[partyIndex] = new TeamParty(*levelParty);
		copiedParties[partyIndex]->AddRef(); //copying a VRefCounter resets the ref count to 0, so add a new ref to it
	}

	//add the neutral team
	TeamParty* neutralParty = new TeamParty();
	neutralParty->AddRef();
	vector<Team*> neutralTeamMates;
	Team* neutralTeam = new Team();
	neutralTeam->AddRef();
	neutralTeam->init(0, "", neutralParty);
	neutralTeamMates.push_back(neutralTeam);
	neutralParty->init(TEAM_0, neutralTeamMates);
	copiedParties[levelParties.size()] = neutralParty;

	TeamManager::sharedInstance().setParties(copiedParties); //transfer parties/teams from LevelManager to TeamManager
}

void GameManager::releaseTeams()
{
	TeamManager::sharedInstance().cleanParties();
}


void GameManager::processInputEvents(float dt)
{
	if (inputEventsBlocked)
		return;

	if (m_pInputMap)
	{
		//KEYBOARD EVENTS
#ifdef SUPPORTS_KEYBOARD
		if (mainCamera)
		{
			bool bLeft = m_pInputMap->GetTrigger(CAMERA_MOVE_LEFT)!=0;
			bool bRight = m_pInputMap->GetTrigger(CAMERA_MOVE_RIGHT)!=0;
			bool bUp = m_pInputMap->GetTrigger(CAMERA_MOVE_FORWARD)!=0;
			bool bDown = m_pInputMap->GetTrigger(CAMERA_MOVE_BACKWARD)!=0;
			bool bTurnLeft = m_pInputMap->GetTrigger(CAMERA_TURN_LEFT)!=0;
			bool bTurnRight = m_pInputMap->GetTrigger(CAMERA_TURN_RIGHT)!=0;
			bool bZoomIn = m_pInputMap->GetTrigger(CAMERA_ZOOM_IN)!=0;
			bool bZoomOut = m_pInputMap->GetTrigger(CAMERA_ZOOM_OUT)!=0;

			if (bLeft)
				mainCamera->moveLeft(dt);
			if (bRight)
				mainCamera->moveRight(dt);
			if (bUp)
				mainCamera->moveForward(dt);
			if (bDown)
				mainCamera->moveBackward(dt);
			if (bTurnLeft)
				mainCamera->turnLeft(dt);
			if (bTurnRight)
				mainCamera->turnRight(dt);
			if (bZoomIn)
				mainCamera->onPinchChanged(hkvVec2(0,0), 10.0f);
			if (bZoomOut)
				mainCamera->onPinchChanged(hkvVec2(0,0), -10.0f);
		}
#endif

		//MOUSE EVENTS
		bool bLeftClickOnPress = false;
		bool bLeftClickOnRelease = false;
		bool bLeftClickOnMove = false;
#ifdef SUPPORTS_MOUSE
		bLeftClickOnPress = m_pInputMap->GetTrigger(POINTER_DOWN)!=0;
		bLeftClickOnRelease = m_pInputMap->GetTrigger(POINTER_UP)!=0;	
		bLeftClickOnMove = m_pInputMap->GetTrigger(POINTER_MOVE)!=0;
#endif

		/************TMP DBUG REMOTE INPUT **************/
#ifdef SUPPORTS_MOUSE
		VMultiTouchInputPC& inputDevice = (VMultiTouchInputPC&)VInputManager::GetInputDevice(INPUT_DEVICE_TOUCHSCREEN);
		int touchCount = 0;
		vector<hkvVec2> touches;
		for (int i = 0; i < inputDevice.GetMaximumNumberOfTouchPoints(); i++)
		{
			if (!inputDevice.IsActiveTouch(i))
				continue;

			touchCount++;
			float xCoords = inputDevice.GetTouchPointValue(i, CT_TOUCH_ABS_X);
			float yCoords = inputDevice.GetTouchPointValue(i, CT_TOUCH_ABS_Y);
			touches.push_back(hkvVec2(xCoords, yCoords));
		}
		MultiTouchManager::sharedInstance().replaceTouches(touches);
#endif
		/************TMP DBUG REMOTE INPUT **************/

		//TOUCH EVENTS
bool touchScreenOnPress = false;
bool touchScreenOnRelease = false;
bool touchScreenOnMove = false;



#ifdef _VISION_ANDROID
		VMultiTouchInputAndroid& inputDevice = (VMultiTouchInputAndroid&)VInputManager::GetInputDevice(INPUT_DEVICE_TOUCHSCREEN);
		int touchCount = 0;

		vector<hkvVec2> touches;
		for (int i = 0; i < inputDevice.GetMaximumNumberOfTouchPoints(); i++)
		{
			if (!inputDevice.IsActiveTouch(i))
				continue;
			touchCount++;
			float xCoords = inputDevice.GetTouchPointValue(i, CT_TOUCH_ABS_X);
			float yCoords = inputDevice.GetTouchPointValue(i, CT_TOUCH_ABS_Y);
			touches.push_back(hkvVec2(xCoords, yCoords));

		}
		MultiTouchManager::sharedInstance().replaceTouches(touches);
#endif
		
		bool pointerOnPress = bLeftClickOnPress || touchScreenOnPress;
		bool pointerOnRelease = bLeftClickOnRelease || touchScreenOnRelease;
		bool pointerOnMove = bLeftClickOnMove || touchScreenOnMove;

		
		
#ifdef SUPPORTS_MOUSE
		//cursorposition
		hkvVec2 pointerLocation = getCursorPosition();

		if (SceneManager::sharedInstance().isCurrentSceneLevel()) //level
		{
			GameDialogManager* gameDialogManager = GameDialogManager::sharedInstance();
			if (pointerOnPress)
			{
				if (gameDialogManager->clickContainedInGUI(pointerLocation)) //we clicked on a GUI element
				{
					gameDialogManager->processPointerEvent(pointerLocation, POINTER_DOWN);
					onPressHandledByGUI = true;
					onPressHandledByGame = false;
				}
				else 
				{
					onPressTouchLocation = pointerLocation;
					onPressHandledByGame = this->processPointerEvent(pointerLocation, POINTER_DOWN);
					onPressHandledByGUI = false;
				}
				lastPointerLocation = pointerLocation;
				onMoveActive = false;
			}
			if (pointerOnMove)
			{
				if (this->onPressHandledByGUI)
				{
					gameDialogManager->processPointerEvent(pointerLocation, POINTER_MOVE);
				}
				else
				{
					if (!this->processPointerEvent(pointerLocation, POINTER_MOVE))
					{

						if (!this->onMoveActive)
						{
							float distanceFromOnPressLocation = (pointerLocation - onPressTouchLocation).getLength();
							if (distanceFromOnPressLocation > ON_MOVE_DELTA_THRESHOLD) //start on move
							{
								onMoveActive = true;
							}
						}
						else
						{
							if (!pointerLocation.isEqual(lastPointerLocation, ON_MOVE_DELTA_EPSILON))
							{
								hkvLog::Error(">>>onMove");
								hkvVec2 deltaPosition = pointerLocation - lastPointerLocation;
								vector<MultiTouchCallback*> callbacks = MultiTouchManager::sharedInstance().getCallbacks();
								for (int callbackIndex = 0; callbackIndex != callbacks.size(); callbackIndex++)
								{
									MultiTouchCallback* callback = callbacks[callbackIndex];
									callback->onPositionChanged(deltaPosition);
								}
							}
						}
						lastPointerLocation = pointerLocation;
					}
				}
			}
			if (pointerOnRelease)
			{
				if (onPressHandledByGUI) //we clicked on a GUI element
				{
					gameDialogManager->processPointerEvent(pointerLocation, POINTER_UP);
				}
				else 
				{
					if (this->onPressHandledByGame)
					{
						//if (GameManager::GlobalManager().processPointerEvent(pointerLocation, POINTER_UP)) //something has been picked
						//{
						//	this->OnPickSphere();
						//}
						//else
						//	this->OnPickNothing(false);
						if (!GameManager::GlobalManager().processPointerEvent(pointerLocation, POINTER_UP)) //something has been picked
							this->OnPickNothing(false);
					}
					else
					{
						if (!onMoveActive)
							this->OnPickNothing(true);
					}
				}
			}
		}
		else //menus
		{
			MenuManager* menuManager = MenuManager::sharedInstance();
			if (pointerOnPress)
				menuManager->processPointerEvent(pointerLocation, POINTER_DOWN);
			if (pointerOnRelease)
				menuManager->processPointerEvent(pointerLocation, POINTER_UP);
			if (pointerOnMove)
				menuManager->processPointerEvent(pointerLocation, POINTER_MOVE);
		}
		#endif
	}
}

hkvVec2 GameManager::getCursorPosition()
{
#ifdef SUPPORTS_MOUSE
	int iCursorX = VInputDeviceManagerPC::GetMouse().GetRawControlValue( CT_MOUSE_RAW_CURSOR_X );
	int iCursorY = VInputDeviceManagerPC::GetMouse().GetRawControlValue( CT_MOUSE_RAW_CURSOR_Y );

	return hkvVec2((float) iCursorX, (float) iCursorY);
#endif
	return hkvVec2(0,0);
}

bool GameManager::processPointerEvent(hkvVec2 pointerLocation, int eventType)
{
	bool sphereHit = false;
	for (int sphereIndex = 0; sphereIndex != nodeSpheres.size(); sphereIndex++)
	{
		NodeSphere* nodeSphere = nodeSpheres[sphereIndex];
		if (nodeSphere->trigger(pointerLocation, eventType))
		{
			sphereHit = true;
		}
	}

	return sphereHit;
}

void GameManager::OnPickSphere()
{
	if (this->pickedNodeSphere && this->pickedNodeSphere->isArtifact())
	{
		((Artifact*)this->pickedNodeSphere)->performPowerAction(this->actionTargetNodeSphere);
	}
	else
	{
		hkvLog::Error("+++onPickSphere");
		GameHUD* gameHUD = GameDialogManager::sharedInstance()->getHUD();
		ActionPanel* actionPanel = gameHUD->getActionPanel();
		if (!actionPanel || 
			(actionPanel->getActionMode() == ACTION_NORMAL_MODE/* && this->pickedNodeSphere != actionPanel->getParentNodeSphere())*/))
		{
			gameHUD->showSelectionHaloPanel();
			gameHUD->showActionPanel();
			gameHUD->showInfoPanel();
			if ((this->pickedNodeSphere->getVisibilityStatus() != VISIBILITY_STATUS_VISIBLE)) //unvisible sphere
				gameHUD->dismissCorePanel();
			else
				gameHUD->showCorePanel();
			GameDialogManager::sharedInstance()->getHUD()->updateHUDForPickedNodeSphere();

			if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1) //first level, tutorials
				TutorialsHandler::sharedInstance().showTutorialGroup2();
		}
		else if (actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
		{
			if (this->pickedNodeSphere != this->actionTargetNodeSphere)
			{
				GameHUD* hud = GameDialogManager::sharedInstance()->getHUD();
				hud->removeActiveTravelPath();
				hud->showTravelPath();
				hud->showMoveSubActionPanel();
				hud->showTravelPath();
			}
		}
		else if (actionPanel->getActionMode() == ACTION_RALLY_POINT_MODE)
		{
			if (!this->pickedNodeSphere->getRallySphere()) //a rally sphere already exists, wait for the player to delete it
			{
				ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
				RallySubMenuPanel* rallySubMenuPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
				rallySubMenuPanel->showBtn(ID_RALLY_SUB_PANEL_VALIDATE, 0.0f);

				gameHUD->removeActiveRallyPath();
				gameHUD->removeRallySphereHalo();

				gameHUD->showRallySphereHalo(true);
				gameHUD->showRallySpherePath();
			}
		}
		else if (actionPanel->getActionMode() == ACTION_BUILD_BRIDGE_MODE)
		{
			bool validBridge = false;
			vector<Bridge*> &potentialBridges = GameDialogManager::sharedInstance()->getHUD()->getPotentialBridges();
			for (int potentialBridgeIndex = 0; potentialBridgeIndex != potentialBridges.size(); potentialBridgeIndex++)
			{
				Bridge* potentialBridge = potentialBridges[potentialBridgeIndex];
				if (potentialBridge->getEndSphere() == actionTargetNodeSphere)
				{
					validBridge = true;
					break;
				}
			}
			if (validBridge)
			{
				//show sub panel
				ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
				BuildSubMenuPanel* buildSubPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL); 
				buildSubPanel->showBtn(ID_BUILD_SUB_PANEL_VALIDATE, 0.2f);	

				//show halo
				GameDialogManager::sharedInstance()->getHUD()->showBuildSphereHalo();
			}
			else
			{
				bool bridgeAlreadyExists = false;
				vector<Bridge*> &allBridges = GameManager::GlobalManager().getBridges();
				for (int bridgeIndex = 0; bridgeIndex != allBridges.size(); bridgeIndex++)
				{
					if (allBridges[bridgeIndex]->equals(pickedNodeSphere, actionTargetNodeSphere))
					{
						bridgeAlreadyExists = true;
						break;
					}
				}

				if (bridgeAlreadyExists)
					MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("bridge_already_exists"));
				else
					MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("unreachable_sphere"));
			}

			hkvLog::Error("actionMode:%i", GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->getActionMode());
		}
	}
}

void GameManager::OnPickNothing(bool unpickLastNodeSphere)
{
	hkvLog::Error("+++OnPickNothing");
	GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
	if (unpickLastNodeSphere && this->pickedNodeSphere)
	{
		GameDialogManager::sharedInstance()->getHUD()->dismissSelectionHaloPanel();
		GameDialogManager::sharedInstance()->getHUD()->removeRallySphereHalo();
		GameDialogManager::sharedInstance()->getHUD()->removeBuildSphereHalo();
		GameDialogManager::sharedInstance()->getHUD()->removeActiveRallyPath();
		GameDialogManager::sharedInstance()->getHUD()->removeActiveTravelPath();
		pickedNodeSphere->unpick();
		GameDialogManager::sharedInstance()->getHUD()->dismissCorePanel();
		GameDialogManager::sharedInstance()->getHUD()->dismissActionPanel();
		GameDialogManager::sharedInstance()->getHUD()->dismissInfoPanel();
	}
	else
	{
		if (this->pickedNodeSphere)
		{
			GameDialogManager::sharedInstance()->getHUD()->removeActiveTravelPath();
			//ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
			//int actionMode = actionPanel->getActionMode();
			//if (actionMode == ACTION_TRAVEL_MODE)
			//{
			//	GameManager::GlobalManager().performMoveAction();
			//	actionPanel->updateActionMode(ACTION_NORMAL_MODE);
			//}
		}
	}
}

void GameManager::initBridges()
{
	for (int i = 0; i != nodeSpheres.size(); i++)
	{
		NodeSphere* nodeSphere = nodeSpheres[i];
		vector<NodeSphere*> neighbouringSpheres = nodeSphere->getNeighbouringSpheres();
		for (int j = 0; j != neighbouringSpheres.size(); j++)
		{
			NodeSphere* startSphere = nodeSphere;
			NodeSphere* endSphere = neighbouringSpheres[j];

			if (startSphere->GetPosition() == hkvVec3(1200,500,400))
				int a = 1;

			if (!(startSphere->containsNeighbouringSphere(endSphere) && endSphere->containsNeighbouringSphere(startSphere))) //spheres are not double linked, continue
				continue;
			
			if (!isBridgeAlreadyBuilt(startSphere, endSphere))
			{
				Bridge* bridge = new Bridge();
				bridge->AddRef();
				bridge->init(startSphere, endSphere);
				bridges.push_back(bridge);

				//reveal bridge or not?
				bool revealBridge = false;
				if (startSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE && endSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
				{
					revealBridge = true;
				}
				else if ((startSphere->getVisibilityStatus() == VISIBILITY_STATUS_HIDDEN && endSphere->getVisibilityStatus() != VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY)
						 ||
						 (endSphere->getVisibilityStatus() == VISIBILITY_STATUS_HIDDEN && startSphere->getVisibilityStatus() != VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY))
				{
					revealBridge = true;
				}
				else if ((startSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE && endSphere->getVisibilityStatus() == VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR)
						 ||
						 (endSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE && startSphere->getVisibilityStatus() == VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR))
				{
					revealBridge = true;
				}

				bridge->setRevealed(revealBridge);
			}
		}
	}
}

void GameManager::deInitBridges()
{
	for (int bridgeIndex = 0; bridgeIndex != bridges.size(); bridgeIndex++)
	{
		bridges[bridgeIndex]->destroySelf();
	}
	bridges.clear();
}

void GameManager::addBridge(Bridge* bridge)
{
	this->bridges.push_back(bridge);
}

bool GameManager::isBridgeAlreadyBuilt(NodeSphere* startSphere, NodeSphere* endSphere)
{
	for (int bridgeIndex = 0; bridgeIndex != bridges.size(); bridgeIndex++)
	{
		Bridge* bridge = (Bridge*) bridges[bridgeIndex];
		if (bridge->equals(startSphere, endSphere))
			return true;
	}
	return false;
}

void GameManager::addTravelingOrb(Orb* orb)
{
	vector<Orb*>& teamedTravelingOrbs = getTravelingOrbsForTeam(orb->getTeam());
	teamedTravelingOrbs.push_back(orb);
}

void GameManager::removeTravelingOrb(Orb* orb)
{
	vector<Orb*> &teamedTravelingOrbs = getTravelingOrbsForTeam(orb->getTeam());

	vector<Orb*>::iterator orbsIt;
	for (orbsIt = teamedTravelingOrbs.begin(); orbsIt != teamedTravelingOrbs.end(); orbsIt++)
	{
		if (*orbsIt == orb)
		{
			teamedTravelingOrbs.erase(orbsIt);
			return;
		}
	}
}

void GameManager::destroyTravelingOrbsOnDeInit()
{
	for (int travelingOrbsIndex = 0; travelingOrbsIndex != travelingOrbs.size(); travelingOrbsIndex++)
	{
		vector<Orb*> &teamedTravelingOrbs = travelingOrbs[travelingOrbsIndex].orbs;
		for (int teamedTravelingOrbsIndex = 0; teamedTravelingOrbsIndex != teamedTravelingOrbs.size(); teamedTravelingOrbsIndex++)
		{
			teamedTravelingOrbs[teamedTravelingOrbsIndex]->Release();
		}
	}

	travelingOrbs.clear();
}

Bridge* GameManager::findBridge(NodeSphere* startSphere, NodeSphere* endSphere)
{
	vector<Bridge*> allBridges = GameManager::GlobalManager().getBridges();
	for (int bridgeIndex = 0; bridgeIndex != allBridges.size(); bridgeIndex++)
	{
		if (allBridges[bridgeIndex]->equals(startSphere, endSphere))
		{
			return allBridges[bridgeIndex];
		}
	}

	return NULL;
}

void GameManager::performMoveAction()
{
	int defaultTeamOrbsCount = this->pickedNodeSphere->getOrbsCountForTeam(TeamManager::sharedInstance().getDefaultTeam());
	if (defaultTeamOrbsCount == 0)
	{
		MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("no_orb_to_move"));
		return;
	}

	ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
	float scale = GameDialogManager::sharedInstance()->getScale();

	if (actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
	{
		actionPanel->updateActionMode(ACTION_NORMAL_MODE);
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
		GameDialogManager::sharedInstance()->getHUD()->removeActiveTravelPath();
		GameHUD* gameHud = GameDialogManager::sharedInstance()->getHUD();
		float dismissAnimationDuration = gameHud->dismissMoveSubActionPanel();

		actionPanel->showMoveBtn(hkvVec2(-78.0f * scale, -180.f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
		if (this->pickedNodeSphere->getTeam() == TeamManager::sharedInstance().getDefaultTeam())
		{
			if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
				&&
				TutorialsHandler::sharedInstance().isRallyBtnRevealed())
				||
				LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
			{
				actionPanel->showRallyBtn(hkvVec2(-78.0f * scale, 0.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
			if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
				&&
				TutorialsHandler::sharedInstance().isBuildBtnRevealed())
				||
				LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
			{
				if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
					actionPanel->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
		}
	}
	else if (actionPanel->getActionMode() == ACTION_RALLY_POINT_MODE)
	{
		performRallyAction();
		performMoveAction();
	}
	else if (actionPanel->getActionMode() == ACTION_BUILD_BRIDGE_MODE)
	{
		performBuildBridgesAction();
		performMoveAction();
	}
	else
	{
		actionPanel->updateActionMode(ACTION_TRAVEL_MODE);

		//scale up move button and translate buttons
		float moveBtnScale = 196.0f / 156.0f;

		actionPanel->showMoveBtn(hkvVec2(-98.0f * scale, -180.f * scale), hkvVec2(moveBtnScale, moveBtnScale), true, 0.2f, 0.0f);
		if (this->pickedNodeSphere->getTeam() == TeamManager::sharedInstance().getDefaultTeam())
		{
			if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
				&&
				TutorialsHandler::sharedInstance().isRallyBtnRevealed())
				||
				LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
			{
				actionPanel->showRallyBtn(hkvVec2(-78.0f * scale, 20.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
			if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
				&&
				TutorialsHandler::sharedInstance().isBuildBtnRevealed())
				||
				LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
			{
				if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
					actionPanel->showBuildBtn(hkvVec2(-78.0f * scale, 200.f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
		}
	}
	//this->displayRingWave();
}

void GameManager::performRallyAction()
{
	ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
	float scale = GameDialogManager::sharedInstance()->getScale();
	if (actionPanel->getActionMode() == ACTION_RALLY_POINT_MODE)
	{
		actionPanel->updateActionMode(ACTION_NORMAL_MODE);

		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);

		//remove travel path
		GameDialogManager::sharedInstance()->getHUD()->removeActiveRallyPath();
		
		//scale up rally button and translate buttons
		actionPanel->showMoveBtn(hkvVec2(-78.0f * scale, -180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.2f);
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isRallyBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			actionPanel->showRallyBtn(hkvVec2(-78.0f * scale, 0), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.2f);
		}
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isBuildBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
				actionPanel->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.2f); 
		}

		//dismiss sub panel
		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		RallySubMenuPanel* rallySubMenuPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
		rallySubMenuPanel->hideAll(0.0f);

		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
		GameDialogManager::sharedInstance()->getHUD()->removeRallySphereHalo();

	}
	else if (actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
	{
		performMoveAction();
		performRallyAction();
	}
	else if (actionPanel->getActionMode() == ACTION_BUILD_BRIDGE_MODE)
	{
		performBuildBridgesAction();
		performRallyAction();
	}
	else
	{
		actionPanel->updateActionMode(ACTION_RALLY_POINT_MODE);

		//scale up rally button and translate buttons
		float rallyBtnScale = 196.0f / 156.0f;

		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isRallyBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			actionPanel->showRallyBtn(hkvVec2(-98.0f * scale, 0), hkvVec2(rallyBtnScale, rallyBtnScale), true, 0.2f, 0.0f);
		}
		actionPanel->dismissMoveBtn(true, 0.0f);
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isBuildBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
				actionPanel->dismissBuildBtn(true, 0.2f);
		}

		//show rally sphere halo
		GameDialogManager::sharedInstance()->getHUD()->showRallySphereHalo(true);
		GameDialogManager::sharedInstance()->getHUD()->showRallySpherePath();

		//show sub panel
		RallySubMenuPanel* rallySubPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL); 
		if (GameDialogManager::sharedInstance()->getHUD()->getNodeSphere()->getRallySphere())
		{
			rallySubPanel->showBtn(ID_RALLY_SUB_PANEL_DELETE, 0.2f);
			rallySubPanel->showBtn(ID_RALLY_SUB_PANEL_CANCEL, 0.2f);
		}
		else
		{
			rallySubPanel->showBtn(ID_RALLY_SUB_PANEL_CANCEL, 0.2f);
		}
	}
	//this->displayRingWave();
}

bool GameManager::performBuildBridgesAction()
{
	ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
	float scale = GameDialogManager::sharedInstance()->getScale();
	if (actionPanel->getActionMode() == ACTION_BUILD_BRIDGE_MODE)
	{
		actionPanel->updateActionMode(ACTION_NORMAL_MODE);
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
		GameDialogManager::sharedInstance()->getHUD()->removePotentialBridges();

		//scale up build button and translate buttons
		actionPanel->showMoveBtn(hkvVec2(-78.0f * scale, -180.f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.2f);
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isRallyBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			actionPanel->showRallyBtn(hkvVec2(-78.0f * scale, 0.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.2f);
		}
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isBuildBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
				actionPanel->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			else
				actionPanel->dismissBuildBtn(true, 0.0f);
		}

		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		BuildSubMenuPanel* buildSubMenuPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL);
		buildSubMenuPanel->hideAll(0.0f);

		GameDialogManager::sharedInstance()->getHUD()->removeBuildSphereHalo();

		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
	}
	else if (actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
	{
		performMoveAction();
		performBuildBridgesAction();
	}
	else if (actionPanel->getActionMode() == ACTION_RALLY_POINT_MODE)
	{
		performRallyAction();
		performBuildBridgesAction();
	}
	else
	{
		actionPanel->updateActionMode(ACTION_BUILD_BRIDGE_MODE);

		//scale up build button and translate buttons
		float buildBtnScale = 196.0f / 156.0f;

		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isBuildBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() > 0)
				actionPanel->showBuildBtn(hkvVec2(-98.0f * scale, 180.0f * scale), hkvVec2(buildBtnScale, buildBtnScale), true, 0.2f, 0.0f);
		}
		actionPanel->dismissMoveBtn(true, 0.0f);
		if ((LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
			&&
			TutorialsHandler::sharedInstance().isRallyBtnRevealed())
			||
			LevelManager::sharedInstance().getCurrentLevel()->getNumber() > 1)
		{
			actionPanel->dismissRallyBtn(true, 0.0f);
		}

		BuildSubMenuPanel* buildSubPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL);
		buildSubPanel->showBtn(ID_BUILD_SUB_PANEL_CANCEL, 0.2f);

		//show potential bridges
		GameDialogManager::sharedInstance()->getHUD()->setPotentialBridges(this->pickedNodeSphere->findPotentialBridges());
	}

	return false;
}

void GameManager::performSacrificeAction(NodeSphere* actionSphere, Team* team)
{
	//this->pickedNodeSphere->getSelectionHalo()->updateActionMode(ACTION_SACRIFICE_MODE);
	//if (vdynamic_cast<SacrificePit*>(actionSphere))
	//{
	//	vector<Orb*> sacrificePitChildOrbs = actionSphere->getChildOrbs(team);
	//	for (int orbIndex = 0; orbIndex != sacrificePitChildOrbs.size(); orbIndex++)
	//	{
	//		sacrificePitChildOrbs[orbIndex]->sacrifice();
	//	}
	//}
}

void GameManager::makeOrbTravel(Orb* orb, vector<NodeSphere*> nodesPath)
{
	if (orb->getStatus() == ORB_STATUS_PENDING)
		return;

	//make orb travel along the path
	if (nodesPath.size() > 1)
	{
		NodeSphere* startSphere = nodesPath[0];
		orb->findTravelPath(nodesPath, orb->getTravelPath(), orb->GetPosition());
		orb->setStatus(ORB_STATUS_PENDING);
		
		startSphere->addPendingTravelingOrb(orb);
	}
}

vector<Orb*> GameManager::makeOrbsTravel(Team* team, NodeSphere* startSphere, NodeSphere* endSphere, int number)
{
	vector<Orb*> pendingOrbs;
	vector<NodeSphere*> nodesPath;
	if (startSphere == endSphere || !this->findNodesPath(startSphere, endSphere, nodesPath))
		return pendingOrbs;

	//make orbs travel along the path
	if (nodesPath.size() > 1)
	{
		NodeSphere* startSphere = nodesPath[0];
		vector<Orb*>& childOrbs = startSphere->getOrbsForTeam(team);

		int orbsCount = childOrbs.size();
		if (number > orbsCount) //this case should never happen
			number = orbsCount;
		pendingOrbs.reserve(number);
		for (int orbIndex = 0; orbIndex != number; orbIndex++)
		{
			Orb* orb = childOrbs[orbIndex];
			if (orb->getStatus() == ORB_STATUS_ORBITING)
			{
				makeOrbTravel(orb, nodesPath);
				pendingOrbs.push_back(orb);
			}
			else
			{
				if ((number + 1) < orbsCount)
					number++;
			}
		}
	}

	return pendingOrbs;
}

bool GameManager::findNodesPath(NodeSphere* startSphere, NodeSphere* endSphere, vector<NodeSphere*> &path)
{
	return Pathfinding::findPath(nodeSpheres, path, startSphere, endSphere, true);
}

void GameManager::unloadLevelElements()
{
	//Remove 2d overlays
	removeSelectionHalo();

	deInitNodeSpheres();
	deInitBridges();
	releaseTeams();

	VisParticleGroupManager_cl::GlobalManager().Instances().Purge();
}

void GameManager::resetGame()
{
	//reset levels
	PersistentDataManager::sharedInstance().setReachedLevel(0);

	//reset achievements
	vector<Achievement*> &allAchievements = AchievementManager::sharedInstance().getAchievements();
	for (int achievementIndex = 0; achievementIndex != allAchievements.size(); achievementIndex++)
	{
		Achievement* achievement = allAchievements[achievementIndex];
		achievement->setUnlocked(false);
	}

	//TODO delete all saved games
}

NodeSphere* GameManager::findNodeSphereBy3DPosition(hkvVec3 position)
{
	for (int nodeSphereIndex = 0; nodeSphereIndex != nodeSpheres.size(); nodeSphereIndex++)
	{
		if (nodeSpheres[nodeSphereIndex]->GetPosition() == position)
			return nodeSpheres[nodeSphereIndex];
	}

	return NULL;
}