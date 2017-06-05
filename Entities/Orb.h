#pragma once

#define DEFAULT_ORB_RADIUS 12.0f
#define ORB_LINEAR_SPEED 80.0f //45.0f

#define MIN_RADIUS 60
#define MAX_RADIUS 100

class NodeSphere;
class Team;
class Orb;
class MetaOrb;
class Bullet;

#define DEFAULT_HEALTH 100.0f //100 hit points
#define DEFAULT_SHOT_COOLDOWN_ORB 3.0f //1 shot every 4 seconds
#define DEFAULT_SHOT_COOLDOWN_META_ORB 1.0f
#define DEFAULT_SUBSHOT_COOLDOWN_META_ORB 0.15f
//#define DEFAULT_BOMB_COOLDOWN 10.0f //1 bomb every 10 seconds
#define DEFAULT_BULLET_DAMAGE 10.0f //10 damage for one shot
//#define DEFAULT_BOMB_DAMAGE 10.0f //10 damage for one bomb
#define DEFAULT_RANGE 100.0f

#define ORB_STATUS_ORBITING 1
#define ORB_STATUS_TRAVELING 2
#define ORB_STATUS_PENDING 3
#define ORB_STATUS_AGGREGATING 4
#define ORB_STATUS_BUILDING_BRIDGE 5
#define ORB_STATUS_TRACKING 6
#define ORB_STATUS_SACRIFICING 7
//#define STATUS_TEST 7

struct PathCheckpoint
{
	NodeSphere* nodeSphere;
	hkvVec3 point;
};

#define REGEN_MAX_HP_PERCENTAGE_ORB 0.005f //0.5% of max health every second
#define REGEN_MAX_HP_PERCENTAGE_META_ORB 0.01f //1% of max health every second

enum ShooterType {
	SHOOTER_TYPE_META_ORB = 1,
	SHOOTER_TYPE_ORB = 2
};

//class for an object that can shoot at other elements (orb/meta-orb) and be shot
class Shooter : public VBaseObject
{
public:
	Shooter() : m_pParentNodeSphere(NULL),
		m_fArmor(0),
		m_fHealth(0),
		m_fMaxHealth(0),
		m_fDamage(0),
		m_fShotCooldown(0),
		m_iType(0),
		m_DebugTime(0){};

	void init(float armor, float health, float maxHealth, float damage, int type);

	//shooters will auto regen over time
	void autoRegen(float dt);
	
	//shooting
	virtual void shootAt(Shooter* target);
	virtual void takeHit(Bullet* bullet);
	virtual void destroy();

	//pure virtual functions that need to be implemented by children
	virtual void processShooting(float dt) = 0;

	//bullets
	void addTrackingBullet(Bullet* bullet);
	void removeTrackingBullet(Bullet* bullet);
	void destroyAllTrackingBullets();
	vector<Bullet*>& getTrackingBullets(){return this->trackingBullets;};

	//getters and setters
	NodeSphere* getParentNodeSphere(){return this->m_pParentNodeSphere;};
	void setParentNodeSphere(NodeSphere* sphere){this->m_pParentNodeSphere = sphere;};
	float getArmor(){return this->m_fArmor;};
	float getHealth(){return this->m_fHealth;};
	float getMaxHealth(){return this->m_fMaxHealth;};
	float getDamage(){return this->m_fDamage;};
	float setArmor(float armor){this->m_fArmor = armor;};
	float setHealth(float health){this->m_fHealth = health;};
	float setMaxHealth(float maxHealth){this->m_fMaxHealth = maxHealth;};
	float setDamage(float damage){this->m_fDamage = damage;};
	int getType(){return this->m_iType;};

protected:
	//parent node sphere
	NodeSphere* m_pParentNodeSphere;

	//armor and health
	float m_fArmor;
	float m_fHealth;
	float m_fMaxHealth;

	//bullet damage and cooldown
	float m_fDamage;
	float m_fShotCooldown;

	//bullets that are currently tracking this orb
	vector<Bullet*> trackingBullets;

	int m_iType;

	float m_DebugTime;
};

class Orb : public BatchSprite, public Shooter
{
public:
	Orb(void) : BatchSprite(),
		radius(0), 
		theta(0),
		phi(0), 
		periodLength(0),
		orbLinearSpeed(0),
		thetaSignOfVariation(false),
		phiSignOfVariation(false),
		radiusVariation(0),
		thetaVariation(0),
		phiVariation(0),
		travelingLinkDirection(hkvVec3(0,0,0)),
		periodRemainingTime(0),
		status(0),
		travelingOnLink(false),
		//health(0),
		//maxHealth(0),
		//bulletShotCooldown(0),
		//bombShotCooldown(0),
		aggregatingMetaOrb(NULL),
		travelingBridge(NULL),
		targetBridge(NULL),
		team(NULL)
{};
	~Orb(void) {};

	virtual void update(float dt);
	
	void initWithParentSet();
	virtual void initVisibilityObject();
	void initTeam(Team* team);
	void startNewPeriod(bool firstPeriod);
	void setUpRandomValues(float &startRadius, float &targetRadius, float &startTheta, float &targetTheta, float &startPhi, float &targetPhi, bool firstPeriod);

