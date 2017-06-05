#include "OrbsPluginPCH.h"
#include "AIManager.h"
#include "GameManager.h"


AIManager AIManager::instance;

void AIManager::OneTimeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin += this;

	initVariables();
}

void AIManager::OneTimeDeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void AIManager::initVariables()
{
	int levelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
	switch (levelNumber)
	{
	case 3:
		{
			startingOrbsCount = 0;
			wave1TimerShown = false;
			wave1Sent = false;
			wave2Sent = false;
			wave3Sent = false;
			wave1Ended = false;
			wave2Ended = false;
			wave3Ended = false;
			wave1EndTime = 0;
			wave2EndTime = 0;
			break;
		}
	case 4:
		{
			sacrificePitDiscovered = false;
			team2NextMoveTime = 0;
			team3NextMoveTime = 0;
		}
	default:
		break;
	}
}

void AIManager::handleAI()
{
	int levelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
	switch (levelNumber)
	{
	case 3:
		{
			handleAIForLevel3();
			break;
		}
	case 4:
		{
			handleAIForLevel4();
			break;
		}
	default:
		break;
	}
}

void AIManager::handleAIForLevel3()
{
	if (!GameManager::GlobalManager().hasGameStarted())
		return;

	NodeSphere* nexus1 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(0,0,400));
	NodeSphere* s1 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(200,800,400));
	NodeSphere* s2 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(800,400,400));
	NodeSphere* s3 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1800,300,400));
	NodeSphere* s4 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1500,1200,400));
	NodeSphere* nexus2 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(4500,1800,400)); //the sphere sending orbs

	int gameTime = GameTimer::sharedInstance().getTimeAsSecs();
	float exactTime = GameTimer::sharedInstance().getExactTime();

	if (!wave1Sent && startingOrbsCount == 0)
	{
		startingOrbsCount = nexus2->getTeam()->getTotalOrbsCount();
	}

	if (!wave1Sent && !wave1TimerShown)
	{
		wave1TimerShown = true;
		//MessageHandler::sharedInstance().showCountdownTimerMessage(StringsXmlHelper::sharedInstance().getStringForTag("next_wave_remaining_time"), 5);
		MiscPanel* pMiscPanel = GameDialogManager::sharedInstance()->getHUD()->getMiscPanel();
		pMiscPanel->buildCountdownTimerMessage(StringsXmlHelper::sharedInstance().getStringForTag("next_wave_remaining_time"), 180);
		pMiscPanel->showInfo(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
	}

	if (!wave1Sent && gameTime >= 5) //3 min
	{
		wave1Sent = true;
		
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s3, LVL_3_WAVE_1_ORBS_COUNT); //send 50 orbs to s3
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s4, LVL_3_WAVE_1_ORBS_COUNT); //send 50 orbs to s4
	}
	if (wave1Ended && !wave2Sent && gameTime >= wave1EndTime + 180) //6 min
	{
		wave2Sent = true;
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s3, LVL_3_WAVE_2_ORBS_COUNT); //send 100 orbs to s3
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s4, LVL_3_WAVE_2_ORBS_COUNT); //send 100 orbs to s4
	}
	if (wave2Ended && !wave3Sent && gameTime >= wave2EndTime + 180) //9 min
	{
		wave3Sent = true;
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s3, LVL_3_WAVE_3_ORBS_COUNT); //send 200 orbs to s3
		GameManager::GlobalManager().makeOrbsTravel(nexus2->getTeam(), nexus2, s4, LVL_3_WAVE_3_ORBS_COUNT); //send 200 orbs to s4
	}

	//check the states of waves
	if (wave1Sent && !wave1Ended)
	{
		int currentOrbsCount = nexus2->getTeam()->getTotalOrbsCount();

		if (currentOrbsCount <= startingOrbsCount - 2 * LVL_3_WAVE_1_ORBS_COUNT)
		{
			wave1Ended = true;
			wave1EndTime = GameTimer::sharedInstance().getTimeAsSecs();
			startingOrbsCount = currentOrbsCount;
			//MessageHandler::sharedInstance().showCountdownTimerMessage(StringsXmlHelper::sharedInstance().getStringForTag("next_wave_remaining_time"), 180);
		}
	}
	else if (wave2Sent && !wave2Ended)
	{
		int currentOrbsCount = nexus2->getTeam()->getTotalOrbsCount();
		if (currentOrbsCount <= startingOrbsCount - 2 * LVL_3_WAVE_2_ORBS_COUNT)
		{
			wave2Ended = true;
			wave2EndTime = GameTimer::sharedInstance().getTimeAsSecs();
			startingOrbsCount = currentOrbsCount;
			//MessageHandler::sharedInstance().showCountdownTimerMessage(StringsXmlHelper::sharedInstance().getStringForTag("next_wave_remaining_time"), 180);
		}
	}
	else if (wave3Sent && !wave3Ended)
	{
		int currentOrbsCount = nexus2->getTeam()->getTotalOrbsCount();
		if (currentOrbsCount <= startingOrbsCount - 2 * LVL_3_WAVE_3_ORBS_COUNT)
		{
			wave3Ended = true;
		}
	}

	TeamManager& teamManager = TeamManager::sharedInstance();
	Team* team2 = teamManager.findTeamById(TEAM_2);
	Team* defaultTeam = teamManager.getDefaultTeam();

	//s4
	vector<Orb*> &assaultingOrbsOnS4 = s4->getOrbsForTeam(team2);
	this->tryToTakeControlAndMoveOn(assaultingOrbsOnS4, team2, defaultTeam, s4, s2, true);

	//s3
	vector<Orb*> &assaultingOrbsOnS3 = s3->getOrbsForTeam(team2);
	this->tryToTakeControlAndMoveOn(assaultingOrbsOnS3, team2, defaultTeam, s3, s2, true);

	//s2
	vector<Orb*> &assaultingOrbsOnS2 = s2->getOrbsForTeam(team2);
	if (s1->getTeam() == defaultTeam) //attack s1
		this->tryToTakeControlAndMoveOn(assaultingOrbsOnS2, team2, defaultTeam, s2, s1, true);
	else //s1 is under team2's control, attack nexus
		this->tryToTakeControlAndMoveOn(assaultingOrbsOnS2, team2, defaultTeam, s2, nexus1, true);

	//s1
	vector<Orb*> &assaultingOrbsOnS1 = s1->getOrbsForTeam(team2);
	this->tryToTakeControlAndMoveOn(assaultingOrbsOnS1, team2, defaultTeam, s1, nexus1, false);

	//nexus1
	vector<Orb*> &assaultingOrbsOnNexus1 = nexus1->getOrbsForTeam(teamManager.findTeamById(TEAM_2));
	this->tryToTakeControlAndMoveOn(assaultingOrbsOnNexus1, team2, defaultTeam, nexus1, NULL, false);
}

