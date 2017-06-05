#pragma once

class Level;
struct IntroText;
class Objective;
class Achievement;

class Level : public VRefCounter
{
public:
	Level() : VRefCounter(),
		number(0), 
		title(""), 
		complete(false), 
		locked(true),
		killedEnemyOrbsCount(0),
		team1SacrifiedOrbsCount(0),
		team2SacrifiedOrbsCount(0),
		team3SacrifiedOrbsCount(0),
		team4SacrifiedOrbsCount(0)
	{};
	Level(const Level &other) : VRefCounter(other),
		number(other.number), 
		title(other.title), 
		complete(other.complete), 
		locked(other.locked),
		killedEnemyOrbsCount(other.killedEnemyOrbsCount),
		team1SacrifiedOrbsCount(other.team1SacrifiedOrbsCount),
		team2SacrifiedOrbsCount(other.team2SacrifiedOrbsCount),
		team3SacrifiedOrbsCount(other.team3SacrifiedOrbsCount),
		team4SacrifiedOrbsCount(other.team4SacrifiedOrbsCount),
		intros(other.intros),
		objectives(other.objectives),
		parties(other.parties){};
	~Level();

	void init(int number, string title, vector<IntroText> intros, vector<Objective*> objectives, vector<TeamParty*> parties);

	int getNumber(){return this->number;};
	string getTitle(){return this->title;};
	bool isComplete();
	void setComplete(bool complete){this->complete = complete;};
	bool isLocked();
	vector<IntroText> &getIntros(){return this->intros;};
	vector<Objective*> &getObjectives(){return this->objectives;};
	vector<TeamParty*> &getParties(){return this->parties;};

	void addKilledEnemyOrb(){this->killedEnemyOrbsCount++;};
	void addTeam1SacrifiedOrb(){this->team1SacrifiedOrbsCount++;};
	void addTeam2SacrifiedOrb(){this->team2SacrifiedOrbsCount++;};
	void addTeam3SacrifiedOrb(){this->team3SacrifiedOrbsCount++;};
	void addTeam4SacrifiedOrb(){this->team4SacrifiedOrbsCount++;};

	void clearGameConstantsInfos();

	//getters
	int getKilledEnemyOrbsCount(){return this->killedEnemyOrbsCount;};
	int getTeam1SacrifiedOrbsCount(){return this->team1SacrifiedOrbsCount;};
	int getTeam2SacrifiedOrbsCount(){return this->team2SacrifiedOrbsCount;};
	int getTeam3SacrifiedOrbsCount(){return this->team3SacrifiedOrbsCount;};
	int getTeam4SacrifiedOrbsCount(){return this->team4SacrifiedOrbsCount;};

private:
	int number;
	string title;
	bool complete;
	bool locked; //is the level locked?

	//infos about some various game constants
	int killedEnemyOrbsCount;
	int team1SacrifiedOrbsCount;
	int team2SacrifiedOrbsCount;
	int team3SacrifiedOrbsCount;
	int team4SacrifiedOrbsCount;
	
	//intros
	vector<IntroText> intros;

	//objectives
	vector<Objective*> objectives;

	//parties and teams
	vector<TeamParty*> parties;
};

class LevelManager : public ResourcePreloader
{
public:
	~LevelManager();
	void preloadStringsFiles();

	void buildLevelsFromXML();

	static LevelManager& sharedInstance() {return instance;};

	vector<Level*> getLevels(){return this->levels;};
	Level* getCurrentLevel(){return this->currentLevel;};
	void setCurrentLevel(Level* level){if (level) this->currentLevel = new Level(*level); else this->currentLevel = NULL;};
	void setCurrentLevelByNumber(int levelNumber);

	int getPrimaryObjectivesCount();
	int getSecondaryObjectivesCount();
	int getCompletedPrimaryObjectivesCount();
	int getCompletedSecondaryObjectivesCount();

private:
	LevelManager();

	static LevelManager instance;

	Level* currentLevel;
	vector<Level*> levels;

	TiXmlDocument levelsDoc;
};

class AchievementManager : public ResourcePreloader
{
public:
	~AchievementManager();
	static AchievementManager& sharedInstance() {return instance;};

	void preloadStringsFiles();

	void buildAchievementsFromXML();

	vector<Achievement*>& getAchievements(){return this->achievements;};
	Achievement* getAchievementForTag(string tag);
	VTextureObject* getAchievementIconForTag(string tag);

	int getAchievementCurrentValue(string achievementID);
	int getAchievementMaxValue(string achievementID);

private:
	AchievementManager();

	static AchievementManager instance;

	vector<Achievement*> achievements;
	bool achievementsBuilt;

	TiXmlDocument achievementsDoc;
};

struct IntroText
{
	int group;
	string text;
};

class Objective : public VRefCounter
{
public:
	Objective() : VRefCounter(), number(0), hidden(false), alreadyDisplayed(false), complete(false), text(""), primary(false){};
	~Objective() {};

	void init(int number, string text, bool primary, bool hidden);

	bool isContainedInObjectivesList(const vector<int>& objectivesNumbers);

	int getNumber(){return this->number;};
	void setComplete();
	bool isComplete() {return this->complete;};
	string getText(){return this->text;};
	bool isPrimary(){return this->primary;};
	bool isHidden(){return this->hidden;};
	void setHidden(bool hidden){this->hidden = hidden;};
	bool hasBeenAlreadyDisplayed(){return this->alreadyDisplayed;};
	void setAlreadyDisplayed(bool displayed){this->alreadyDisplayed = displayed;};

private:
	int number;
	bool hidden; //is the objective hidden waiting for an event to happen
	bool alreadyDisplayed;
	bool complete;
	string text;
	bool primary; //is the objective a primary objective
};

#define ACHIEVEMENT_KILL_LVL_1 "ACHIEVEMENT_KILL_LVL_1"
#define ACHIEVEMENT_KILL_LVL_2 "ACHIEVEMENT_KILL_LVL_2"
#define ACHIEVEMENT_KILL_LVL_3 "ACHIEVEMENT_KILL_LVL_3"
#define ACHIEVEMENT_SACRIFICE_LVL_1 "ACHIEVEMENT_SACRIFICE_LVL_1"
#define ACHIEVEMENT_SACRIFICE_LVL_2 "ACHIEVEMENT_SACRIFICE_LVL_2"
#define ACHIEVEMENT_SACRIFICE_LVL_3 "ACHIEVEMENT_SACRIFICE_LVL_3"
#define ACHIEVEMENT_PRIMARY_OBJECTIVES "ACHIEVEMENT_PRIMARY_OBJECTIVES"
#define ACHIEVEMENT_SECONDARY_OBJECTIVES "ACHIEVEMENT_SECONDARY_OBJECTIVES"

class Achievement : public VRefCounter
{
public:
	Achievement() : VRefCounter(), tag(""), title(""), text(""), unlocked(false){};
	~Achievement() {};

	void init(string tag, string title, string text, bool unlocked, string iconPath);

	string getTag(){return this->tag;};
	string getTitle(){return this->title;};
	string getDescription(){return this->text;};
	void setUnlocked(bool unlocked){this->unlocked = unlocked;};
	bool isUnlocked(){return this->unlocked;};
	string getIconPath(){return this->iconPath;};

private:
	string tag;
	string title;
	string text;
	bool unlocked;
	string iconPath;
};