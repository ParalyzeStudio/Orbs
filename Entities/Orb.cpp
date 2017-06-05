#include "OrbsPluginPCH.h"
#include "GameManager.h"


///////////////////////////////Shooter///////////////////////////////
void Shooter::init(float armor, float health, float maxHealth, float damage, int type)
{
	this->m_fArmor = armor;
	this->m_fHealth = health;
	this->m_fMaxHealth = maxHealth;
	this->m_fDamage = damage;
	this->m_iType = type;

	this->m_fShotCooldown = 0;
}

void Shooter::autoRegen(float dt)
{
	this->m_DebugTime += dt;

	if (this->m_fHealth == this->m_fMaxHealth) //object is full health
		return;

	float maxHpPercentageRegenPerSecond;
	if (this->getType() == SHOOTER_TYPE_META_ORB)
		maxHpPercentageRegenPerSecond = REGEN_MAX_HP_PERCENTAGE_META_ORB;
	else if (this->getType() == SHOOTER_TYPE_ORB)
		maxHpPercentageRegenPerSecond = REGEN_MAX_HP_PERCENTAGE_ORB;

	float regenAmount = maxHpPercentageRegenPerSecond * this->m_fMaxHealth * dt;
	this->m_fHealth += regenAmount;
	if (this->m_fHealth > m_fMaxHealth)
		this->m_fHealth = m_fMaxHealth;

	
	if (this->getType() == SHOOTER_TYPE_META_ORB && this->getParentNodeSphere()->GetPosition() == hkvVec3(800, 400, 400))
	{
		hkvLog::Error("AUTOREGEN health:%f regenAmount:%f time:%f", this->m_fHealth, regenAmount, m_DebugTime);
	}
}

void Shooter::shootAt(Shooter* target)
{
	Bullet* bullet = new Bullet();
	bullet->AddRef();

	//get the 3D position of the bullet , depending on the type of the shooting object
	hkvVec3 bulletPosition;
	if (this->m_iType == SHOOTER_TYPE_META_ORB)
		bulletPosition = ((MetaOrb*) this)->GetPosition();
	else if (this->m_iType == SHOOTER_TYPE_ORB)
		bulletPosition = ((Orb*) this)->GetPosition();

	bullet->init(bulletPosition, target, this->m_fDamage);
}

void Shooter::takeHit(Bullet* bullet)
{
	float damage = bullet->getDamage();
	this->m_fHealth -= damage;
	if (this->m_fHealth <= 0)
	{		
		this->destroy();
	}
	else
	{
		this->removeTrackingBullet(bullet);
		bullet->destroySelf();
	}
}

void Shooter::destroy()
{
	//reset the tracked object of all tracking bullets
	for (int trackingBulletIndex = 0; trackingBulletIndex != trackingBullets.size(); trackingBulletIndex++)
	{
		trackingBullets[trackingBulletIndex]->setTrackedObject(NULL);
	}
}

void Shooter::addTrackingBullet(Bullet* bullet)
{
	this->trackingBullets.push_back(bullet);
}

void Shooter::removeTrackingBullet(Bullet* bullet)
{
	vector<Bullet*>::iterator trackingBulletsIt;
	for (trackingBulletsIt = trackingBullets.begin(); trackingBulletsIt != trackingBullets.end(); trackingBulletsIt++)
	{
		if (bullet == *trackingBulletsIt)
		{
			trackingBullets.erase(trackingBulletsIt);
			return;
		}
	}
}

void Shooter::destroyAllTrackingBullets()
{
	vector<Bullet*>::iterator trackingBulletsIt;
	for (trackingBulletsIt = trackingBullets.begin(); trackingBulletsIt != trackingBullets.end(); trackingBulletsIt++)
	{
		(*trackingBulletsIt)->destroySelf();
	}

	//clear the vector (not really needed as the orb is destroyed then)
	this->trackingBullets.clear();
}

///////////////////////////////Orb///////////////////////////////
void Orb::initWithParentSet()
{
	//set up team and add particle effect if team is not null
	Team* parentNodeSphereTeam = this->m_pParentNodeSphere->getTeam();
	this->initTeam(parentNodeSphereTeam);

	//health, damage, fire rate
	MetaOrb* generationMetaOrb = this->m_pParentNodeSphere->getGenerationMetaOrb();
	float maxHealth = generationMetaOrb ? generationMetaOrb->getOrbHP() : ORB_BASE_HP; 
	float health = maxHealth;
	float damage = generationMetaOrb ? generationMetaOrb->getOrbDmg() : ORB_BASE_DMG;
	float armor = generationMetaOrb ? generationMetaOrb->getOrbArmor() : ORB_BASE_ARMOR;
	//this->bombDamage = generationMetaOrb ? generationMetaOrb->getOrbBombDmg() : ORB_BOMB_BASE_DMG;

	Shooter::init(armor, health, maxHealth, damage, SHOOTER_TYPE_ORB);

	//size
	hkvVec3 orbPosition = m_pParentNodeSphere->GetPosition();
	hkvVec2 orbSize = 2 * hkvVec2(DEFAULT_ORB_RADIUS);
	
	BatchSprite::init(orbPosition, orbSize);
	this->orbit(true);
}

void Orb::initVisibilityObject()
{
	m_visibilityObject = this->m_pParentNodeSphere->getSharedVisibilityObject();
	m_visibilityObject->attachBatchSprite(this);
}

void Orb::initTeam(Team* team)
{
	this->team = team;
}

