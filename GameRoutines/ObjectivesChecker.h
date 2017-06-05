#pragma once

enum GameResult
{
	RESULT_RUNNING = 1,
	RESULT_DEFEAT,
	RESULT_VICTORY
};

class ObjectivesChecker
{
public:
	~ObjectivesChecker() {};
	static ObjectivesChecker& sharedInstance() {return instance;};

	void checkObjectives(Level* level);
	int checkLevel1Objectives(const vector<Objective*> &objectives);
	int checkLevel2Objectives(const vector<Objective*> &objectives);
	int checkLevel3Objectives(const vector<Objective*> &objectives);
	int checkLevel4Objectives(const vector<Objective*> &objectives);
	int checkLevel5Objectives(const vector<Objective*> &objectives);
	int checkLevel6Objectives(const vector<Objective*> &objectives);
	int checkLevel7Objectives(const vector<Objective*> &objectives);
	int checkLevel8Objectives(const vector<Objective*> &objectives);
	int checkLevel9Objectives(const vector<Objective*> &objectives);
	int checkLevel10Objectives(const vector<Objective*> &objectives);
	int checkLevel11Objectives(const vector<Objective*> &objectives);
	int checkLevel12Objectives(const vector<Objective*> &objectives);
	int checkLevel13Objectives(const vector<Objective*> &objectives);
	int checkLevel14Objectives(const vector<Objective*> &objectives);
	int checkLevel15Objectives(const vector<Objective*> &objectives);
	int checkLevel16Objectives(const vector<Objective*> &objectives);
	int checkLevel17Objectives(const vector<Objective*> &objectives);
	int checkLevel18Objectives(const vector<Objective*> &objectives);

private:
	ObjectivesChecker() {};

	static ObjectivesChecker instance;
};

