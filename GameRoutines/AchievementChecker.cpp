#include "OrbsPluginPCH.h"
#include "AchievementChecker.h"

void AchievementChecker::checkAchievements()
{
	PersistentDataManager& persistentDataManager = PersistentDataManager::sharedInstance();

	//kill lvl 1
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_KILL_LVL_1))
	{
		int killedOrbsCount = persistentDataManager.getKilledEnemyOrbsCount();
		if (killedOrbsCount >= ACHIEVEMENT_KILL_1_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_KILL_LVL_1);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_KILL_LVL_1);
		}
	}
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_KILL_LVL_2))
	{
		int killedOrbsCount = persistentDataManager.getKilledEnemyOrbsCount();
		if (killedOrbsCount >= ACHIEVEMENT_KILL_2_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_KILL_LVL_2);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_KILL_LVL_2);
		}
	}
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_KILL_LVL_3))
	{
		int killedOrbsCount = persistentDataManager.getKilledEnemyOrbsCount();
		if (killedOrbsCount >= ACHIEVEMENT_KILL_3_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_KILL_LVL_3);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_KILL_LVL_3);
		}
	}
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_1))
	{
		int sacrifiedOrbsCount = persistentDataManager.getSacrifiedOrbsCount();
		if (sacrifiedOrbsCount >= ACHIEVEMENT_SACRIFICE_1_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_SACRIFICE_LVL_1);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_1);
		}
	}
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_2))
	{
		int sacrifiedOrbsCount = persistentDataManager.getSacrifiedOrbsCount();
		if (sacrifiedOrbsCount >= ACHIEVEMENT_SACRIFICE_2_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_SACRIFICE_LVL_2);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_2);
		}
	}
	if (!persistentDataManager.isAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_3))
	{
		int sacrifiedOrbsCount = persistentDataManager.getSacrifiedOrbsCount();
		if (sacrifiedOrbsCount >= ACHIEVEMENT_SACRIFICE_3_COUNT)
		{
			persistentDataManager.unlockAchievement(ACHIEVEMENT_SACRIFICE_LVL_3);
			GameDialogManager::sharedInstance()->getHUD()->showAchievementUnlocked(ACHIEVEMENT_SACRIFICE_LVL_3);
		}
	}
}
