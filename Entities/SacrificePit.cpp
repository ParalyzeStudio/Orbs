#include "OrbsPluginPCH.h"
#include "SacrificePit.h"

V_IMPLEMENT_SERIAL(SacrificePit, NodeSphere, 0, &g_myComponentModule );
START_VAR_TABLE(SacrificePit,NodeSphere,"sacrifice pit",0, NULL );
	
END_VAR_TABLE;

//Called when the entity is created
void SacrificePit::InitFunction()
{
	NodeSphere::InitFunction();
}

void SacrificePit::ThinkFunction()
{
	NodeSphere::ThinkFunction();

	float unscaledDt = Vision::GetTimer()->GetTimeDifference();
	float dt = unscaledDt * GameTimer::sharedInstance().getTimeScaleFactor(true);

	this->animateCube(unscaledDt);
	this->update(dt);
}

void SacrificePit::Serialize( VArchive &ar )
{
	NodeSphere::Serialize(ar);
}

void SacrificePit::init()
{
	this->radius = 50.0f;

	//this->life = SACRIFICE_PIT_LIFE;
	//this->maxLife = SACRIFICE_PIT_LIFE;
	initNeighbouringSpheres();
	initAtmosphere();
	initSharedVisibilityObject();
	initOrbs();
	initBadge();

	this->sacrificePeriod = DEFAULT_SACRIFICE_PERIOD;
	this->sacrificeElapsedTime = 0;

	this->SetUseEulerAngles(false);

	fxVerticalStretchedParticles = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\sacrifice_altar.xml");
	fxVerticalStretchedParticles->CreateParticleEffectInstance(this->GetPosition(), hkvVec3(0,0,0));

	this->visibilityStatus = VISIBILITY_STATUS_VISIBLE;
}

void SacrificePit::initAtmosphere()
{
	this->innerRadius = 100.0f;
	this->outerRadius = 300.0f;
}

void SacrificePit::addSacrifice(Team* orbTeam)
{
	orbTeam->incrementSacrifiedOrbCount();
}

void SacrificePit::animateCube(float dt)
{
	if (this->getVisibilityStatus() != VISIBILITY_STATUS_VISIBLE) //don't animate the cube if its not visible
		return;

	float yawDeltaAngle = 2;
	float pitchDeltaAngle = 1.0f;
	float rollDeltaAngle = 0.5f;

	hkvMat3 xRotationMatrix = hkvMat3(hkvNoInitialization);
	xRotationMatrix.setRotationMatrixX(yawDeltaAngle);
	hkvMat3 yRotationMatrix = hkvMat3(hkvNoInitialization);
	yRotationMatrix.setRotationMatrixY(pitchDeltaAngle);
	hkvMat3 zRotationMatrix = hkvMat3(hkvNoInitialization);
	zRotationMatrix.setRotationMatrixZ(rollDeltaAngle);

	hkvMat3 rotationMatrix = this->GetRotationMatrix();
	rotationMatrix = rotationMatrix.multiply(xRotationMatrix);
	rotationMatrix = rotationMatrix.multiply(yRotationMatrix);
	rotationMatrix = rotationMatrix.multiply(zRotationMatrix);

	this->SetRotationMatrix(rotationMatrix);
}

void SacrificePit::update(float dt)
{
	NodeSphere::update(dt);

	vector<int> vTeamIndexes;
	vector<int> vPartyIDs;
	vTeamIndexes.reserve(childOrbs.size());
	for (int childOrbsIndex = 0; childOrbsIndex != childOrbs.size(); childOrbsIndex++)
	{
		TeamedOrbs &teamedChildOrbs = childOrbs[childOrbsIndex];
		if (teamedChildOrbs.orbs.size() > 0)
		{
			vTeamIndexes.push_back(childOrbsIndex);

			bool bPushPartyID = true;
			int iPushPartyID = teamedChildOrbs.team->getParty()->getID();
			for (int iPartyIDIndex = 0; iPartyIDIndex != vPartyIDs.size(); iPartyIDIndex++)
			{
				int iPartyID = teamedChildOrbs.team->getParty()->getID();
				if (iPartyID == vPartyIDs[iPartyIDIndex])
				{
					bPushPartyID = false;
					break;
				}
			}
			if (bPushPartyID)
				vPartyIDs.push_back(iPushPartyID);
		}
	}

	if (vPartyIDs.size() > 1) //more than 1 parties fighting for controlling the sacrifice altar
	{
		sacrificeElapsedTime = 0;
	}
	else if (vPartyIDs.size() == 1)
	{
		sacrificeElapsedTime += dt;
		hkvLog::Error("sacrificeElapsedTime:%f sacrificePeriod:%f", sacrificeElapsedTime, sacrificePeriod);
		if (sacrificeElapsedTime > sacrificePeriod)
		{
			int randTeamIndex = rand() % ((int) vTeamIndexes.size());
			vector<Orb*> &orbs = childOrbs[vTeamIndexes[randTeamIndex]].orbs;
			for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
			{
				Orb* orb = orbs[orbIndex];
				if (orbs[orbIndex]->getStatus() == ORB_STATUS_ORBITING)
				{
					orb->setStatus(ORB_STATUS_SACRIFICING);
					sacrificeElapsedTime = 0;
					break;
				}
			}
		}
	}
}