void Orb::startNewPeriod(bool firstPeriod)
{
	float distance; //the distance the orb has to go over
	if (status == ORB_STATUS_TRAVELING /*&& travelingOnLink*/)
	{
		if (!travelPath.empty()) //travel path sould not be empty because we are on a link
		{
			hkvVec3 targetPoint = travelPath.top().point;
			hkvVec3 startPoint = this->GetPosition();
			distance = (targetPoint - startPoint).getLength();
			travelingLinkDirection = targetPoint - startPoint;
			travelingLinkDirection.normalize();
		}
		else
			status = ORB_STATUS_ORBITING;
	}
	else
	{
		float startRadius, targetRadius, startTheta, targetTheta, startPhi, targetPhi;
		if (status == ORB_STATUS_ORBITING || status == ORB_STATUS_PENDING)
			setUpRandomValues(startRadius, targetRadius, startTheta, targetTheta, startPhi, targetPhi, firstPeriod);

		//radius variation
		radiusVariation = targetRadius - startRadius;

		//theta variation
		thetaVariation = targetTheta - startTheta;

		//phi variation
		phiVariation = abs(targetPhi - startPhi);
		if (180 < phiVariation &&  phiVariation < 360) //take the smallest angle
		{
			phiVariation = 360 - phiVariation;
			if (startPhi < targetPhi)
				phiVariation = -phiVariation;
		}
		else
		{
			if (targetPhi < startPhi)
				phiVariation = -phiVariation;
		}

		if (firstPeriod)
			distance = GeometryUtils::sphericalToCartesian(hkvVec3(targetRadius, targetTheta, targetPhi)).getLength();
		else
			distance = GeometryUtils::arcDistance((startRadius + targetRadius) / 2.0f, 
												  hkvVec2(GeometryUtils::degreesToRadians(startTheta), GeometryUtils::degreesToRadians(startPhi)), 
												  hkvVec2(GeometryUtils::degreesToRadians(targetTheta), GeometryUtils::degreesToRadians(targetPhi)));
	}
		

	//set the speed of an orb
	int speedVariancePercentage = 50; //percentage of variation of the speed around the stantdard speed ORB_LINEAR_SPEED
	int speedVariance = rand() % (2 * speedVariancePercentage + 1);
	speedVariance -= speedVariance / 2; //shift it to the left
	orbLinearSpeed = (1 + speedVariance / 100.0f) * ORB_LINEAR_SPEED;

	//Adjust the length of the period according to the speed of the orb
	periodLength = distance / orbLinearSpeed; //linear case

	periodRemainingTime = periodLength;
	periodElapsedTime = 0;
}

void Orb::setUpRandomValues(float &startRadius, float &targetRadius, float &startTheta, float &targetTheta, float &startPhi, float &targetPhi, bool firstPeriod)
{
	//radius
	int parentNodeRadiusRange = GeometryUtils::round(m_pParentNodeSphere->getOuterRadius() - m_pParentNodeSphere->getInnerRadius()); //atmosphere thickness
	float midRadius = m_pParentNodeSphere->getInnerRadius() + 0.5f * parentNodeRadiusRange;
	if (firstPeriod)
	{
		startRadius = 0; //orbs are emitted from the center of the sphere
		radius = startRadius;
		targetRadius = midRadius;
	}
	else
	{
		startRadius = radius;
	}

	if (!firstPeriod)
	{		
		int deltaRadius = (parentNodeRadiusRange == 0) ? 0 : rand() % ((int) (0.30f * parentNodeRadiusRange));
		bool radiusSignOfVariation;

		float centerDirectionProbability = ((abs(startRadius - midRadius) / parentNodeRadiusRange) + 0.5f) * 100; //probability for radius to increase or decrease towards the center of the nodeSphere
		int randPercentage = rand() % 101; //percentage between 0 and 100
		if (startRadius >= m_pParentNodeSphere->getInnerRadius() && startRadius < midRadius)
		{
			if (centerDirectionProbability >= randPercentage)
				radiusSignOfVariation = true;
			else
				radiusSignOfVariation = false;
		}
		else
		{
			if (centerDirectionProbability >= randPercentage)
				radiusSignOfVariation = false;
			else 
				radiusSignOfVariation = true;
		}

		if (radiusSignOfVariation)
		{
			if (startRadius + deltaRadius > m_pParentNodeSphere->getOuterRadius())
				targetRadius = startRadius - deltaRadius;
			else
				targetRadius = startRadius + deltaRadius;
		}
		else
		{
			if (startRadius - deltaRadius < m_pParentNodeSphere->getInnerRadius())
				targetRadius = startRadius + deltaRadius;
			else
				targetRadius = startRadius - deltaRadius;
		}
	}

	//theta
	if (firstPeriod)
	{
		startTheta = (float) (rand() % 181);
		theta = startTheta;
	}
	else
	{
		startTheta = theta;
	}	

	float equatorDirectionProbability = ((abs(startTheta - 90) / 180) + 0.5f) * 100; //probability for theta to increase or decrease towards the equator
	int randPercentage = rand() % 101; //percentage between 0 and 100
	if (startTheta >= 0 && startTheta < 90)
	{
		if (equatorDirectionProbability >= randPercentage)
			thetaSignOfVariation = true;
		else
			thetaSignOfVariation = false;
	}
	else
	{
		if (equatorDirectionProbability >= randPercentage)
			thetaSignOfVariation = false;
		else 
			thetaSignOfVariation = true;
	}
	
	int deltaTheta = rand() % 20 + 5;
	if (thetaSignOfVariation)
	{		
		if ((startTheta + deltaTheta) > 180)
		{
			//thetaSignOfVariation = false;
			targetTheta = startTheta - deltaTheta;
		}
		else
			targetTheta = startTheta + deltaTheta;
	}
	else
	{
		if ((startTheta + deltaTheta) < 0)
		{
			//thetaSignOfVariation = true;
			targetTheta = startTheta + deltaTheta;
		}
		else
			targetTheta = startTheta - deltaTheta;
	}


	//phi	
	if (firstPeriod)
	{
		int signOfVariation = rand() % 2;
		phiSignOfVariation = (signOfVariation == 1) ? true : false;
	}

	if (firstPeriod)
	{
		startPhi = (float) (rand() % 360);
		phi = startPhi;
	}
	else
	{
		GeometryUtils::normalizeAngle(phi);
		startPhi = phi;
	}

	if (phiSignOfVariation)
		targetPhi = startPhi + rand() % 80 + 20;
	else
		targetPhi = startPhi - (rand() % 80 + 20);
	GeometryUtils::normalizeAngle(targetPhi);
}


