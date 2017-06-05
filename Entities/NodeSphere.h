#pragma once

class Orb;
class MetaOrb;
class MetaOrbLink;
class Bridge;
class SelectionHalo;
class SphereBadge;
class Path;
class Bomb;
class EndpointHalo;
class SphereGlowHalo;

#define STANDARD_INNER_RADIUS_PERCENTAGE 1.05f
#define STANDARD_OUTER_RADIUS_PERCENTAGE 1.6f

#define MAX_CORE_ITEM_LEVEL 3

#define TAKE_CONTROL_COST 10
#define GENERATION_ORBS_CAP 500

#define PENDING_ORBS_PROCESSED_BY_FRAME 100 //no more than 10 orbs processed every frame

#define BRIDGE_COST_FACTOR 0.01f // bridgeCost = round(bridgeLength * BRIDGE_COST_FACTOR)
#define BRIDGE_LENGTH_FACTOR 1.0f //bridgeLength = maxLife * BRIDGE_LENGTH_FACTOR

//visibility
#define VISIBILITY_STATUS_VISIBLE 1 //sphere has been discovered and orbs are on this sphere
#define VISIBILITY_STATUS_HIDDEN 2 //sphere has been discovered but no orb is on this sphere
#define VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR 3 //sphere has not been found yet but it is neighbouring one of visible spheres
#define VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY 4 //sphere has not been found yet and hes out of reach from one of visible spheres

#define NODE_SPHERE_ON_POINTER_MOVE_DELTA_THRESHOLD 4.0f

//Neighbouring sphere status
#define NEIGHBOURING_SPHERE_STATUS_NONE 0 //not a neigbouring sphere
#define NEIGHBOURING_SPHERE_STATUS_NORMAL 1 //sphere linked with a bridge
#define NEIGHBOURING_SPHERE_STATUS_ISOLATED 2 //isolated sphere

//visibility render context bitmask
#define NODESPHERE_VISIBLE_BITMASK 0x00000011

class Team;

struct TeamedOrbs
{
	Team* team;
	vector<Orb*> orbs;
};

struct NodeSphereTeamCache
{
	NodeSphereTeamCache() : m_pTeam(NULL), m_iAlliedOrbsCount(0), m_iEnemyOrbsCount(0), m_bAlliedOrbsCached(false), m_bEnemyOrbsCached(false) {};

	Team* m_pTeam; //the team that is enemy of all orbs contained int the enemy orbs vector
	vector<vector<Orb*>* > m_vAlliedOrbs; //the vector of allied orbs
	vector<vector<Orb*>* > m_vEnemyOrbs; //the vector of enemy orbs
	int m_iAlliedOrbsCount;
	int m_iEnemyOrbsCount;
	bool m_bAlliedOrbsCached;
	bool m_bEnemyOrbsCached;
};

class NodeSphere : public TexturedSphere, public Touchable
{
public:
	NodeSphere(void) : TexturedSphere(), Touchable(),
		orbsStartingCount(0),
		//life(0),
		innerRadius(0),
		outerRadius(0),
		generationMetaOrb(NULL),
		assaultMetaOrb(NULL),
		defenseMetaOrb(NULL),
		generationMetaOrbInitialFill(0),
		defenseMetaOrbInitialFill(0),
		assaultMetaOrbInitialFill(0),
		generationOrbsElapsedTime(0),
		m_pFxTakeControlParticleEffectFile(NULL),
		badge(NULL),
		teamId(0),
		team(NULL),
		rallySphere(NULL),
		playerAlliedOrbsCountDirty(true),
		playerEnemyOrbsCountDirty(true),
		//sphereLifeDirty(true),
		visibilityStatus(0),
		picked(false),
		selectionHalo(NULL),
		lastTravelPath(NULL),
		lastRallyPath(NULL),
		lastTargetSphere(NULL),
		sharedVisibilityObject(NULL),
		m_pStdTechnique(NULL),
		onMoveActive(false),
		onPointerDownTouchLocation(hkvVec2(0,0))
	{};
	  ~NodeSphere(void);

	virtual void InitFunction();
	virtual void ThinkFunction();
	virtual void Serialize( VArchive &ar );

	virtual void update(float dt);

	V_DECLARE_SERIAL_DLLEXP( NodeSphere, DECLSPEC_DLLEXPORT ); 
	IMPLEMENT_OBJ_CLASS(NodeSphere);

	//init/deinit
	virtual void init();
	void initTeam();
	//void initLife();
	//void initParticleGroup();
	void initNeighbouringSpheres();
	void initOrbs();
	void initMetaOrbs();
	void initTeamCache();
	void initBadge();
	//void createPortals();
	virtual void initAtmosphere(); //atmosphere is defined by an inner and an outer radius
	void initSharedVisibilityObject();