void AIManager::handleAIForLevel4()
{
	int gameTime = GameTimer::sharedInstance().getTimeAsSecs();
	SacrificePit* sacrificePit = (SacrificePit*) GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(0, 0, 400));
	NodeSphere* s3 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(-565, 565, 400));
	NodeSphere* s4 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(-1365, 565, 400));
	NodeSphere* s5 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(-565, 1365, 400));
	NodeSphere* s6 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(-565, 565, 400));
	NodeSphere* s7 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(565, 1365, 400));
	NodeSphere* s8 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(-565, 1365, 400));

	Team* team2 = TeamManager::sharedInstance().findTeamById(TEAM_2);
	Team* team3 = TeamManager::sharedInstance().findTeamById(TEAM_3);

	if (!sacrificePitDiscovered)
	{
		int alliedOrbsCountInSacrificePit = sacrificePit->getOrbsCountForTeam(TeamManager::sharedInstance().getDefaultTeam());
		if (alliedOrbsCountInSacrificePit > 0)
		{
			this->sacrificePitDiscovered = true;
			team2NextMoveTime = gameTime;
			team3NextMoveTime = gameTime + 20;
		}
	}

	if (!sacrificePitDiscovered || gameTime < 60) //do nothing until 1 min passed or sacrifice altar discovered
		return;
	
	if (gameTime > team2NextMoveTime)
	{
		team2NextMoveTime = gameTime + 90;

		//make the move
		int orbsToSend = 10;
		if (s5->getOrbsCountForTeam(team2) >= (10 + orbsToSend)) //always keep 10 orbs defending the sphere
		{
			if (!sacrificePitDiscovered)
				moveOrbsToSphere(team2, s3, sacrificePit, orbsToSend);
			else
				moveOrbsToSphere(team2, s5, sacrificePit, orbsToSend);
		}
	}
	if (gameTime > team3NextMoveTime)
	{
		team3NextMoveTime = gameTime + 90;

		//make the move
		int orbsToSend = 10;
		if (s7->getOrbsCountForTeam(team3) >= (10 + orbsToSend)) //always keep 10 orbs defending the sphere
		{
			if (!sacrificePitDiscovered)
				moveOrbsToSphere(team2, s6, sacrificePit, orbsToSend);
			else
				moveOrbsToSphere(team3, s7, sacrificePit, orbsToSend);
		}			
	}
}