float Orb::thetaSinusoidalVariation(float dt)
{
	float t1 = periodElapsedTime;
	float t2 = periodElapsedTime - dt;
	
	float theta1 = (float) (thetaVariation * pow(cos(t1 * M_PI / (2 * periodLength)), 2));
	float theta2 = (float) (thetaVariation * pow(cos(t2 * M_PI / (2 * periodLength)), 2));

	float dTheta = theta2 - theta1;
	return dTheta;
}


float Orb::phiSinusoidalVariation(float dt)
{
	float t1 = periodElapsedTime;
	float t2 = periodElapsedTime - dt;
	
	float phi1 = (float) (phiVariation * pow(cos(t1 * M_PI / (2 * periodLength)), 2));
	float phi2 = (float) (phiVariation * pow(cos(t2 * M_PI / (2 * periodLength)), 2));

	return phi2 - phi1;
}


float Orb::radiusSinusoidalVariation(float dt)
{
	return (float) -(radiusVariation / 2 * (cos(periodElapsedTime * M_PI / periodLength) - cos((periodElapsedTime - dt) * M_PI / periodLength)));
}

float Orb::radiusLinearVariation(float dt)
{
	return radiusVariation / periodLength * dt;
}

float Orb::thetaLinearVariation(float dt)
{
	return thetaVariation / periodLength * dt;
}

float Orb::phiLinearVariation(float dt)
{
	return phiVariation / periodLength * dt;
}

void Orb::destroy()
{
	Shooter::destroy();

	if (this->m_pParentNodeSphere) //orb is in a sphere's atmosphere
	{
		//detach this orb from possible meta-orb targetOrb
		MetaOrb* generationMetaOrb = this->m_pParentNodeSphere->getGenerationMetaOrb();
		MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
		MetaOrb* assaultMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();
		if (generationMetaOrb && generationMetaOrb->getTargetOrb() == this)
			generationMetaOrb->setTargetOrb(NULL);
		else if (defenseMetaOrb && defenseMetaOrb->getTargetOrb() == this)
			defenseMetaOrb->setTargetOrb(NULL);
		else if (assaultMetaOrb && assaultMetaOrb->getTargetOrb() == this)
			assaultMetaOrb->setTargetOrb(NULL);
	}

	//destroy the orb
	this->destroySelf();
}

void Orb::destroySelf()
{
	if (this->m_pParentNodeSphere)
	{
		this->m_pParentNodeSphere->removeOrb(this);
		this->m_pParentNodeSphere->removeOrbFromOrbsCount(this);
		this->m_pParentNodeSphere->getSharedVisibilityObject()->detachBatchSprite(this);
	}
	else //orb is traveling
	{
		GameManager::GlobalManager().removeTravelingOrb(this);
	}

	BatchSprite::destroySelf();
}

void Orb::orbit(bool firstPeriod)
{
	this->status = ORB_STATUS_ORBITING;
	if (!firstPeriod)
	{
		hkvVec3 sphericalPosition = GeometryUtils::cartesianToSpherical(this->GetPosition() - this->m_pParentNodeSphere->GetPosition());
		this->radius = sphericalPosition.x;
		this->theta = GeometryUtils::radiansToDegrees(sphericalPosition.y);
		this->phi = GeometryUtils::radiansToDegrees(sphericalPosition.z);
	}
	startNewPeriod(firstPeriod);
}

void Orb::aggregateToMetaOrb(int metaOrbID)
{
	if (metaOrbID == VGUIManager::GetID(ID_META_ORB_GENERATION))
		this->aggregatingMetaOrb = this->m_pParentNodeSphere->getGenerationMetaOrb();
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		this->aggregatingMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
		this->aggregatingMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();

	this->status = ORB_STATUS_AGGREGATING;

	this->destroyAllTrackingBullets(); //make it invinicible by removing tracking bullets and preventing enemies form shooting at it
}

void Orb::startBuildingBridge(Bridge* bridge)
{
	this->status = ORB_STATUS_BUILDING_BRIDGE;
	this->targetBridge = bridge;
}

void Orb::travel()
{
	if (travelPath.size() == 0)
		return;

	//update the visibility object of this orb
	this->m_pParentNodeSphere->getSharedVisibilityObject()->detachBatchSprite(this);
	GameManager::GlobalManager().addTravelingOrb(this);
	this->status = ORB_STATUS_TRAVELING;

	this->travelingBridge = GameManager::GlobalManager().findBridge(this->m_pParentNodeSphere, travelPath.top().nodeSphere);
	this->travelingBridge->getSharedVisibilityObject()->attachBatchSprite(this);

	//do the same with tracking bullets
	for (int bulletIndex = 0; bulletIndex != trackingBullets.size(); bulletIndex++)
	{
		Bullet* bullet = trackingBullets[bulletIndex];
		this->m_pParentNodeSphere->getSharedVisibilityObject()->detachBatchSprite(bullet);
		this->travelingBridge->getSharedVisibilityObject()->attachBatchSprite(bullet);
	}

	this->m_pParentNodeSphere = NULL;
	startNewPeriod(false);
}