	//orb Add and removal
	void addOrb(Orb* orb);
	void removeOrb(Orb* orb);
	void addOrbToOrbsCount(Orb* orb);
	void removeOrbFromOrbsCount(Orb* orb);
	void destroyOrbsOnDeInit();
	void destroyOrbsByTeamOnDeInit(vector<Orb*> &teamOrbs);
	int getOrbsCountForTeam(Team* team);
	vector<Orb*>& getOrbsForTeam(Team* team);
	vector<vector<Orb*>* > getAlliedOrbsForTeam(Team* team);
	vector<vector<Orb*>* > getEnemyOrbsForTeam(Team* team);
	int getAlliedOrbsCountForTeam(Team* team);
	int getEnemyOrbsCountForTeam(Team* team);
	float getDefensiveOrbsRatio();

	//orb cache
	vector<vector<Orb*>* >& getCachedAlliedOrbsForTeam(Team* team, bool &isCached);
	vector<vector<Orb*>* >& getCachedEnemyOrbsForTeam(Team* team, bool &isCached);
	void setCachedEnemyOrbsForTeam(Team* team, vector<vector<Orb*>* > orbs);
	void setCachedAlliedOrbsForTeam(Team* team, vector<vector<Orb*>* > orbs);
	void setCachedEnemyOrbsCountForTeam(Team* team, int count);
	void setCachedAlliedOrbsCountForTeam(Team* team, int count);
	void resetOrbsCache();

	//meta-orbs
	void createMetaOrb(int metaOrbID, int initialNumberOfOrbs);
	void createMetaOrbLink(MetaOrb* metaOrb);
	void removeMetaOrb(int metaOrbID);
	void removeMetaOrbLinkForMetaOrb(MetaOrb* metaOrb);
	void removeMetaOrbs();
	void removeMetaOrbLinks();
	void hideMetaOrbs();
	void showMetaOrbs();
	bool hasAtLeastOneMetaOrb();
	
	void updateAggregatingOrbs(int metaOrbID, int number);
	MetaOrb* getGenerationMetaOrb(){return this->generationMetaOrb;};
	MetaOrb* getDefenseMetaOrb(){return this->defenseMetaOrb;};
	MetaOrb* getAssaultMetaOrb(){return this->assaultMetaOrb;};
	void setGenerationMetaOrb(MetaOrb* generationMetaOrb){this->generationMetaOrb = generationMetaOrb;};
	void setDefenseMetaOrb(MetaOrb* defenseMetaOrb){this->defenseMetaOrb = defenseMetaOrb;};
	void setAssaultMetaOrb(MetaOrb* assaultMetaOrb){this->assaultMetaOrb = assaultMetaOrb;};
	vector<MetaOrbLink*> &getMetaOrbLinks(){return this->metaOrbLinks;};

	//take control related methods
	bool takeControl(Team* team, bool forceTakeControl, bool generateRandomMetaOrb); //take control of a neutral sphere
	bool isDefended(Team* pAssaultingTeam);
	void preloadTakeControlParticleEffectFile();
	void launchTakeControlParticleEffect();

	//core items actions
	void generateOrbs(float dt);

	//travel routines
	void addPendingTravelingOrb(Orb* travelingOrb);
	void addPendingTravelingOrbs(vector<Orb*> &travelingOrbs);
	void processPendingOrbs();
	
	//bridges
	vector<Bridge*> findPotentialBridges();
	bool buildBridge(NodeSphere* endSphere);

	//portals
	//void addPortal(Portal* portal){this->portals.push_back(portal);};
	//void removePortals();

	//reveal/visible
	void clearVisibilityStatus();
	void initVisibilityStatus();
	virtual void setVisibilityStatus(int visibilityStatus);
	int getVisibilityStatus(){return this->visibilityStatus;};
	void updateVisibilityStatus();
	void revealNeighbouringSpheresAroundVisibleSphere();
	bool isVisibleFromDefaultTeamOrbs();
	//void invalidateStandardTechnique();
	VCompiledTechnique* getStandardTechnique(){return this->m_pStdTechnique;};

	//neighbouring spheres related methods
	bool containsNeighbouringSphere(NodeSphere* nodeSphere);
	int getNeighbouringSphereStatus(NodeSphere* nodeSphere);

