#include "OrbsPluginPCH.h"
#include "GameObjects.h"

////////////////////////////////////LevelManager////////////////////////////////////
LevelManager LevelManager::instance;

LevelManager::LevelManager() : currentLevel(NULL) 
{

}

LevelManager::~LevelManager()
{
	for (int levelIndex = 0; levelIndex != levels.size(); levelIndex++)
	{
		V_SAFE_RELEASE(levels[levelIndex]);
	}
}

void LevelManager::preloadStringsFiles()
{
	if (!stringsFilesLoaded)
	{
		string levelsFile = GetStringsFolder() + string("levels.xml");
		levelsDoc.LoadFile(levelsFile.c_str());
		ResourcePreloader::preloadStringsFiles();
	}
}

void LevelManager::buildLevelsFromXML()
{
	if (this->levels.size() != 0) //levels vector has already been populated
		return;

	int levelNumber = 1;
	for (TiXmlElement *levelNode = levelsDoc.RootElement()->FirstChildElement("level"); levelNode; levelNode = levelNode->NextSiblingElement("level"))
	{
		Level* level = new Level();
		level->AddRef();
		string levelTitle = string(XMLHelper::Exchange_String(levelNode,"text",NULL,false));

		//intro texts
		vector<IntroText> intros;

		//push the title of the level before the actual intro texts
		string strTitle = StringsXmlHelper::sharedInstance().getStringForTag("level") + string(" ") + stringFromInt(levelNumber) + string(" - ") + levelTitle;
		IntroText titleIntro;
		titleIntro.group = 0;
		titleIntro.text = strTitle;
		intros.push_back(titleIntro);

		//now push the intro texts
		for (TiXmlElement *introNode = levelNode->FirstChildElement("intro"); introNode; introNode = introNode->NextSiblingElement("intro"))
		{
			int group = 0;
			XMLHelper::Exchange_Int(introNode,"group",group,false);
			string introText = string(XMLHelper::Exchange_String(introNode,"text",NULL,false));
			IntroText intro;
			intro.group = group;
			intro.text = introText;
			intros.push_back(intro);
		}

		//objectives
		vector<Objective*> objectives;
		int objectiveNumber = 1;
		for (TiXmlElement *objectiveNode = levelNode->FirstChildElement("objective"); objectiveNode; objectiveNode = objectiveNode->NextSiblingElement("objective"))
		{
			Objective* objective = new Objective();
			objective->AddRef();
			string objectiveText = string(XMLHelper::Exchange_String(objectiveNode,"text",NULL,false));
			bool primary;
			bool hidden;
			XMLHelper::Exchange_Bool(objectiveNode,"primary",primary,false);
			XMLHelper::Exchange_Bool(objectiveNode,"hidden",hidden,false);
			objective->init(objectiveNumber, objectiveText, primary, hidden);
			objectives.push_back(objective);
			objectiveNumber++;
		}

		//parties
		vector<TeamParty*> parties;
		int partyNumber = 0;
		for (TiXmlElement *partyNode = levelNode->FirstChildElement("party"); partyNode; partyNode = partyNode->NextSiblingElement("party"))
		{
			TeamParty* teamParty = new TeamParty();
			teamParty->AddRef();
			int teamPartyId;
			XMLHelper::Exchange_Int(partyNode, "id", teamPartyId, false);
			vector<Team*> teamMates;
			for (TiXmlElement *teamNode = partyNode->FirstChildElement("team"); teamNode; teamNode = teamNode->NextSiblingElement("team"))
			{
				Team* team = new Team();
				team->AddRef();
				int teamId;
				XMLHelper::Exchange_Int(teamNode, "id", teamId, false);
				string teamName = string(XMLHelper::Exchange_String(teamNode,"name",NULL,false));
				team->init(teamId, teamName, teamParty);
				teamMates.push_back(team);
			}
			teamParty->init(teamPartyId, teamMates);
			parties.push_back(teamParty);

			partyNumber++;
		}

		level->init(levelNumber, levelTitle, intros, objectives, parties);
		levelNumber++;

		levels.push_back(level);
	}
}

void LevelManager::setCurrentLevelByNumber(int levelNumber)
{
	this->currentLevel = levels[levelNumber - 1];
}

int LevelManager::getPrimaryObjectivesCount()
{
	int primaryObjectivesCount = 0;
	vector<Objective*> objectives = this->currentLevel->getObjectives();

	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		if (objectives[objectiveIndex]->isPrimary())
			primaryObjectivesCount++;
	}
	
	return primaryObjectivesCount;
}

