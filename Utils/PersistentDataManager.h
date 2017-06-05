#pragma once

//categories
#define PERSISTENT_DATA_CATEGORY_OPTIONS "PERSISTENT_DATA_CATEGORY_OPTIONS"
#define PERSISTENT_DATA_CATEGORY_LEVELS "PERSISTENT_DATA_CATEGORY_LEVELS"
#define PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS "PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS"
#define PERSISTENT_DATA_CATEGORY_HELP_MENU "PERSISTENT_DATA_CATEGORY_HELP_MENU"

//fields
#define FIELD_MUSIC "FIELD_MUSIC"
#define FIELD_SOUND "FIELD_SOUND"
#define FIELD_REACHED_LEVEL "FIELD_REACHED_LEVEL"
#define KILLED_ENEMY_ORBS_COUNT "KILLED_ENEMY_ORBS_COUNT"
#define SACRIFIED_ORBS_COUNT "SACRIFIED_ORBS_COUNT"
#define ACHIEVEMENT_KILL_LVL_1 "ACHIEVEMENT_KILL_LVL_1"
#define ACHIEVEMENT_KILL_LVL_2 "ACHIEVEMENT_KILL_LVL_2"
#define ACHIEVEMENT_KILL_LVL_3 "ACHIEVEMENT_KILL_LVL_3"
#define ACHIEVEMENT_SACRIFICE_LVL_1 "ACHIEVEMENT_SACRIFICE_LVL_1"
#define ACHIEVEMENT_SACRIFICE_LVL_2 "ACHIEVEMENT_SACRIFICE_LVL_2"
#define ACHIEVEMENT_SACRIFICE_LVL_3 "ACHIEVEMENT_SACRIFICE_LVL_3"
#define ACHIEVEMENT_PRIMARY_OBJECTIVES "ACHIEVEMENT_PRIMARY_OBJECTIVES"
#define ACHIEVEMENT_SECONDARY_OBJECTIVES "ACHIEVEMENT_SECONDARY_OBJECTIVES"
#define ACHIEVEMENT_XP_LVL_10 "ACHIEVEMENT_XP_LVL_10"
#define ACHIEVEMENT_XP_LVL_25 "ACHIEVEMENT_XP_LVL_25"
#define ACHIEVEMENT_XP_LVL_50 "ACHIEVEMENT_XP_LVL_50"
#define HELP_MENU_LAST_PAGE "HELP_MENU_LAST_PAGE"

class PersistentDataManager
{
public:
	~PersistentDataManager(void) {};

	static PersistentDataManager& sharedInstance(){return instance;};

	//music/sound
	bool toggleMusic();
	bool toggleSound();
	bool isMusicActive();
	bool isSoundActive();

	//achievements
	void unlockAchievement(const char* achievementFieldName);
	bool isAchievementUnlocked(const char* achievementFieldName);
	int getKilledEnemyOrbsCount();
	void incrementKilledEnemyOrbsCount();
	int getSacrifiedOrbsCount();
	void incrementSacrifiedOrbsCount();

	//Levels
	int getReachedLevel();
	void setReachedLevel(int level);

	//help menu
	int getHelpMenuLastReachedPage();
	void setHelpMenuLastReachedPage(int page);

private:
	PersistentDataManager(void) {};

	static PersistentDataManager instance;
};