void Orb::findTravelPath(const vector<NodeSphere*> &vSpheresPath, stack<PathCheckpoint> &vTravelPath, hkvVec3 vStartPosition)
{
	PathCheckpoint firstCheckpoint;
	firstCheckpoint.nodeSphere = vSpheresPath[0];
	firstCheckpoint.point = vStartPosition;
	vector<PathCheckpoint> tmpTravelPath;
	tmpTravelPath.reserve(vSpheresPath.size() - 1);
	for (int sphereIndex = 0; sphereIndex != vSpheresPath.size() - 1; sphereIndex++)
	{
		//find the equation of the plane defined by:
		//-the bisector of the angle formed by 3 consecutives spheres (or the director of the line joining the last 2 spheres in case we reached the end of the path)
		//-the z-vector
		//-a point inside the plane (sphere center)
		//OR
		//the orthogonal vector of the previous bisector and the z-vector (still the director if only 2 spheres remain)
		//-a point inside the plane (sphere center)
		hkvVec3 intersection;
		hkvVec3 lastCheckpointPosition = (tmpTravelPath.size() == 0) ? vStartPosition : tmpTravelPath[tmpTravelPath.size() - 1].point;
		if (sphereIndex == vSpheresPath.size() - 2) //end of the path, only 2 spheres remain
		{
			NodeSphere* lastSphere = vSpheresPath[sphereIndex+1];
			hkvVec3 lastSphereCenter = lastSphere->GetPosition();
			hkvVec3 director = lastCheckpointPosition - lastSphereCenter;
			director.normalize();
			intersection = lastSphereCenter + lastSphere->getMidRadius() * director;
		}
		else
		{
			NodeSphere* sphere1 = vSpheresPath[sphereIndex];
			NodeSphere* sphere2 = vSpheresPath[sphereIndex+1];
			NodeSphere* sphere3 = vSpheresPath[sphereIndex+2];
			//first director
			hkvVec3 u = sphere2->GetPosition() - sphere1->GetPosition();
			u.normalize();
			//second director
			hkvVec3 v = sphere3->GetPosition() - sphere2->GetPosition();
			v.normalize();
			//bisector
			hkvVec3 w = v - u;
			w.normalize();
			//normal to the mid plane
			hkvVec3 n = w.cross(hkvVec3(0,0,1));
			//point that belongs to the mid plane, take the center of the sphere
			hkvVec3 A = sphere2->GetPosition();
			//find the equation of the plane with point A and normal n
			hkvVec4 planeEquation = GeometryUtils::findPlaneEquation(A, n);
			//find the intersection point of line starting at lastCheckpointPosition and directed by u and plane
			intersection = GeometryUtils::lineAndPlaneIntersection(lastCheckpointPosition, u, planeEquation);
		}
		PathCheckpoint newCheckpoint;
		newCheckpoint.nodeSphere = vSpheresPath[sphereIndex + 1];
		newCheckpoint.point = intersection;
		tmpTravelPath.push_back(newCheckpoint);
	}

	//reverse the stack
	int tmpTravelPathSize = (int) tmpTravelPath.size();
	for (int i = tmpTravelPathSize - 1; i >= 0; i--)
	{
		vTravelPath.push(tmpTravelPath[i]);
	}
}

void Orb::rally(NodeSphere* targetNodeSphere)
{
	vector<NodeSphere*> nodesPath;
	if (GameManager::GlobalManager().findNodesPath(this->getParentNodeSphere(), targetNodeSphere, nodesPath))
		GameManager::GlobalManager().makeOrbTravel(this, nodesPath);
}

void Orb::sacrifice()
{
	SacrificePit* sacrificePit = (SacrificePit*) this->m_pParentNodeSphere;
	sacrificePit->addSacrifice(this->getTeam());

	//Vanish the orb
	this->destroySelf();
}

void Orb::processShooting(float dt)
{
	//shoot at enemy orbs
	if (m_fShotCooldown > 0)
	{
		m_fShotCooldown -= dt;
		return;
	}
	//if (bombShotCooldown > 0)
	//	bombShotCooldown -= dt;

	bool isCached = false;
	vector<vector<Orb*>* >& cachedEnemyOrbs = this->m_pParentNodeSphere->getCachedEnemyOrbsForTeam(this->team, isCached);
	vector<vector<Orb*>* > enemyOrbsVec;
	if (!isCached)
	{
		enemyOrbsVec = this->m_pParentNodeSphere->getEnemyOrbsForTeam(this->team);
		this->m_pParentNodeSphere->setCachedEnemyOrbsForTeam(this->team, enemyOrbsVec);
	}
	else
	{
		enemyOrbsVec = cachedEnemyOrbs;
	}

	if (!this->getParentNodeSphere()->isSacrificePit() && !this->getParentNodeSphere()->isArtifact() && !this->isParentNodeSphereAlly()) //this orb is an assaulting orb
	{
		float defendingOrbsRatio = this->m_pParentNodeSphere->getDefensiveOrbsRatio();

		if (defendingOrbsRatio < 0.1f) //less than 10% defending orbs, shoot at meta-orbs
		{
			MetaOrb* generationMetaOrb = this->m_pParentNodeSphere->getGenerationMetaOrb();
			MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
			MetaOrb* assaultMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();
			if (generationMetaOrb)
			{
				float shotDistance = (position - generationMetaOrb->GetPosition()).getLength();
				if (shotDistance < DEFAULT_RANGE)
				{
					m_fShotCooldown = DEFAULT_SHOT_COOLDOWN_ORB;
					this->shootAt(generationMetaOrb);
				}
			}
			else if (defenseMetaOrb)
			{
				float shotDistance = (position - defenseMetaOrb->GetPosition()).getLength();
				if (shotDistance < DEFAULT_RANGE)
				{
					m_fShotCooldown = DEFAULT_SHOT_COOLDOWN_ORB;
					this->shootAt(defenseMetaOrb);
				}
			}
			else if (assaultMetaOrb)
			{
				float shotDistance = (position - assaultMetaOrb->GetPosition()).getLength();
				if (shotDistance < DEFAULT_RANGE)
				{
					m_fShotCooldown = DEFAULT_SHOT_COOLDOWN_ORB;
					this->shootAt(assaultMetaOrb);
				}
			}
		}
	}
	if (m_fShotCooldown <= 0) //orb did not shoot, try to shoot there
	{
		for (int enemyOrbVecIndex = 0; enemyOrbVecIndex != enemyOrbsVec.size(); enemyOrbVecIndex++)
		{
			vector<Orb*>& enemyOrbs = *enemyOrbsVec[enemyOrbVecIndex];
			for (int enemyOrbIndex = 0; enemyOrbIndex != enemyOrbs.size(); enemyOrbIndex++)
			{
				Orb* targetOrb = enemyOrbs[enemyOrbIndex];

				if (this == targetOrb || this->team == targetOrb->getTeam() || targetOrb->getStatus() == ORB_STATUS_AGGREGATING)
					continue;
				float shotDistance = (position - targetOrb->GetPosition()).getLength();

				if (shotDistance < DEFAULT_RANGE)
				{
					m_fShotCooldown = DEFAULT_SHOT_COOLDOWN_ORB;
					this->shootAt(targetOrb);
					break;
				}
			}
		}
	}

	//	//REMOVE BOMB DROPPING
	//	//if (bulletShotCooldown <= 0 &&
	//	//	bombShotCooldown <= 0 &&
	//	//	!this->isParentNodeSphereAlly() &&
	//	//	this->parentNodeSphere->getDefensiveOrbsRatio() < 0.1f) //less than 10% of defending troops
	//	//{
	//	//	dropBomb();
	//	//	bombShotCooldown = DEFAULT_BOMB_COOLDOWN;
	//	//}
}

