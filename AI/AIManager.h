#pragma once

class Orb;
class NodeSphere;

#define LVL_3_WAVE_1_ORBS_COUNT 20
#define LVL_3_WAVE_2_ORBS_COUNT 40
#define LVL_3_WAVE_3_ORBS_COUNT 50

class AIManager : public IVisCallbackHandler_cl
{
public:
	~AIManager(){};
	void OneTimeInit();
	void OneTimeDeInit();

	static AIManager &sharedInstance(){return instance;};

	void initVariables();
	
	void handleAI();
	void handleAIForLevel3();
	void handleAIForLevel4();

	//sub functions
	void tryToTakeControlAndMoveOn(vector<Orb*> &assaultingOrbs, Team* assaultingTeam, Team* defendingTeam, NodeSphere* controlTargetSphere, NodeSphere* nextTravelSphere, bool setUpDefenseMetaOrb);
	void moveOrbsToSphere(Team* pTeam, NodeSphere* pStartSphere, NodeSphere* pEndSphere, int iNumOrbs);

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	virtual void update(float dt);

	////GETTERS AND SETTERS
	//level3
	bool isWave1Ended(){return this->wave1Ended;};
	bool isWave3Ended(){return this->wave2Ended;};
	bool isWave2Ended(){return this->wave3Ended;};

	//level4


private:
	AIManager(){};
	static AIManager instance;

	//level3 variables
	int startingOrbsCount;
	bool wave1TimerShown;
	bool wave1Sent;
	bool wave2Sent;
	bool wave3Sent;
	bool wave1Ended;
	bool wave2Ended;
	bool wave3Ended;
	int wave1EndTime;
	int wave2EndTime;

	//level4 variables
	bool sacrificePitDiscovered;
	int team2NextMoveTime;
	int team3NextMoveTime;
};

