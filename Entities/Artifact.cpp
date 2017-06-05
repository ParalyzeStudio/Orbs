#include "OrbsPluginPCH.h"
#include "Artifact.h"
#include "GameManager.h"


V_IMPLEMENT_SERIAL(Artifact, NodeSphere, 0, &g_myComponentModule );
START_VAR_TABLE(Artifact,NodeSphere,"artifact",0, NULL );
	
END_VAR_TABLE;

//Called when the entity is created
void Artifact::InitFunction()
{
	NodeSphere::InitFunction();
}

void Artifact::ThinkFunction()
{
	NodeSphere::ThinkFunction();

	float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
	this->update(dt);
}

void Artifact::Serialize( VArchive &ar )
{
	NodeSphere::Serialize(ar);

	this->init();
}

void Artifact::init()
{
	this->m_pCooldownTimer = new CountdownTimer();
}

void Artifact::performPowerAction(NodeSphere* pTargetSphere)
{
	if (this->m_iActivePower == ANNIHILATION)
	{
		annihilate(pTargetSphere);
	}
	else if (this->m_iActivePower == REVELATION)
	{
		giveVision(pTargetSphere);
	}
	else if (this->m_iActivePower == BERSERK)
	{
		goBerserk(pTargetSphere);
	}
}

void Artifact::annihilate(NodeSphere* pTargetSphere)
{
	m_pCooldownTimer->init(COOLDOWN_TIME_ANNIHILATE);
	m_pCooldownTimer->start();

	//TODO give vision and destroy everything that orbits on the target sphere after a short delay
	CallFuncObject* destroySphereAction = new CallFuncObject(pTargetSphere, (CallFunc)(&NodeSphere::destroyEverything), 0.5f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(destroySphereAction);
	destroySphereAction->executeFunc();
}

void Artifact::giveVision(NodeSphere* pTargetSphere)
{
	m_pCooldownTimer->init(COOLDOWN_TIME_VISION);
	m_pCooldownTimer->start();

	//TODO give vision by forcing the status of the sphere to VISIBILITY_STATUS_VISIBLE
}

void Artifact::goBerserk(NodeSphere* pTargetSphere)
{
	m_pCooldownTimer->init(COOLDOWN_TIME_BERSERK);
	m_pCooldownTimer->start();

	//TODO
}

void Artifact::update(float dt)
{
	if (m_pCooldownTimer->getRemainingTime() <= 0)
		;
		//TODO show artifact options inside core panel
}