//void Orb::dropBomb()
//{
//	Bomb* bomb = new Bomb();
//	bomb->AddRef();
//	bomb->init(this, this->parentNodeSphere, this->team, this->bombDamage);
//	bomb->SetPosition(this->GetPosition());
//}

bool Orb::isVisibleForPlayer()
{
#ifdef MAKE_ALL_SCENE_ELEMENTS_VISIBLE
	return true;
#endif

	if (this->team == TeamManager::sharedInstance().getDefaultTeam())
		return true;
	else
	{
		if (this->status == ORB_STATUS_TRAVELING)
		{
			NodeSphere* s4 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1500, 1200, 400));
			NodeSphere* s6 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(2800, 1800, 400));

			if (this->travelingBridge->getStartSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE
				||
				this->travelingBridge->getEndSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
			{
				return true;
			}
		}
		else //orb is attached to a sphere
		{
			if (this->m_pParentNodeSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
				return true;
		}
	}

	return false;
}

bool Orb::isParentNodeSphereAlly()
{
	TeamParty* teamParty = this->team->getParty();
	vector<Team*> &teamMates = teamParty->getTeamMates();
	for (int teamMateIndex = 0; teamMateIndex != teamMates.size(); teamMateIndex++)
	{
		if (teamMates[teamMateIndex] == this->m_pParentNodeSphere->getTeam())
			return true;
	}

	return false;
}

//Called every frame to update the simulation
void Orb::update(float dt)
{	
	dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);

	//auto regen
	this->autoRegen(dt);

	//handle orb behavior
	if (status == ORB_STATUS_AGGREGATING)
	{
		hkvVec3 metaOrbDirection = this->aggregatingMetaOrb->GetPosition() - this->GetPosition();
		float distanceToMetaOrb = metaOrbDirection.getLengthAndNormalize();

		if (distanceToMetaOrb < 10.0f)
		{
			this->aggregatingMetaOrb->addOrb();
			this->destroySelf();
			return; //orb has been released, return
		}
		else
		{
			hkvVec3 deltaPosition = orbLinearSpeed * dt * metaOrbDirection;
			this->SetPosition(this->GetPosition() + deltaPosition);
		}
	}
	else if (status == ORB_STATUS_BUILDING_BRIDGE)
	{
		hkvVec3 targetBridgePoint = targetBridge->getStartPoint();

		hkvVec3 direction = targetBridgePoint - this->GetPosition();
		float distanceToBridgeStart = direction.getLengthAndNormalize();

		if (distanceToBridgeStart < 0.5f * this->GetSize().x)
		{
			targetBridge->addOrb();
			this->destroySelf();
			return; //orb has been released, return
		}
		else
		{
			hkvVec3 deltaPosition = orbLinearSpeed * dt * direction;
			this->SetPosition(this->GetPosition() + deltaPosition);
		}
	}
	else if (status == ORB_STATUS_SACRIFICING)
	{
		hkvVec3 sacrificeTargetPoint = this->m_pParentNodeSphere->GetPosition();

		hkvVec3 direction = sacrificeTargetPoint - this->GetPosition();
		float distanceToSacrificePitCenter = direction.getLengthAndNormalize();

		if (distanceToSacrificePitCenter < 0.5f * this->GetSize().x)
		{
			this->sacrifice();
			return; //orb has been released, return
		}
		else
		{
			hkvVec3 deltaPosition = orbLinearSpeed * dt * direction;
			this->SetPosition(this->GetPosition() + deltaPosition);
		}
	}
	else //ORB_STATUS_ORBITING || ORB_STATUS_TRAVELING || ORB_STATUS_TRACKING 
	{
		bool lastDt = false;
		if (periodRemainingTime > 0 && periodRemainingTime < dt) //recalculate dt at pro rata
		{
			lastDt = true;
			dt = periodRemainingTime;
		}

		if (periodRemainingTime == 0 && !lastDt) //period has ended
		{
			if (status == ORB_STATUS_TRAVELING)
			{
				if (travelPath.size() > 0)
				{
					NodeSphere* lastVisitedNodeSphere = travelPath.top().nodeSphere;
					travelPath.pop();
					if (this->travelingBridge)
						this->travelingBridge->getSharedVisibilityObject()->detachBatchSprite(this); //always detach the sprite from the bridge shared visibility object
					this->travelingBridge = NULL;
					if (this->travelPath.empty())
					{
						this->setParentNodeSphere(lastVisitedNodeSphere);
						lastVisitedNodeSphere->getSharedVisibilityObject()->attachBatchSprite(this);
						lastVisitedNodeSphere->addOrb(this);
						hkvVec3 orbSphericalCoordinates = GeometryUtils::cartesianToSpherical(this->GetPosition() - this->getParentNodeSphere()->GetPosition());
						this->radius = orbSphericalCoordinates.x;
						this->theta = GeometryUtils::radiansToDegrees(orbSphericalCoordinates.y);
						this->phi = GeometryUtils::radiansToDegrees(orbSphericalCoordinates.z);

						GameManager::GlobalManager().removeTravelingOrb(this);
						this->orbit(false);
					}
					else
					{
						if (this->team != TeamManager::sharedInstance().getDefaultTeam())
						{
							//set the status of th orb to ORB_STATUS_ORBITING in order to make it fight or take the control of the sphere
							GameManager::GlobalManager().removeTravelingOrb(this);
							this->setParentNodeSphere(lastVisitedNodeSphere);
							this->orbit(false);
						}
						else //command given by the player, let the orb travling to next spheres
						{
							//set the bridge the orb is currently traveling on
							NodeSphere* bridgeStartSphere = lastVisitedNodeSphere;
							NodeSphere* bridgeEndSphere = travelPath.top().nodeSphere;

							this->travelingBridge = GameManager::GlobalManager().findBridge(bridgeStartSphere, bridgeEndSphere);

							if (this->travelingBridge)
								this->travelingBridge->getSharedVisibilityObject()->attachBatchSprite(this);
						}
					}
				}
			}
			startNewPeriod(false);
			periodRemainingTime -= dt;
			periodElapsedTime += dt;
		}
		else
		{
			periodRemainingTime -= dt;
			periodElapsedTime += dt;
		}

		hkvVec3 newPosition = hkvVec3(hkvNoInitialization);

		if (status == ORB_STATUS_ORBITING || status == ORB_STATUS_PENDING)
		{
			float dRadius = radiusSinusoidalVariation(dt);
			float dTheta = thetaSinusoidalVariation(dt);
			float dPhi = phiLinearVariation(dt);		

			theta += dTheta;
			phi += dPhi;
			radius += dRadius;

			GeometryUtils::normalizeAngle(phi);

			float thetaRad = GeometryUtils::degreesToRadians(theta);
			float phiRad = GeometryUtils::degreesToRadians(phi);
			newPosition.x = radius * cos(phiRad) * sin(thetaRad);
			newPosition.y = radius * sin(phiRad) * sin(thetaRad);
			newPosition.z = radius * cos(thetaRad);
			newPosition.operator+=(m_pParentNodeSphere->GetPosition());

			if (this->team != TeamManager::sharedInstance().getDefaultTeam() && this->status == ORB_STATUS_ORBITING && this->travelPath.size() > 0) //orbiting orb handled by AI
			{
				if ((this->m_pParentNodeSphere->isCommonNodeSphere() && ((!this->isParentNodeSphereAlly() && !this->getParentNodeSphere()->isDefended(this->team)) || this->isParentNodeSphereAlly())) //enemy sphere not defended or allied sphere
					|| 
					(!this->m_pParentNodeSphere->isCommonNodeSphere() && !this->getParentNodeSphere()->isDefended(this->team))) //sacrifice altar/artifact with no enemy orbs on it
				{
					if (!this->isParentNodeSphereAlly()) //enemy sphere that is not defended, take control
						this->getParentNodeSphere()->takeControl(this->team, true, true);

					this->setStatus(ORB_STATUS_PENDING);
					this->m_pParentNodeSphere->addPendingTravelingOrb(this);
				}
			}
		}
		else if (status == ORB_STATUS_TRAVELING)
		{
			newPosition = this->GetPosition() + travelingLinkDirection * orbLinearSpeed * dt;
		}
		else if (status == ORB_STATUS_TRACKING)
			return;

		this->SetPosition(newPosition);

		//shoot at enemy targets
		if (status == ORB_STATUS_ORBITING || status == ORB_STATUS_TRACKING)
		{
			this->processShooting(dt);
		}
	}
	BatchSprite::update(dt);
}


