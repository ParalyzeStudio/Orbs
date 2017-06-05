#pragma once

#define ACHIEVEMENT_KILL_1_COUNT 100
#define ACHIEVEMENT_KILL_2_COUNT 1000
#define ACHIEVEMENT_KILL_3_COUNT 10000
#define ACHIEVEMENT_SACRIFICE_1_COUNT 10
#define ACHIEVEMENT_SACRIFICE_2_COUNT 100
#define ACHIEVEMENT_SACRIFICE_3_COUNT 1000

class AchievementChecker
{
public:
	~AchievementChecker() {};

	static AchievementChecker& sharedInstance(){return instance;};

	void checkAchievements();

private:
	AchievementChecker() {};

	static AchievementChecker instance;
};

