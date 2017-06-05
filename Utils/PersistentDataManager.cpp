#include "OrbsPluginPCH.h"
#include "PersistentDataManager.h"

PersistentDataManager PersistentDataManager::instance;

bool PersistentDataManager::toggleMusic()
{
	bool music = Vision::PersistentData.GetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_MUSIC, true);
	music = !music;
	Vision::PersistentData.SetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_MUSIC, music);
	return music;
}

bool PersistentDataManager::toggleSound()
{
	bool sound = Vision::PersistentData.GetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_SOUND, true);
	sound = !sound;
	Vision::PersistentData.SetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_SOUND, sound);
	return sound;
}

bool PersistentDataManager::isMusicActive()
{
	return Vision::PersistentData.GetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_MUSIC, true);
}

bool PersistentDataManager::isSoundActive()
{
	return Vision::PersistentData.GetBoolean(PERSISTENT_DATA_CATEGORY_OPTIONS, FIELD_SOUND, true);
}

void PersistentDataManager::unlockAchievement(const char* achievementFieldName)
{
	Vision::PersistentData.SetBoolean(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, achievementFieldName, true);
}

bool PersistentDataManager::isAchievementUnlocked(const char* achievementFieldName)
{
	return Vision::PersistentData.GetBoolean(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, achievementFieldName, false);
}

int PersistentDataManager::getKilledEnemyOrbsCount()
{
	return (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, KILLED_ENEMY_ORBS_COUNT, 0);
}

void PersistentDataManager::incrementKilledEnemyOrbsCount()
{
	int oldKilledEnemyOrbsCount = (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, KILLED_ENEMY_ORBS_COUNT, 0);
	Vision::PersistentData.SetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, KILLED_ENEMY_ORBS_COUNT, (float) (oldKilledEnemyOrbsCount + 1));
}

int PersistentDataManager::getSacrifiedOrbsCount()
{
	return (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, SACRIFIED_ORBS_COUNT, 0);
}

void PersistentDataManager::incrementSacrifiedOrbsCount()
{
	int oldSacrifiedEnemyOrbsCount = (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, SACRIFIED_ORBS_COUNT, 0);
	Vision::PersistentData.SetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, SACRIFIED_ORBS_COUNT, (float) (oldSacrifiedEnemyOrbsCount + 1));
}

int PersistentDataManager::getReachedLevel()
{
	return (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, FIELD_REACHED_LEVEL, 1);
}

void PersistentDataManager::setReachedLevel(int level)
{
	int oldLevel = getReachedLevel();
	if (level > oldLevel)
		Vision::PersistentData.SetNumber(PERSISTENT_DATA_CATEGORY_ACHIEVEMENTS, FIELD_REACHED_LEVEL, (float) level);
}

int PersistentDataManager::getHelpMenuLastReachedPage()
{
	return 2;
	return (int) Vision::PersistentData.GetNumber(PERSISTENT_DATA_CATEGORY_HELP_MENU, HELP_MENU_LAST_PAGE, 1);
}

void PersistentDataManager::setHelpMenuLastReachedPage(int page)
{
	int oldReachedPage = getHelpMenuLastReachedPage();
	if (page > oldReachedPage)
		Vision::PersistentData.SetNumber(PERSISTENT_DATA_CATEGORY_HELP_MENU, HELP_MENU_LAST_PAGE, (float) page);
}