///////////////////////////////MetaOrb///////////////////////////////
V_IMPLEMENT_SERIAL(MetaOrb, TexturedSphere, 0, &g_myComponentModule);

void MetaOrb::init(int numberOfOrbs, NodeSphere* parentNodeSphere, int id)
{
	TexturedSphere::init();
	Shooter::init(0, 0, 0, 0, SHOOTER_TYPE_META_ORB); //values will be invalidate right after, so set them to 0

	this->currentOrbsValue = numberOfOrbs;
	this->targetOrbsValue = numberOfOrbs;
	this->m_pParentNodeSphere = parentNodeSphere;
	this->m_iID = id;
	
	float fParentSphereRadius = this->getParentNodeSphere()->getRadius();
	this->maxOrbsValue = (fParentSphereRadius < 50) ? GeometryUtils::round(fParentSphereRadius) : GeometryUtils::round(2.5f * fParentSphereRadius -75.0f);

	this->minScale = 0.2f;
	this->maxScale = 0.4f;
	this->currentOrbsValueDirty = true;
	updateSize();

	this->invalidateArmor();
	this->invalidateDmg();
	this->invalidateMaxHP();
	this->m_fHealth = this->m_fMaxHealth;

	//angularSpeed
	int randPercentage = rand() % 201; //number between 0 and 200
	randPercentage -= 100;
	this->angularSpeed = META_ORB_ANGULAR_SPEED * (1 + (float) randPercentage / 100.0f * META_ORB_ANGULAR_SPEED_VARIANCE);

	//find a random position for the meta orb
	this->r = this->m_pParentNodeSphere->getRadius() + META_ORB_ALTITUDE; //radius is invariant through time
	this->phi = (float) (rand() % 360);
	hkvVec3 startPosition = this->findPosition();
	this->SetPosition(this->m_pParentNodeSphere->GetPosition() + startPosition);

	//init standard technique
	this->invalidateTechnique(V_RGBA_WHITE, true);

	if (this->m_pParentNodeSphere->isVisibleFromDefaultTeamOrbs())
		this->SetVisibleBitmask(0xffffffff);
	else
		this->SetVisibleBitmask(0);
}

void MetaOrb::Serialize( VArchive &ar )
{
	VisBaseEntity_cl::Serialize(ar);
}

void MetaOrb::addOrb()
{
	if (this->currentOrbsValue < maxOrbsValue - 1)
	{
		currentOrbsValueDirty = true;
		this->currentOrbsValue++;
	}

	//if (GameManager::GlobalManager().getPickedNodeSphere() == parentNodeSphere) //update the HUD
	//{
	//	ScreenMaskPanel* metaOrbPanel = NULL;
	//	if (this->m_iID == VGUIManager::GetID(ID_META_ORB_GENERATION))
	//		metaOrbPanel = (ScreenMaskPanel*) GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->findElementById(ID_GENERATION_PANEL);
	//	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
	//		metaOrbPanel = (ScreenMaskPanel*) GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->findElementById(ID_DEFENSE_PANEL);
	//	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
	//		metaOrbPanel = (ScreenMaskPanel*) GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->findElementById(ID_ASSAULT_PANEL);

	//	if (metaOrbPanel)
	//	{
	//		Gauge* gauge = (Gauge*) metaOrbPanel->getPanels()[0];
	//		gauge->updateElements();
	//	}
	//}
}