int LevelManager::getSecondaryObjectivesCount()
{
	int secondaryObjectivesCount = 0;
	vector<Objective*> objectives = this->currentLevel->getObjectives();

	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		if (!objectives[objectiveIndex]->isPrimary())
			secondaryObjectivesCount++;
	}
	
	return secondaryObjectivesCount;
}

int LevelManager::getCompletedPrimaryObjectivesCount()
{
	int completedPrimaryObjectivesCount = 0;
	vector<Objective*> objectives = this->currentLevel->getObjectives();

	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		Objective* objective = objectives[objectiveIndex];
		if (objective->isPrimary() && objective->isComplete())
			completedPrimaryObjectivesCount++;
	}

	return completedPrimaryObjectivesCount;
}

int LevelManager::getCompletedSecondaryObjectivesCount()
{
	int completedSecondaryObjectivesCount = 0;
	vector<Objective*> objectives = this->currentLevel->getObjectives();

	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		Objective* objective = objectives[objectiveIndex];
		if (!objective->isPrimary() && objective->isComplete())
			completedSecondaryObjectivesCount++;
	}

	return completedSecondaryObjectivesCount;
}

////////////////////////////////////AchievementManager////////////////////////////////////
AchievementManager AchievementManager::instance;

AchievementManager::AchievementManager() : achievementsBuilt(false)
{
	
}

AchievementManager::~AchievementManager()
{
	for (int achievementIndex = 0; achievementIndex != achievements.size(); achievementIndex++)
	{
		V_SAFE_RELEASE(achievements[achievementIndex]);
	}
}

void AchievementManager::preloadStringsFiles()
{
	if (!stringsFilesLoaded)
	{
		string achievementsFile = GetStringsFolder() + string("achievements.xml");
		achievementsDoc.LoadFile(achievementsFile.c_str());
		ResourcePreloader::preloadStringsFiles();
	}
}

void AchievementManager::buildAchievementsFromXML()
{
	if (!achievementsBuilt)
	{
		int levelNumber = 1;
		for (TiXmlElement *achievementNode = achievementsDoc.RootElement()->FirstChildElement("achievement"); achievementNode; achievementNode = achievementNode->NextSiblingElement("achievement"))
		{
			Achievement* achievement = new Achievement();
			achievement->AddRef();
			string achievementTag = string(XMLHelper::Exchange_String(achievementNode,"tag",NULL,false));
			string achievementTitle = string(XMLHelper::Exchange_String(achievementNode,"title",NULL,false));
			string achievementText = string(XMLHelper::Exchange_String(achievementNode,"text",NULL,false));
			string achievementIconPath = string(XMLHelper::Exchange_String(achievementNode,"icon",NULL,false));

			achievement->init(achievementTag, achievementTitle, achievementText, false, achievementIconPath);

			achievements.push_back(achievement);
		}
		achievementsBuilt = true;
	}

	//lock/unlock achievements
	for (int achievementIndex = 0; achievementIndex != achievements.size(); achievementIndex++)
	{
		Achievement* achievement = achievements[achievementIndex];
		bool achievementUnlocked = PersistentDataManager::sharedInstance().isAchievementUnlocked(achievement->getTag().c_str());
		achievement->setUnlocked(achievementUnlocked);
	}
}

Achievement* AchievementManager::getAchievementForTag(string tag)
{
	for (int achievementIndex = 0; achievementIndex != achievements.size(); achievementIndex++)
	{
		if (!achievements[achievementIndex]->getTag().compare(tag))
			return achievements[achievementIndex];
	}
	return NULL;
}

VTextureObject* AchievementManager::getAchievementIconForTag(string tag)
{
	Achievement* achievement = getAchievementForTag(tag);
	if (achievement)
	{
		if (!achievement->isUnlocked())
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_locked.png");

		if (!tag.compare(ACHIEVEMENT_KILL_LVL_1))
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_1.png");
		else if (!tag.compare(ACHIEVEMENT_KILL_LVL_2))
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_2.png");
		else if (!tag.compare(ACHIEVEMENT_KILL_LVL_3))
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_3.png");
		else if (!tag.compare(ACHIEVEMENT_SACRIFICE_LVL_1))
			;
		//return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_sacrifice1.png");
		else if (!tag.compare(ACHIEVEMENT_SACRIFICE_LVL_2))
			;
		//return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_sacrifice2.png");
		else if (!tag.compare(ACHIEVEMENT_SACRIFICE_LVL_3))
			;
		//return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_sacrifice3.png");
		else if (!tag.compare(ACHIEVEMENT_PRIMARY_OBJECTIVES))
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_all_primary.png");
		else if (!tag.compare(ACHIEVEMENT_SECONDARY_OBJECTIVES))
			return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_all_secondary.png");
	}
	return Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_locked.png"); //default
}

