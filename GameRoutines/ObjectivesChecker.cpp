#include "OrbsPluginPCH.h"
#include "ObjectivesChecker.h"
#include "GameManager.h"

ObjectivesChecker ObjectivesChecker::instance;

void ObjectivesChecker::checkObjectives(Level* level)
{
	if (level->isComplete()) //level is already complete no need to check objectives
		return;

	int levelResult;
	vector<Objective*> &levelObjectives = level->getObjectives();
	switch (level->getNumber())
	{
	case 1:
		{
			levelResult = checkLevel1Objectives(levelObjectives);
			break;
		}
	case 2:
		{
			levelResult = checkLevel2Objectives(levelObjectives);
			break;
		}
	case 3:
		{
			levelResult = checkLevel3Objectives(levelObjectives);
			break;
		}
	case 4:
		{
			levelResult = checkLevel4Objectives(levelObjectives);
			break;
		}
	default:
		{
			break;
		}
	}

	if (levelResult == RESULT_VICTORY)
	{
		level->setComplete(true);
		GameManager::GlobalManager().endLevel(true);
		PersistentDataManager::sharedInstance().setReachedLevel(level->getNumber());
	}
	else if (levelResult == RESULT_DEFEAT)
	{
		level->setComplete(true);
		GameManager::GlobalManager().endLevel(false);
	}
}

int ObjectivesChecker::checkLevel1Objectives(const vector<Objective*> &objectives)
{
	VASSERT_MSG(objectives.size() == 4, "objectives count differs from what is expected");
	//objectives have to be completed in order so return if one objective is not complete
	Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
	if (!objectives[0]->isComplete()) //check if ten orbs have moved to second sphere
	{
		if (!objectives[0]->isHidden())
		{
			NodeSphere* secondSphere = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1150,0,400)); //second sphere
			VASSERT_MSG(secondSphere, "sphere not found at position x:%f y:%f z:%f", 1150, 0, 400);

			if (secondSphere->getOrbsCountForTeam(defaultTeam) >= 10)
			{
				objectives[0]->setComplete();
				objectives[1]->setHidden(false);
				TutorialsHandler::sharedInstance().showTutorialGroup4();
			}
		}
		return RESULT_RUNNING;
	}
	if (!objectives[1]->isComplete()) //check if rally point has been set
	{
		if (!objectives[1]->isHidden())
		{
			NodeSphere* firstSphere = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(0,0,400)); //second sphere
			NodeSphere* secondSphere = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1150,0,400)); //second sphere
			VASSERT_MSG(secondSphere, "sphere not found at position x:%f y:%f z:%f", 1150, 0, 400);

			if (firstSphere->getRallySphere() == secondSphere)
			{
				objectives[1]->setComplete();
				objectives[2]->setHidden(false);
				TutorialsHandler::sharedInstance().showTutorialGroup5();
			}
		}
		return RESULT_RUNNING;
	}
	if (!objectives[2]->isComplete()) //check if meta-orb contains 20 orbs
	{
		if (!objectives[2]->isHidden())
		{
			NodeSphere* secondSphere = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1150,0,400)); //second sphere
			VASSERT_MSG(secondSphere, "sphere not found at position x:%f y:%f z:%f", 1150, 0, 400);

			if (secondSphere->getGenerationMetaOrb() && secondSphere->getGenerationMetaOrb()->getCurrentOrbsValue() >= 20)
			{
				objectives[2]->setComplete();
				objectives[3]->setHidden(false);
				GameManager::GlobalManager().lockInputEvents();
				TutorialsHandler::sharedInstance().showTutorialGroup6();
			}
		}
		return RESULT_RUNNING;
	}
	if (!objectives[3]->isComplete()) //check if 3rd sphere contains at least 20 orbs
	{
		if (!objectives[3]->isHidden())
		{
			NodeSphere* thirdSphere = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(2500,600,400)); //second sphere
			VASSERT_MSG(thirdSphere, "sphere not found at position x:%f y:%f z:%f", 2500, 600, 400);

			if (thirdSphere->getTeam() == TeamManager::sharedInstance().getDefaultTeam())
			{
				objectives[3]->setComplete();
				return RESULT_VICTORY;
			}
		}
		return RESULT_RUNNING;
	}

	return RESULT_VICTORY;
}

int ObjectivesChecker::checkLevel2Objectives(const vector<Objective*> &objectives)
{
	NodeSphere* s4 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(2500,900,400)); //s4
	NodeSphere* s5 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(2500,-700,400)); //s5
	NodeSphere* s6 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(4000,0,400)); //s6
	NodeSphere* s7 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(4800,1400,400)); //s7
	if ((s4->getTeamId() == TEAM_1) &&
		(s5->getTeamId() == TEAM_1) &&
		(s6->getTeamId() == TEAM_1) &&
		(s7->getTeamId() == TEAM_1))
		return RESULT_VICTORY;
	else
		return RESULT_RUNNING;
}

