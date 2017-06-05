#include "OrbsPluginPCH.h"
#include "Team.h"

/////////////////////////////Team/////////////////////////////////
Team::Team(const Team& otherTeam) : VRefCounter(otherTeam)
{

}

Team::~Team()
{

}

void Team::init(int iID, string strName, TeamParty* pParty)
{
	//id, name
	this->m_iID = iID;
	this->m_strName = strName;

	//color
	if (iID == TEAM_0) //neutral team, grey color
		this->m_color = TEAM_NEUTRAL_COLOR;
	else if (iID == TEAM_1)
		this->m_color = TEAM_1_COLOR;
	else if (iID == TEAM_2)
		this->m_color = TEAM_2_COLOR;
	else if (iID == TEAM_3)
		this->m_color = TEAM_3_COLOR;
	else if (iID == TEAM_4)
		this->m_color = TEAM_4_COLOR;
	else if (iID == TEAM_5)
		this->m_color = TEAM_5_COLOR;
	else if (iID == TEAM_6)
		this->m_color = TEAM_6_COLOR;
	else if (iID == TEAM_7)
		this->m_color = TEAM_7_COLOR;
	else
		this->m_color = V_RGBA_WHITE;

	//orbs count set to 0
	this->m_iTotalOrbCount = 0;

	//party
	this->m_pParty = pParty;
}

void Team::addOrbToOrbsCount()
{
	this->m_iTotalOrbCount++;
}

void Team::removeOrbFromOrbsCount()
{
	this->m_iTotalOrbCount--;
}

void Team::incrementSacrifiedOrbCount()
{
	this->m_iSacrifiedOrbCount++;
}

bool Team::isInDefaultTeamParty()
{
	if (this->m_iID == TEAM_1)
		return true;
	else //check among team mates
	{
		TeamParty* pParty = this->m_pParty;
		vector<Team*> teamMates = pParty->getTeamMates();
		for (int iTeamIndex = 0; iTeamIndex != teamMates.size(); iTeamIndex++)
		{
			if (teamMates[iTeamIndex]->getID() == TEAM_1)
				return true;
		}
	}

	return false;
}

/////////////////////////////TeamParty/////////////////////////////////
TeamParty::TeamParty(const TeamParty& other) : VRefCounter(other), teamMates(other.teamMates.size()), m_iID(other.m_iID) 
{
    for (std::size_t i = 0; i < other.teamMates.size(); ++i)
	{
        teamMates[i] = new Team();
		teamMates[i]->init(other.teamMates[i]->getID(), other.teamMates[i]->getName(), this);
		teamMates[i]->AddRef();
	}
}

TeamParty::~TeamParty()
{
	for (int teamIndex = 0; teamIndex != teamMates.size(); teamIndex++)
	{
		V_SAFE_RELEASE(teamMates[teamIndex]);
	}
}

void TeamParty::init(int id, vector<Team*> teamMates)
{
	this->m_iID = id;
	this->teamMates = teamMates;
}

void TeamParty::addTeam(Team* team) 
{
	team->setParty(this);
	this->teamMates.push_back(team);
}

void TeamParty::removeTeam(Team* team) 
{
	team->setParty(NULL);
	vector<Team*>::iterator teamsIt;
	for (teamsIt = teamMates.begin(); teamsIt != teamMates.end(); teamsIt++)
	{
		if (*teamsIt == team)
		{
			teamMates.erase(teamsIt);
			return;
		}
	}
};

/////////////////////////////TeamManager/////////////////////////////////
TeamManager TeamManager::s_instance;

Team* TeamManager::findTeamById(int id)
{
	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		vector<Team*> &teams = parties[partyIndex]->getTeamMates();

		for (int teamIndex = 0; teamIndex != teams.size(); teamIndex++)
		{
			if (teams[teamIndex]->getID() == id)
				return teams[teamIndex];
		}
	}

	return NULL;
}

Team* TeamManager::getDefaultTeam()
{
	return findTeamById(TEAM_1);
}

int TeamManager::getTeamsCount()
{
	int teamCount = 0;
	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		teamCount += (int) parties[partyIndex]->getTeamMates().size();
	}

	return teamCount;
}

vector<Team*> TeamManager::getAllTeams()
{
	vector<Team*> allTeams;
	int teamsCount = getTeamsCount();
	allTeams.reserve(teamsCount);

	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		vector<Team*> teams = parties[partyIndex]->getTeamMates();
		if (teams.size() != 0)
			allTeams.insert(allTeams.end(), teams.begin(), teams.end());
	}

	return allTeams;
}

void TeamManager::cleanParties()
{
	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		parties[partyIndex]->Release();
	}

	parties.clear();
}