	float thetaSinusoidalVariation(float dt);
	float phiSinusoidalVariation(float dt);
	float radiusSinusoidalVariation(float dt);
	float radiusLinearVariation(float dt);
	float thetaLinearVariation(float dt);
	float phiLinearVariation(float dt);

	//bullets
	//void addTrackingBullet(Bullet* bullet);
	//void removeTrackingBullet(Bullet* bullet);
	//void destroyAllTrackingBullets();
	//vector<Bullet*>& getTrackingBullets(){return this->trackingBullets;};

	virtual void destroy();
	virtual void destroySelf(); //when health has reached zero

	//orbit
	void orbit(bool firstPeriod);

	//aggregate
	void aggregateToMetaOrb(int metaOrbID);
	MetaOrb* getAggregatingMetaOrb(){return this->aggregatingMetaOrb;};

	//build bridge
	void startBuildingBridge(Bridge* bridge);

	//travel mode
	void travel();
	void findTravelPath(const vector<NodeSphere*> &vSpheresPath, stack<PathCheckpoint> &vTravelPath, hkvVec3 vStartPosition);
	stack<PathCheckpoint> &getTravelPath(){return this->travelPath;};
	stack<PathCheckpoint> &getAITravelPath(){return this->AITravelPath;};
	//pair<Portal*, hkvVec3> findCheckpoint(Portal* portal, hkvVec3 previousCheckpointAbsPosition, bool pairFirstCheckpoint);

	//rally point
	void rally(NodeSphere* targetNodeSphere);

	//sacrifice the orb
	void sacrifice();

	//make the orb shot on an enemy orb
	virtual void processShooting(float dt);
	
	//drop a bomb on the enemy sphere
	//void dropBomb();

	//visibility (fog of war)
	bool isVisibleForPlayer();

	bool isParentNodeSphereAlly();
	void setTeam(Team* team){this->team = team;};
	Team* getTeam(){return this->team;};

	//float getHealth(){return this->health;};
	//float getMaxHealth(){return this->maxHealth;};
	//float getShotCooldown(){return this->bulletShotCooldown;};
	//float getBulletDamage(){return this->bulletDamage;};
	//float getBombDamage(){return this->bombDamage;};
	//void setHealth(float health){this->health = health;};
	//void setMaxHealth(float maxHealth){this->maxHealth = maxHealth;};
	//void setShotCooldown(float fireRate){this->bulletShotCooldown = bulletShotCooldown;};
	//void setBulletDamage(float bulletDamage){this->bulletDamage = bulletDamage;};
	//void setBombDamage(float bombDamage){this->bombDamage = bombDamage;};

	Bridge* getTravelingBridge(){return this->travelingBridge;};

	int getStatus(){return this->status;};
	void setStatus(int status){this->status = status;};

private:
	//spherical coordinate system and other position/speed variables
	float radius;
	float theta;
	float phi;
	float periodLength;
	float orbLinearSpeed;

	bool thetaSignOfVariation;
	bool phiSignOfVariation;


	//vector that stores the variations of all 3 coords (r, theta, phi) over a period
	float radiusVariation;
	float thetaVariation;
	float phiVariation;

	//direction vector when orb is traveling on link
	hkvVec3 travelingLinkDirection;

	//(r, theta, phi) target position of the orb changes periodically
	float periodElapsedTime; 
	float periodRemainingTime; 

	int status; //is the orb in travel mode, orbiting quietly or in a tracking mode hunting for an enemy
	bool travelingOnLink; //during the travel mode, is the orb moving in a sphere or on a link
	//stack<pair<Portal*, hkvVec3> > travelPath; //the actual checkpoints the orb has to pass through before arriving at its final destination
	stack<PathCheckpoint> travelPath;
	stack<PathCheckpoint> AITravelPath; //travel path for enemy orbs handled by AI

	//health, firing rate and damage made by the orb
	//float health;
	//float maxHealth;
	//float bulletShotCooldown;
	//float bombShotCooldown;
	//float bulletDamage;
	//float bombDamage;

	//bullets that are currently tracking this orb
	vector<Bullet*> trackingBullets;

	//target meta orb where this orb will aggregate to
	MetaOrb* aggregatingMetaOrb;
	
	//bridge this orb is traveling on (if ors is traveling)
	Bridge* travelingBridge;

	//bridge this orb is going to build
	Bridge* targetBridge;

	//Which team the orb belongs to
	Team* team;

	//Visibility object attached to this MeshBufferObject
	//VSmartPtr<VisVisibilityObject_cl> m_visibilityObject;
};


#define ID_META_ORB_GENERATION "ID_META_ORB_GENERATION"
#define ID_META_ORB_DEFENSE "ID_META_ORB_DEFENSE"
#define ID_META_ORB_ASSAULT "ID_META_ORB_ASSAULT"
#define META_ORB_GENERATION_COLOR V_RGBA_GREEN
#define META_ORB_DEFENSE_COLOR V_RGBA_BLUE
#define META_ORB_ASSAULT_COLOR V_RGBA_RED
#define META_ORB_ANGULAR_SPEED 5.0f //30 degrees per second
#define META_ORB_ANGULAR_SPEED_VARIANCE 0.1f //+- 10%
#define META_ORB_GENERATION_REF_THETA 30.0f //when phi = 0
#define META_ORB_DEFENSE_REF_THETA 90.0f //when phi = 0
#define META_ORB_ASSAULT_REF_THETA 150.0f //when phi = 0

