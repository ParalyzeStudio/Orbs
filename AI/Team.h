#pragma once

#define TEAM_0 0 //neutral
#define TEAM_1 1 //team of the player
#define TEAM_2 2
#define TEAM_3 3
#define TEAM_4 4
#define TEAM_5 5
#define TEAM_6 6 //thieves
#define TEAM_7 7 //evil

//#define TEAM_1_COLOR VColorRef(0,255,240)
#define TEAM_1_COLOR VColorRef(255,255,255)
#define TEAM_2_COLOR V_RGBA_RED
#define TEAM_3_COLOR V_RGBA_YELLOW
#define TEAM_4_COLOR V_RGBA_PURPLE
#define TEAM_5_COLOR V_RGBA_CYAN
#define TEAM_6_COLOR V_RGBA_GREY
#define TEAM_7_COLOR V_RGBA_BLACK
#define TEAM_NEUTRAL_COLOR VColorRef(128,128,128,255)

#define MAX_TEAM_COUNT 4

class TeamParty;

class Team : public VRefCounter
{
public:
	Team() : m_iID(TEAM_1), m_strName(""), m_color(V_RGBA_WHITE), m_iTotalOrbCount(0), m_iSacrifiedOrbCount(0), m_pParty(NULL) {};
	Team(const Team& otherTeam);
	~Team();

	void init(int id, string name, TeamParty* party);

	void setID(int id){this->m_iID = id;};
	void setName(string name){this->m_strName = name;};
	int getID(){return this->m_iID;};
	string getName(){return this->m_strName;};
	VColorRef getColor(){return this->m_color;};
	int getTotalOrbsCount(){return this->m_iTotalOrbCount;};
	TeamParty* getParty(){return this->m_pParty;};
	void setParty(TeamParty* party){this->m_pParty = party;};
	bool isNeutral(){return this->m_iID == TEAM_0;};
	
	//modify and get the orbs count for this sphere
	void addOrbToOrbsCount();
	void removeOrbFromOrbsCount();

	//same for sacrified orbs count
	void incrementSacrifiedOrbCount();
	int getSacrifiedOrbCount(){return this->m_iSacrifiedOrbCount;};

	bool isInDefaultTeamParty(); //is the team allied to the TEAM_1

private:
	int m_iID;
	string m_strName;
	VColorRef m_color;

	int m_iTotalOrbCount;
	int m_iSacrifiedOrbCount;

	TeamParty* m_pParty;
};

class TeamParty : public VRefCounter //Group for alliances
{
public:
	TeamParty() : m_iID(0){};
	TeamParty(const TeamParty& otherParty);
	~TeamParty();

	void init(int id, vector<Team*> teamMates);

	void addTeam(Team* team);
	void removeTeam(Team* team);

	int getID(){return this->m_iID;};
	vector<Team*>& getTeamMates(){return this->teamMates;};

private:
	int m_iID;
	vector<Team*> teamMates;
};

class TeamManager
{
public:
	static TeamManager& sharedInstance() {return s_instance;};

	Team* getDefaultTeam();
	Team* findTeamById(int id);

	void setParties(vector<TeamParty*> parties){this->parties = parties;};
	vector<TeamParty*>& getParties(){return this->parties;};
	int getTeamsCount();
	vector<Team*> getAllTeams();

	void cleanParties();

private:
	TeamManager() {};

	static TeamManager s_instance;
	vector<TeamParty*> parties;
};