void MetaOrb::processShooting(float dt)
{
	//shoot at enemy orbs
	if (m_fShotCooldown > 0)
		m_fShotCooldown -= dt;

	if (m_fShotCooldown <= 0) //meta-orb is already shooting or starts shooting
	{
		if (!m_pTargetOrb) //meta-orb has focused one orb
		{
			//get cached enemy orbs or get them and cache them
			bool isCached = false;
			Team* metaOrbTeam = this->m_pParentNodeSphere->getTeam();
			vector<vector<Orb*>* >& cachedEnemyOrbs = this->m_pParentNodeSphere->getCachedEnemyOrbsForTeam(metaOrbTeam, isCached);
			vector<vector<Orb*>* > enemyOrbsVec;
			if (!isCached)
			{
				enemyOrbsVec = this->m_pParentNodeSphere->getEnemyOrbsForTeam(metaOrbTeam);
				this->m_pParentNodeSphere->setCachedEnemyOrbsForTeam(metaOrbTeam, enemyOrbsVec);
			}
			else
			{
				enemyOrbsVec = cachedEnemyOrbs;
			}

			//focus one orb
			for (int enemyOrbVecIndex = 0; enemyOrbVecIndex != enemyOrbsVec.size(); enemyOrbVecIndex++)
			{
				vector<Orb*>& enemyOrbs = *enemyOrbsVec[enemyOrbVecIndex];
				for (int enemyOrbIndex = 0; enemyOrbIndex != enemyOrbs.size(); enemyOrbIndex++)
				{
					Orb* targetOrb = enemyOrbs[enemyOrbIndex];

					if (targetOrb->getStatus() == ORB_STATUS_AGGREGATING)
						continue;
					float shotDistance = (this->GetPosition() - targetOrb->GetPosition()).getLength();

					if (shotDistance < DEFAULT_RANGE)
					{
						this->m_pTargetOrb = targetOrb;

						m_fShotCooldown = DEFAULT_SHOT_COOLDOWN_META_ORB;
						m_fSubShotCooldown = DEFAULT_SUBSHOT_COOLDOWN_META_ORB;
						m_iRemainingAmmo = META_ORB_MAX_AMMO - 1; //meta-orb has already shot once
						this->shootAt(targetOrb);
						break;
					}
				}
			}
		}
	}
	else
	{
		if (m_iRemainingAmmo > 0)
		{
			if (m_fSubShotCooldown > 0)
				m_fSubShotCooldown -= dt;
			else
			{
				m_fSubShotCooldown = DEFAULT_SUBSHOT_COOLDOWN_META_ORB;
				if (m_pTargetOrb)
					this->shootAt(m_pTargetOrb);

				if (--m_iRemainingAmmo == 0) //last subshot, set the target to null
					this->m_pTargetOrb = NULL;
			}
		}
	}
}

void MetaOrb::destroy()
{
	Shooter::destroy();

	if (this->m_iID == VGUIManager::GetID(ID_META_ORB_GENERATION))
		this->m_pParentNodeSphere->setGenerationMetaOrb(NULL);
	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		this->m_pParentNodeSphere->setDefenseMetaOrb(NULL);
	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
		this->m_pParentNodeSphere->setAssaultMetaOrb(NULL);

	//if no more meta-orbs show core panel
	if (!this->m_pParentNodeSphere->hasAtLeastOneMetaOrb())
		GameDialogManager::sharedInstance()->getHUD()->showCorePanel();

	Vision::Game.RemoveEntity(this);
}

hkvVec3 MetaOrb::findPosition()
{
	float localX = r * cos(GeometryUtils::degreesToRadians(phi));
	float localY = r * sin(GeometryUtils::degreesToRadians(phi));

	//transform this coordinates with the rotation matrix of this plane
	float referenceAngle;
	if (this->m_iID == VGUIManager::GetID(ID_META_ORB_GENERATION))
		referenceAngle = 60.0f;
	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		referenceAngle = 0.0f;
	else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
		referenceAngle = -60.0f;
	hkvMat3 orbitCircleRotationMatrix = hkvMat3(hkvNoInitialization);
	orbitCircleRotationMatrix.setIdentity();
	orbitCircleRotationMatrix.setRotationMatrixY(-referenceAngle);
	hkvVec3 newPosition = orbitCircleRotationMatrix.transformDirection(hkvVec3(localX, localY, 0));

	return newPosition;
}