int ObjectivesChecker::checkLevel3Objectives(const vector<Objective*> &objectives)
{
	//check if player has lost
	vector<NodeSphere*> &allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	bool bDefaultPlayerControlsOneSphere = false;
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		if (allNodeSpheres[sphereIndex]->getTeam() == TeamManager::sharedInstance().getDefaultTeam())
		{
			bDefaultPlayerControlsOneSphere = true;
			break;
		}
	}

	if (!bDefaultPlayerControlsOneSphere)
		return RESULT_DEFEAT;

	//check objectives
	if (!objectives[0]->isComplete()) //check if wave 1 has ended
	{
		if (!objectives[0]->isHidden())
		{
			if (AIManager::sharedInstance().isWave1Ended())
			{
				objectives[0]->setComplete();
				objectives[1]->setHidden(false);
			}
		}
		return RESULT_RUNNING;
	}
	if (!objectives[1]->isComplete()) //check if wave 2 has ended
	{
		if (!objectives[1]->isHidden())
		{
			if (AIManager::sharedInstance().isWave2Ended())
			{
				objectives[1]->setComplete();
				objectives[2]->setHidden(false);
			}
		}
		return RESULT_RUNNING;
	}
	if (!objectives[2]->isComplete()) //check if wave 3 has ended
	{
		if (!objectives[2]->isHidden())
		{
			if (AIManager::sharedInstance().isWave3Ended())
			{
				objectives[2]->setComplete();
				return RESULT_VICTORY;
			}
		}
		return RESULT_RUNNING;
	}

	return RESULT_VICTORY;
}

int ObjectivesChecker::checkLevel4Objectives(const vector<Objective*> &objectives)
{ 
	if (!objectives[0]->isComplete()) //check if player discovered the sacrifice altar
	{
		SacrificePit* sacrificePit = (SacrificePit*) GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(0, 0, 400));
		int iPlayerOrbCountInSacrificePit = sacrificePit->getOrbsCountForTeam(TeamManager::sharedInstance().getDefaultTeam());
		if (iPlayerOrbCountInSacrificePit > 0)
		{
			objectives[0]->setComplete();
			objectives[1]->setHidden(false);

			MiscPanel* pMiscPanel = GameDialogManager::sharedInstance()->getHUD()->getMiscPanel();
			pMiscPanel->buildSacrifiedOrbsCount();
			pMiscPanel->showInfo(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT);
			pMiscPanel->buildAndStackNewObjectivesAlert();
/*
			CallFuncObject* pShowNextAlertAction = new CallFuncObject(pMiscPanel, (CallFunc)(&MiscPanel::showNextAlert), MISC_PANEL_ALERT_DURATION);
			CallFuncHandler::sharedInstance().addCallFuncInstance(pShowNextAlertAction);
			pShowNextAlertAction->executeFunc();*/
		}
	}
	else if (!objectives[1]->isComplete()) //check if player discovered the sacrifice altar
	{
		Team* pDefaultTeam = TeamManager::sharedInstance().getDefaultTeam();
		Team* pTeam2 = TeamManager::sharedInstance().findTeamById(TEAM_2);
		Team* pTeam3 = TeamManager::sharedInstance().findTeamById(TEAM_3);

		int iPlayerSacrifiedOrbCount = pDefaultTeam->getSacrifiedOrbCount();
		int iTeam2SacrifiedOrbCount = pTeam2->getSacrifiedOrbCount();
		int iTeam3SacrifiedOrbCount = pTeam3->getSacrifiedOrbCount();

		if (iPlayerSacrifiedOrbCount >= 50)
		{
			objectives[1]->setComplete();
			return RESULT_VICTORY;
		}
		if (iTeam2SacrifiedOrbCount >= 50)
			return RESULT_DEFEAT;
		if (iTeam3SacrifiedOrbCount >= 50)
			return RESULT_DEFEAT;
	}

	//secondary objective
	if (!objectives[2]->isComplete())
	{
		Team* pTeam2 = TeamManager::sharedInstance().findTeamById(TEAM_2);
		Team* pTeam3 = TeamManager::sharedInstance().findTeamById(TEAM_3);

		vector<NodeSphere*> &vAllNodeSpheres = GameManager::GlobalManager().getNodeSpheres();

		bool bEnemyControlsAtLeastOneSphere = false;
		for (int iSphereIndex = 0; iSphereIndex != vAllNodeSpheres.size(); iSphereIndex++)
		{
			NodeSphere* pNodeSphere = vAllNodeSpheres[iSphereIndex];
			if (pNodeSphere->getTeam() == pTeam2 || pNodeSphere->getTeam() == pTeam3)
			{
				bEnemyControlsAtLeastOneSphere = true;
				break;
			}
		}

		if (!bEnemyControlsAtLeastOneSphere)
		{
			objectives[2]->setComplete();
		}
	}

	return RESULT_RUNNING;
}


int ObjectivesChecker::checkLevel5Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel6Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel7Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel8Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel9Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel10Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel11Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel12Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel13Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel14Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel15Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel16Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel17Objectives(const vector<Objective*> &objectives){ return true; }
int ObjectivesChecker::checkLevel18Objectives(const vector<Objective*> &objectives){ return true; }