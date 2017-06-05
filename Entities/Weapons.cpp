#include "OrbsPluginPCH.h"
#include "Weapons.h"
#include "GameManager.h"

/////////////////////////Bullet/////////////////////////
void Bullet::init(hkvVec3 position, Shooter* trackedObject, float damage)
{
	this->m_pTrackedObject = trackedObject;
	this->m_fDamage = damage;

	//radius of the target
	if (m_pTrackedObject->getType() == SHOOTER_TYPE_ORB)
		this->m_fTargetRadius = 0.5f * ((Orb*)m_pTrackedObject)->GetSize().x;
	else if (m_pTrackedObject->getType() == SHOOTER_TYPE_META_ORB)
		this->m_fTargetRadius = ((MetaOrb*)m_pTrackedObject)->getRadius();

	trackedObject->addTrackingBullet(this);

	//visibility object
	if (trackedObject->getType() == SHOOTER_TYPE_ORB)
	{
		if (!trackedObject->getParentNodeSphere())
			this->m_visibilityObject = ((Orb*) trackedObject)->getTravelingBridge()->getSharedVisibilityObject();
		else
			this->m_visibilityObject = trackedObject->getParentNodeSphere()->getSharedVisibilityObject();
	}
	else //target is a meta-orb
		this->m_visibilityObject = ((MetaOrb*) trackedObject)->getParentNodeSphere()->getSharedVisibilityObject();

	BatchSprite::init(position, 2 * hkvVec2(DEFAULT_BULLET_RADIUS, DEFAULT_BULLET_RADIUS));
}

void Bullet::initVisibilityObject()
{
	NodeSphere* parentNodeSphere = this->m_pTrackedObject->getParentNodeSphere();
	this->m_visibilityObject->attachBatchSprite(this);
}

void Bullet::destroySelf()
{
	NodeSphere* parentNodeSphere = this->m_pTrackedObject->getParentNodeSphere();
	this->m_visibilityObject->detachBatchSprite(this);
	BatchSprite::destroySelf();
}

void Bullet::update(float dt)
{
	BatchSprite::update(dt);

	hkvVec3 oldPosition = this->GetPosition();
	hkvVec3 targetPosition;
	if (!m_pTrackedObject) //take the last known position of the object
	{
		targetPosition = this->m_vTargetLastKnownPosition;
	}
	else
	{
		if (m_pTrackedObject->getType() == SHOOTER_TYPE_ORB)
			targetPosition = ((Orb*)m_pTrackedObject)->GetPosition();
		else if (m_pTrackedObject->getType() == SHOOTER_TYPE_META_ORB)
			targetPosition = ((MetaOrb*)m_pTrackedObject)->GetPosition();
		else //this case never happens
			return;

		this->m_vTargetLastKnownPosition = targetPosition;
	}

	hkvVec3 diff = targetPosition - oldPosition;
	float distance = diff.getLength();

	if (distance > this->m_fTargetRadius) //distance is bigger than the orb radius
	{
		hkvVec3 direction = diff;
		direction.normalize();
		direction = direction * (BULLET_SPEED * dt);

		hkvVec3 newPosition = oldPosition + direction;
		this->SetPosition(newPosition);
	}
	else //destroy the bullet and do damage to the tracked orb
	{
		if (m_pTrackedObject)
			m_pTrackedObject->takeHit(this);
	}
	
}

/////////////////////////Bomb/////////////////////////
//void Bomb::init(Orb* shooter, NodeSphere* targetSphere, Team* team, float damage)
//{
//	this->targetSphere = targetSphere;
//	this->team = team;
//	this->damage = damage;
//	this->targetSphere->addTrackingBomb(this);
//	
//	BatchSprite::init(shooter->GetPosition(), 2 * hkvVec2(DEFAULT_BOMB_RADIUS, DEFAULT_BOMB_RADIUS));
//}
//
//void Bomb::initVisibilityObject()
//{
//	m_visibilityObject = this->targetSphere->getSharedVisibilityObject();
//	m_visibilityObject->attachBatchSprite(this);
//}
//
//void Bomb::destroySelf()
//{
//	this->targetSphere->getSharedVisibilityObject()->detachBatchSprite(this);
//	BatchSprite::destroySelf();
//}
//
//void Bomb::update(float dt)
//{
//	hkvVec3 director = targetSphere->GetPosition() - this->GetPosition();
//	float distanceToCenter = director.getLength();
//	director.normalize();
//
//	if (distanceToCenter < targetSphere->getRadius())
//	{
//		targetSphere->takeBombHit(this);
//	}
//	else
//	{
//		float deltaDistance = BOMB_SPEED * dt;
//		hkvVec3 newPosition = this->GetPosition() + deltaDistance * director;
//		this->SetPosition(newPosition);
//	}
//}