VColorRef MetaOrb::getColor()
{
	if (this->getID() == VGUIManager::GetID(ID_META_ORB_GENERATION))
		return META_ORB_GENERATION_COLOR;
	else if (this->getID() == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		return META_ORB_DEFENSE_COLOR;
	else if (this->getID() == VGUIManager::GetID(ID_META_ORB_ASSAULT))
		return META_ORB_ASSAULT_COLOR;

	return V_RGBA_WHITE;
}

void MetaOrb::invalidateMaxHP()
{
	MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
	if (defenseMetaOrb)
	{
		this->m_fMaxHealth = META_ORB_BASE_HP + defenseMetaOrb->getCurrentOrbsValue() * META_ORB_HP_FACTOR;
	}

	this->m_fMaxHealth= META_ORB_BASE_HP;
}

void MetaOrb::invalidateArmor()
{
	MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
	if (defenseMetaOrb)
	{
		this->m_fArmor = META_ORB_BASE_ARMOR + defenseMetaOrb->getCurrentOrbsValue() * META_ORB_ARMOR_FACTOR;
	}

	this->m_fArmor = META_ORB_BASE_ARMOR;
}

void MetaOrb::invalidateDmg()
{
	MetaOrb* assaultMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();
	if (assaultMetaOrb)
	{
		this->m_fDamage = META_ORB_BASE_DMG + assaultMetaOrb->getCurrentOrbsValue() * META_ORB_DMG_FACTOR;
	}

	this->m_fDamage = META_ORB_BASE_DMG;
}

float MetaOrb::getOrbHP()
{
	MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
	if (defenseMetaOrb)
	{
		return ORB_BASE_HP + defenseMetaOrb->getCurrentOrbsValue() * ORB_HP_FACTOR;
	}

	return ORB_BASE_HP;
}

float MetaOrb::getOrbArmor()
{
	MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
	if (defenseMetaOrb)
	{
		return ORB_BASE_ARMOR + defenseMetaOrb->getCurrentOrbsValue() * ORB_ARMOR_FACTOR;
	}

	return ORB_BASE_ARMOR;
}

float MetaOrb::getOrbDmg()
{
	MetaOrb* assaultMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();
	if (assaultMetaOrb)
	{
		return ORB_BASE_DMG + assaultMetaOrb->getCurrentOrbsValue() * ORB_DMG_FACTOR;
	}

	return ORB_BASE_DMG;
}

//float MetaOrb::getOrbBombDmg()
//{
//	MetaOrb* assaultMetaOrb = this->parentNodeSphere->getAssaultMetaOrb();
//	if (assaultMetaOrb)
//	{
//		return ORB_BASE_DMG + assaultMetaOrb->getCurrentOrbsValue() * ORB_DMG_FACTOR;
//	}
//
//	return ORB_BASE_DMG;
//}

void MetaOrb::ThinkFunction()
{
	float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
	this->update(dt);
}

void MetaOrb::update(float dt)
{
	//auto regen
	this->autoRegen(dt);

	//handle meta-orb behavior
	float dPhi = dt * this->angularSpeed;

	phi += dPhi;
	GeometryUtils::normalizeAngle(phi);

	hkvVec3 newPosition = this->findPosition();
	newPosition.operator+=(m_pParentNodeSphere->GetPosition());

	this->SetPosition(newPosition);
	this->getHalo()->SetPosition(newPosition);

	//always stick the targetOrbsValue to the currentOrbsValue
	if (targetOrbsValue < currentOrbsValue)
		targetOrbsValue = currentOrbsValue;

	//adjust the size of the meta-orb to the currentOrbsValue
	if (currentOrbsValueDirty)
	{
		currentOrbsValueDirty = false;
		updateSize();
		this->invalidateMaxHP();

		//recalculate armor and dmg for all meta-orbs
		MetaOrb* generationMetaOrb = this->m_pParentNodeSphere->getGenerationMetaOrb();
		MetaOrb* defenseMetaOrb = this->m_pParentNodeSphere->getDefenseMetaOrb();
		MetaOrb* assaultMetaOrb = this->m_pParentNodeSphere->getAssaultMetaOrb();
		if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		{	
			if (generationMetaOrb)
				generationMetaOrb->invalidateArmor();
			this->invalidateArmor();
			if (assaultMetaOrb)
				assaultMetaOrb->invalidateArmor();
		}
		else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		{
			if (generationMetaOrb)
				generationMetaOrb->invalidateDmg();
			this->invalidateDmg();
			if (defenseMetaOrb)
				defenseMetaOrb->invalidateDmg();
		}
	}

	//shoots at enemy orbs
	this->processShooting(dt);

	//string metaOrbTypeString;
	//if (this->m_iID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
	//	metaOrbTypeString = "DEFENSE";
	//else if (this->m_iID == VGUIManager::GetID(ID_META_ORB_GENERATION))
	//	metaOrbTypeString = "GENERATION";
	//else
	//	metaOrbTypeString = "ASSAULT";
	//hkvLog::Error("METAORB %s hp:%f maxHP:%f dmg:%f armor:%f", metaOrbTypeString.c_str(), this->health, this->maxHealth, this->bulletDamage, this->armor);
}

void MetaOrb::updateSize()
{
	float scale;
	if (this->currentOrbsValue < 0) //orb has been created
		scale = (META_ORB_CREATION_COST + currentOrbsValue) / (float) META_ORB_CREATION_COST * minScale;
	else
		scale = (this->currentOrbsValue / (float) this->maxOrbsValue) * (maxScale - minScale) + minScale;

	this->radius = 50.0f * scale;
	this->SetScaling(hkvVec3(scale, scale, scale));

	//update the size of the halo
	float haloRadius = 1.45f * this->radius;
	this->getHalo()->SetSize(hkvVec2(2 * haloRadius, 2 * haloRadius));
}

///////////////////////////////MetaOrbLink///////////////////////////////
void MetaOrbLink::init(NodeSphere* linkedNodeSphere, MetaOrb* linkedMetaOrb)
{
	this->linkedNodeSphere = linkedNodeSphere;
	this->linkedMetaOrb = linkedMetaOrb;

	float linkLength = META_ORB_ALTITUDE + 10.0f; //add 10 pixel so link go through the surfaces of both the node sphere and the meta-orb
	this->size = hkvVec2(linkLength, META_ORB_LINK_THICKNESS);

	updatePosition();
}

void MetaOrbLink::updatePosition()
{
	hkvVec3 linkedNodeSpherePosition = linkedNodeSphere->GetPosition();
	hkvVec3 linkedMetaOrbPosition = linkedMetaOrb->GetPosition();
	hkvVec3 director = linkedNodeSpherePosition - linkedMetaOrbPosition;
	director.normalize();

	this->position = linkedMetaOrbPosition + director * 0.5f * this->size.x;
}

void MetaOrbLink::update(float dt)
{
	updatePosition();
	updateRotationMatrix();
}

void MetaOrbLink::updateRotationMatrix()
{
	hkvVec3 startPoint = linkedNodeSphere->GetPosition();
	hkvVec3 endPoint = linkedMetaOrb->GetPosition();
	hkvVec3 diff = endPoint - startPoint;
	VSmartPtr<VisObject3D_cl> tmp3DObject = new VisObject3D_cl();
	tmp3DObject->SetUseEulerAngles(false);
	tmp3DObject->SetDirection(diff);
	this->rotationMatrix = tmp3DObject->GetRotationMatrix();
}