int AchievementManager::getAchievementCurrentValue(string achievementID)
{
	Achievement* achievement = getAchievementForTag(achievementID);

	if (!achievementID.compare(ACHIEVEMENT_KILL_LVL_1) ||
		!achievementID.compare(ACHIEVEMENT_KILL_LVL_2) ||
		!achievementID.compare(ACHIEVEMENT_KILL_LVL_3))
	{
		return PersistentDataManager::sharedInstance().getKilledEnemyOrbsCount();
	}
	else if (!achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_1) ||
			 !achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_2) ||
			 !achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_3))
	{
		return PersistentDataManager::sharedInstance().getSacrifiedOrbsCount();
	}
	else //other achievements do not require a gauge value
		return 0;
}

int AchievementManager::getAchievementMaxValue(string achievementID)
{
	if (!achievementID.compare(ACHIEVEMENT_KILL_LVL_1))
		return ACHIEVEMENT_KILL_1_COUNT;
	else if (!achievementID.compare(ACHIEVEMENT_KILL_LVL_2))
		return ACHIEVEMENT_KILL_2_COUNT;
	else if (!achievementID.compare(ACHIEVEMENT_KILL_LVL_3))
		return ACHIEVEMENT_KILL_3_COUNT;
	else if (!achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_1))
		return ACHIEVEMENT_SACRIFICE_1_COUNT;
	else if (!achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_2))
		return ACHIEVEMENT_SACRIFICE_2_COUNT;
	else if (!achievementID.compare(ACHIEVEMENT_SACRIFICE_LVL_3))
		return ACHIEVEMENT_SACRIFICE_3_COUNT;
	else //other achievements do not require a gauge value
		return 0;
}

////////////////////////////////////Level////////////////////////////////////
Level::~Level()
{
	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		V_SAFE_RELEASE(objectives[objectiveIndex]);
	}

	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		V_SAFE_RELEASE(parties[partyIndex]);
	}
}

void Level::init(int number, string title, vector<IntroText> intros, vector<Objective*> objectives, vector<TeamParty*> parties)
{
	this->number = number;
	this->title = title;
	this->intros = intros;
	this->objectives = objectives;
	this->parties = parties;
}

bool Level::isComplete()
{
	if (this->complete)
		return true;

	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		if (!objectives[objectiveIndex]->isComplete())
			return false;
	}

	this->complete = true;
	return true;
}

bool Level::isLocked()
{
	return this->number > PersistentDataManager::sharedInstance().getReachedLevel();
}

void Level::clearGameConstantsInfos()
{
	killedEnemyOrbsCount = 0;
	team1SacrifiedOrbsCount = 0;
	team2SacrifiedOrbsCount = 0;
	team3SacrifiedOrbsCount = 0;
	team4SacrifiedOrbsCount = 0;
}

////////////////////////////////////Objective////////////////////////////////////
void Objective::init(int number, string text, bool primary, bool hidden)
{
	this->number = number;
	this->text = text;
	this->primary = primary;
	this->hidden = hidden;
}

bool Objective::isContainedInObjectivesList(const vector<int>& objectivesNumbers)
{
	for (int numberIndex = 0; numberIndex != objectivesNumbers.size(); numberIndex++)
	{
		if (this->number == objectivesNumbers[numberIndex])
			return true;
	}
	return false;
}

void Objective::setComplete()
{
	if (this->complete == true)
		return;

	this->complete = true;
	GameDialogManager::sharedInstance()->getHUD()->completeObjective(this->number);
	MessageHandler::sharedInstance().showObjectiveCompletedMessage();
}

////////////////////////////////////Achievement////////////////////////////////////
void Achievement::init(string tag, string title, string text, bool unlocked, string iconPath)
{
	this->tag = tag;
	this->title = title;
	this->text = text;
	this->unlocked = unlocked;
	this->iconPath = iconPath;
}