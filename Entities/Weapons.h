#pragma once

#define DEFAULT_BULLET_RADIUS 6.0f
#define BULLET_SPEED 180.0f

class Bullet : public BatchSprite
{
public:
	Bullet(void) : BatchSprite(),
		m_pTrackedObject(NULL),
		//m_pTrackedOrb(NULL),
		//m_pTrackedMetaOrb(NULL),
		m_fDamage(0),
		m_vTargetLastKnownPosition(hkvVec3()){};
	~Bullet(void) {};

	//init
	void init(hkvVec3 position, Shooter* trackedOrb, float damage);
	virtual void initVisibilityObject();

	//update
	virtual void update(float dt);

	//destroy
	virtual void destroySelf();

	//getters and setters
	//void setTrackedOrb(Orb* trackedOrb){this->m_pTrackedOrb = trackedOrb;};
	//Orb* getTrackedOrb(){return this->m_pTrackedOrb;};
	//void setTrackedMetaOrb(MetaOrb* trackedMetaOrb){this->m_pTrackedMetaOrb = trackedMetaOrb;};
	//MetaOrb * getTrackedMetaOrb(){return this->m_pTrackedMetaOrb;};
	void setTrackedObject(Shooter* object){this->m_pTrackedObject = object;};
	Shooter* getTrackedObject(){return this->m_pTrackedObject;};
	void setDamage(float damage){this->m_fDamage = damage;};
	float getDamage(){return this->m_fDamage;};

private:
	//Orb* m_pTrackedOrb;
	//MetaOrb* m_pTrackedMetaOrb;
	Shooter* m_pTrackedObject;
	float m_fDamage;
	hkvVec3 m_vTargetLastKnownPosition; //the last known position of the tracked object in case this one dies
	float m_fTargetRadius; //the cached radius value of the tracked object
};

//#define DEFAULT_BOMB_RADIUS 15.0f
//#define BOMB_SPEED 20.0f
//#define BOMB_DAMAGE 20.0f
//
//class Bomb : public BatchSprite
//{
//public:
//	void init(Orb* shooter, NodeSphere* targetSphere, Team* team, float damage);
//	virtual void initVisibilityObject();
//
//	//destroy
//	virtual void destroySelf();
//
//	virtual void update(float dt);
//
//	float getDamage(){return this->damage;};
//	Team* getTeam(){return this->team;};
//	NodeSphere* getTargetSphere(){return this->targetSphere;};
//
//private:
//	NodeSphere* targetSphere;
//	Team* team;
//	float damage;
//};