	//touchable related methods
	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual bool trigger(hkvVec2 touchLocation, int eventType); //trigger the item (press, release or move)
	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);

	virtual bool pick();
	virtual void unpick();

	//rally
	NodeSphere* getRallySphere(){return this->rallySphere;};
	void setRallySphere(NodeSphere* rallySphere);

	//sphere type
	bool isCommonNodeSphere();
	bool isSacrificePit();
	bool isArtifact();

	//destroy everything on this sphere when it gets hit by an artifact
	void destroyEverything();

	//getters and setters
	float calculateProjectedRadius();
	vector<NodeSphere*> &getNeighbouringSpheres(){return this->neighbouringSpheres;};
	BatchSpriteVisibilityObject* getSharedVisibilityObject(){return this->sharedVisibilityObject;};

	float getInnerRadius(){return this->innerRadius;};
	float getOuterRadius(){return this->outerRadius;};
	float getMidRadius(){return 0.5f * (this->outerRadius + this->innerRadius);};
	int getOrbsStartingCount(){return this->orbsStartingCount;};
	//float getLife(){return this->life;};
	//float getMaxLife(){return this->maxLife;};

	int getTeamId(){return this->teamId;};
	Team* getTeam(){return this->team;};

	//refresh values that are displayed in the info panel
	bool isPlayerAlliedOrbsCountDirty(){return this->playerAlliedOrbsCountDirty;};
	void setPlayerAlliedOrbsCountDirty(bool dirty){this->playerAlliedOrbsCountDirty = dirty;};
	bool isPlayerEnemyOrbsCountDirty(){return this->playerEnemyOrbsCountDirty;};
	void setPlayerEnemyOrbsCountDirty(bool dirty){this->playerEnemyOrbsCountDirty = dirty;};
	//bool isSphereLifeDirty(){return this->sphereLifeDirty;};
	//void setSphereLifeDirty(bool dirty){this->sphereLifeDirty = dirty;};

	bool isPicked(){return this->picked;};
	SelectionHalo* getSelectionHalo(){return this->selectionHalo;};

	//implemented in cpp file
	void addNeighbouringSphere(NodeSphere* sphere);

	//purge particles
	void purgeParticleEffects();

protected:
	//Neighbouring spheres (initially no more than 4)
	VTypedObjectReference neighbouringSphere1;
	VTypedObjectReference neighbouringSphere2;
	VTypedObjectReference neighbouringSphere3;
	VTypedObjectReference neighbouringSphere4;

	//orbs evoluting in this nodeSphere's atmosphere
	vector<TeamedOrbs> childOrbs;
	vector<NodeSphereTeamCache> m_vTeamCaches; //allied and enemy orbs that are cached at every frame for processing orb shooting
	int orbsStartingCount;

	//node spheres that can be reached directly from this node sphere
	vector<NodeSphere*> neighbouringSpheres;

	//atmosphere
	float innerRadius;
	float outerRadius;

	//meta-orbs
	MetaOrb* generationMetaOrb;
	MetaOrb* defenseMetaOrb;
	MetaOrb* assaultMetaOrb;
	float generationMetaOrbInitialFill;
	float defenseMetaOrbInitialFill;
	float assaultMetaOrbInitialFill;
	vector<MetaOrbLink*> metaOrbLinks;

	//visibility
	int visibilityStatus;
	VCompiledTechnique* m_pStdTechnique;

	//particle effects related to a node sphere
	VSmartPtr<VisParticleEffectFile_cl> m_pFxTakeControlParticleEffectFile;

	//badge
	SphereBadge* badge;

	//hp of the sphere
	//bool sphereLifeDirty; //has the hp of the sphere changed
	//float life; //current life
	//float maxLife; //maximum hp to reach when on auto-regen mode

	float generationOrbsElapsedTime;

	int teamId; //id of the team that is passed between vForge and the engine
	Team* team; //team that controls this sphere, 0 if sphere is neutral

	//portals related to neighbouring spheres
	//vector<Portal*> portals;

	//rally point
	NodeSphere* rallySphere;

	//vectors of orbs waiting for traveling
	vector<NodeSphere*> pendingSpheresPath;
	stack<Orb*> pendingTravelingOrbs;

	//count of orbs (more efficient than querying the vector of all orbs every time)
	bool playerAlliedOrbsCountDirty; //has the playerAlliedOrbsCount of the sphere changed
	bool playerEnemyOrbsCountDirty; //has the playerEnemyOrbsCount of the sphere changed

	//bombs
	vector<Bomb*> trackingBombs;

	//has the sphere been picked after a click
	bool picked;
	VSmartPtr<SelectionHalo> selectionHalo;
	Path* lastTravelPath; //the last path drawn in TRAVEL mode
	Path* lastRallyPath; //the last path drawn in RALLY mode
	NodeSphere* lastTargetSphere; //if a path was drawn on previous frame, this pointer contains the address of the last path end sphere

	//bounding box shared across child sprites
	VSmartPtr<BatchSpriteVisibilityObject> sharedVisibilityObject;

	//is move active (e.g it passed the threshold)
	bool onMoveActive;
	hkvVec2 onPointerDownTouchLocation;
};

class SphereBadge : public SpriteScreenMask
{
public:
	SphereBadge();
	~SphereBadge();

	void init(VTextureObject* textureObj, NodeSphere* parentNodeSphere);
	void updatePosition();

	void OnHandleCallback(IVisCallbackDataObject_cl *pData);

private:
	NodeSphere* parentNodeSphere;
};



class SphereGlowHalo : public BatchSprite
{
public:
	SphereGlowHalo() : m_pParentSphereEntity(NULL) {};

	void init(hkvVec3 position, hkvVec2 size, VisBaseEntity_cl* parentSphereEntity);

	virtual void faceCamera();

	VisBaseEntity_cl* getParentSphereEntity(){return this->m_pParentSphereEntity;};

private:
	VisBaseEntity_cl* m_pParentSphereEntity; //nodesphere or meta-orb
};


