#pragma once

enum PowerType
{
	ANNIHILATION = 1,
	REVELATION,
	BERSERK
};

#define COOLDOWN_TIME_ANNIHILATE 300.0f //5min
#define COOLDOWN_TIME_VISION 60.0f //1min
#define COOLDOWN_TIME_BERSERK 180.0f //3min

class Artifact : public NodeSphere
{
public:
	Artifact(void) : NodeSphere(), 
		m_pCooldownTimer(0),
		m_pVisionRemainingTime(0),
		m_pBerserkRemainingTime(0){};
	~Artifact(void) {};

	virtual void InitFunction();
	virtual void ThinkFunction();
	virtual void Serialize( VArchive &ar );

	virtual void init();

	void performPowerAction(NodeSphere* pTargetSphere); //performs one of the three actions listed below
	void annihilate(NodeSphere* pTargetSphere); //destroys everything on the sphere (friend/enemy orbs and meta-orbs
	void giveVision(NodeSphere* pTargetSphere); //reveals a distant sphere for a certain period of time (30 sec)
	void goBerserk(NodeSphere* pTargetSphere); //increases the rate of fire of every friend orb on this sphere for a certain period of time (30 sec)

	void update(float dt);

	V_DECLARE_SERIAL_DLLEXP(Artifact, DECLSPEC_DLLEXPORT ); 
	IMPLEMENT_OBJ_CLASS(Artifact);

	CountdownTimer* getCooldownTimer(){return this->m_pCooldownTimer;};
	int getActivePower(){return this->m_iActivePower;};
	void setActivePower(int iPower){this->m_iActivePower = iPower;};

private:
	VSmartPtr<CountdownTimer> m_pCooldownTimer;
	VSmartPtr<CountdownTimer> m_pVisionRemainingTime;
	VSmartPtr<CountdownTimer> m_pBerserkRemainingTime;

	int m_iActivePower; //When the player has clicked one of the 3 powers set this int to its correct PowerType value
};