void AIManager::tryToTakeControlAndMoveOn(vector<Orb*> &assaultingOrbs, Team* assaultingTeam, Team* defendingTeam, NodeSphere* controlTargetSphere, NodeSphere* nextTravelSphere, bool setUpDefenseMetaOrb)
{
	vector<Orb*> &assaultingOrbsOnTargetSphere = controlTargetSphere->getOrbsForTeam(assaultingTeam);
	if (assaultingOrbsOnTargetSphere.size() > 0)
	{
		int defaultOrbsCountOnTargetSphere = controlTargetSphere->getOrbsCountForTeam(defendingTeam);

		if ((controlTargetSphere->getTeam() == defendingTeam || controlTargetSphere->getTeam()->isNeutral()) 
			&& defaultOrbsCountOnTargetSphere == 0  
			&& !controlTargetSphere->hasAtLeastOneMetaOrb()) //neutral team or team is controlled by player but none of his orbs are orbiting and no meta-orbs is alive
		{
			controlTargetSphere->takeControl(assaultingTeam, true, true);
		}
		else if (nextTravelSphere && controlTargetSphere->getTeam() == assaultingTeam && defaultOrbsCountOnTargetSphere == 0) //sphere is controlled by team2 and there is no enemy orb on it, jump to the next sphere (s2)
		{
			for (int assaultingOrbIndex = 0; assaultingOrbIndex != assaultingOrbs.size(); assaultingOrbIndex++)
			{
				Orb* assaultingOrb = assaultingOrbs[assaultingOrbIndex];

				vector<NodeSphere*> nodesPath;
				if (GameManager::GlobalManager().findNodesPath(controlTargetSphere, nextTravelSphere, nodesPath))
					GameManager::GlobalManager().makeOrbTravel(assaultingOrb, nodesPath);
			}
		}
	}
}

void AIManager::moveOrbsToSphere(Team* pTeam, NodeSphere* pStartSphere, NodeSphere* pEndSphere, int iNumOrbs)
{
	vector<NodeSphere*> vAiPath;
	if (GameManager::GlobalManager().findNodesPath(pStartSphere, pEndSphere, vAiPath))
	{
		vector<Orb*>& vOrbs = pStartSphere->getOrbsForTeam(pTeam);
		int iOrbsCount = (int) vOrbs.size();
		int iNumOrbsToSend = (iNumOrbs > iOrbsCount) ? iOrbsCount : iNumOrbs;

		for (int iOrbIndex = 0; iOrbIndex != iNumOrbsToSend; iOrbIndex++)
		{
			Orb* pOrb = vOrbs[iOrbIndex];
			pOrb->findTravelPath(vAiPath, pOrb->getTravelPath(), pOrb->GetPosition());
		} 
	}
}

void AIManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
		this->update(dt);
	}
}

void AIManager::update(float dt)
{
	handleAI();
}