#define META_ORB_CREATION_COST 10

//generation meta-orb
#define META_ORB_BASE_PERIOD 10.0f //for a zero-level meta-orb generate 1 orb every 10 seconds
#define META_ORB_FACTOR 0.03f

//defense meta-orb
#define META_ORB_BASE_ARMOR 2.0f
#define META_ORB_ARMOR_FACTOR 0.2f
#define META_ORB_BASE_HP 300.0f
#define META_ORB_HP_FACTOR 2.0f
#define ORB_BASE_HP 30.0f
#define ORB_HP_FACTOR 0.15f
#define ORB_BASE_ARMOR 0.0f
#define ORB_ARMOR_FACTOR 0.25f

//assault meta-orb
#define META_ORB_BASE_DMG 10.0f
#define META_ORB_DMG_FACTOR 0.3f
#define ORB_BASE_DMG 10.0f
#define ORB_DMG_FACTOR 0.3f
//#define ORB_BOMB_BASE_DMG 40.0f
//#define ORB_BOMB_DMF_FACTOR 0.3f

#define META_ORB_MIN_HEALTH 100.0f
#define META_ORB_MAX_AMMO 5

#define META_ORB_ALTITUDE 120.0f

class MetaOrb : public TexturedSphere, public Shooter
{
public:
	MetaOrb() : currentOrbsValue(0), 
		targetOrbsValue(0), 
		maxOrbsValue(0), 
		currentOrbsValueDirty(false),
		angularSpeed(0),
		r(0),
		theta(0),
		phi(0),
		m_pTargetOrb(NULL),
		m_fSubShotCooldown(0),
		m_iRemainingAmmo(0),
		relatedGauge(NULL),
		m_iID(-1),
		m_pStdTechnique(NULL){};

	virtual void InitFunction(){};
	virtual void ThinkFunction();
	virtual void Serialize( VArchive &ar );

	virtual void update(float dt);
	void updateSize();

	V_DECLARE_SERIAL_DLLEXP(MetaOrb, DECLSPEC_DLLEXPORT); 
	IMPLEMENT_OBJ_CLASS(MetaOrb);

	void init(int numberOfOrbs, NodeSphere* parentNodeSphere, int id);

	void addOrb();

	hkvVec3 findPosition();

	//attack/defense
	virtual void processShooting(float dt);
	
	//destroy the orb
	virtual void destroy();

	//find values of meta-orbs and orbs characteristics
	VColorRef getColor();
	void invalidateMaxHP();
	void invalidateArmor();
	void invalidateDmg();
	float getOrbHP();
	float getOrbArmor();
	float getOrbDmg();
	//float getOrbBombDmg();

	int getCurrentOrbsValue(){return this->currentOrbsValue;};
	int getTargetOrbsValue(){return this->targetOrbsValue;};
	int getMaxOrbsValue(){return this->maxOrbsValue;};
	void setCurrentOrbsValue(int currentOrbsValue){this->currentOrbsValue = currentOrbsValue;};
	void setTargetOrbsValue(int targetOrbsValue){this->targetOrbsValue = targetOrbsValue;};
	void setMaxOrbsValue(int maxOrbsValue){this->maxOrbsValue = maxOrbsValue;};

	//get and set orb that is currently focused by this meta-orb
	Orb* getTargetOrb(){return this->m_pTargetOrb;};
	void setTargetOrb(Orb* orb){this->m_pTargetOrb = orb;};

	int getID(){return this->m_iID;};

private:
	int currentOrbsValue;
	int targetOrbsValue;
	int maxOrbsValue;
	bool currentOrbsValueDirty;

	//min and max scale the meta-orb can reach
	float minScale;
	float maxScale;

	//spherical coordinates
	float angularSpeed;
	float r;
	float theta;
	float phi;	

	//metaorbs can shots more than one bullet at once
	Orb* m_pTargetOrb;
	float m_fSubShotCooldown;
	int m_iRemainingAmmo;

	//gauge related to this meta-orb
	Gauge* relatedGauge;

	//technique
	VCompiledTechnique* m_pStdTechnique;

	int m_iID;
};

#define META_ORB_LINK_THICKNESS 4.0f

class MetaOrbLink : public BatchSprite
{
public:
	void init(NodeSphere* linkedNodeSphere, MetaOrb* linkedMetaOrb);

	void updatePosition();
	void updateRotationMatrix();
	virtual void update(float dt);

	NodeSphere* getLinkedNodeSphere(){return this->linkedNodeSphere;};
	MetaOrb* getLinkedMetaOrb(){return this->linkedMetaOrb;};
private:
	NodeSphere* linkedNodeSphere;
	MetaOrb* linkedMetaOrb;
};