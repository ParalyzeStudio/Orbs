#include "OrbsPluginPCH.h"
#include "NodeSphere.h"
#include "GameManager.h"

V_IMPLEMENT_SERIAL( NodeSphere, TexturedSphere, 0, &g_myComponentModule);

START_VAR_TABLE(NodeSphere,VisBaseEntity_cl,"custom entity",0, NULL )
	DEFINE_VAR_OBJECT_REFERENCE(NodeSphere, neighbouringSphere1, "neighbouringSphere1", 0, 0, 0);
	DEFINE_VAR_OBJECT_REFERENCE(NodeSphere, neighbouringSphere2, "neighbouringSphere2", 0, 0, 0);
	DEFINE_VAR_OBJECT_REFERENCE(NodeSphere, neighbouringSphere3, "neighbouringSphere3", 0, 0, 0);
	DEFINE_VAR_OBJECT_REFERENCE(NodeSphere, neighbouringSphere4, "neighbouringSphere4", 0, 0, 0);
	DEFINE_VAR_INT(NodeSphere,  teamId, "team number", 0, 0, 0);
	//DEFINE_VAR_FLOAT(NodeSphere,  maxLife, "HP of the sphere (depends of the radius)", 0, 0, 0);
	DEFINE_VAR_INT(NodeSphere,  orbsStartingCount, "orbsStartingCount", 0, 0, 0);
	DEFINE_VAR_FLOAT(NodeSphere,  generationMetaOrbInitialFill, "generationMetaOrbInitialFill", 0, 0, 0);
	DEFINE_VAR_FLOAT(NodeSphere,  defenseMetaOrbInitialFill, "defenseMetaOrbInitialFill", 0, 0, 0);
	DEFINE_VAR_FLOAT(NodeSphere,  assaultMetaOrbInitialFill, "assaultMetaOrbInitialFill", 0, 0, 0);
END_VAR_TABLE


NodeSphere::~NodeSphere()
{
	selectionHalo = NULL;
	sharedVisibilityObject = NULL;
}

//Called when the entity is created
void NodeSphere::InitFunction()
{
	
}

//Called every frame to update the simulation
void NodeSphere::ThinkFunction()
{
	float unscaledDt = Vision::GetTimer()->GetTimeDifference();

	if (this->isCommonNodeSphere() && this->visibilityStatus == VISIBILITY_STATUS_VISIBLE)
	{
		//make the sphere rotate around Z axis
		float sphereRotationSpeed = 20;
		hkvMat3 zRotationMatrix;
		zRotationMatrix.setRotationMatrixZ(sphereRotationSpeed * unscaledDt);
		hkvMat3 sphereRotationMatrix = this->GetRotationMatrix();
		sphereRotationMatrix = sphereRotationMatrix.multiply(zRotationMatrix);
		this->SetRotationMatrix(sphereRotationMatrix);
	}

	float dt = unscaledDt * GameTimer::sharedInstance().getTimeScaleFactor(true);

	this->update(dt);
}


void NodeSphere::Serialize( VArchive &ar )
{
	VisBaseEntity_cl::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> neighbouringSphere1;
		ar >> neighbouringSphere2;
		ar >> neighbouringSphere3;
		ar >> neighbouringSphere4;
		ar >> teamId;
		//ar >> maxLife;
		ar >> orbsStartingCount;
		ar >> generationMetaOrbInitialFill;
		ar >> defenseMetaOrbInitialFill; 
		ar >> assaultMetaOrbInitialFill;
	} else
	{
		ar << neighbouringSphere1;
		ar << neighbouringSphere2;
		ar << neighbouringSphere3;
		ar << neighbouringSphere4;
		ar << teamId;
		//ar << maxLife;
		ar << orbsStartingCount;
		ar << generationMetaOrbInitialFill;
		ar << defenseMetaOrbInitialFill;
		ar << assaultMetaOrbInitialFill;
	}
}

void NodeSphere::init()
{
	TexturedSphere::init();

	initTeam();
	//initLife();
	initNeighbouringSpheres();
	initAtmosphere();
	initSharedVisibilityObject();
	initOrbs();
	initMetaOrbs();
	initTeamCache();
	initBadge();

	//preload various effects
	preloadTakeControlParticleEffectFile();

	//this->SetVisibleBitmask(NODESPHERE_VISIBLE_BITMASK);
}

//void NodeSphere::initLife()
//{
//	this->life = maxLife;
//}

//void NodeSphere::initParticleGroup()
//{
	//VisParticleEffectFile_cl* fxFile;
	//if (vdynamic_cast<SacrificePit*>(this))
	//	fxFile = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\sacrifice_pit.xml");
	//else
	//	fxFile = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\node_sphere.xml");

	//fxSphere = fxFile->CreateParticleEffectInstance(this->GetPosition(), hkvVec3(0,0,0), 1.0f);

	////access the ring layer and set the sphere radius to half a particle size
	//ParticleGroupBase_cl* ringLayer = fxSphere->GetParticleGroup(0);
	//VisParticleGroupDescriptor_cl* descriptor = ringLayer->GetDescriptor();
	//VCurve2DPtr particleSizeCurve = descriptor->m_spSizeCurve;
	//sphereRadius = 0.5f * particleSizeCurve->GetValueFast(0) * this->GetScaling().x; //multiply the radius by the blank entity scale factor
//}

void NodeSphere::initTeam()
{
	this->team = TeamManager::sharedInstance().findTeamById(this->teamId);
}

void NodeSphere::initNeighbouringSpheres()
{
	VTypedObject* referenceSphere1 = neighbouringSphere1.GetReferencedObject();
	VTypedObject* referenceSphere2 = neighbouringSphere2.GetReferencedObject();
	VTypedObject* referenceSphere3 = neighbouringSphere3.GetReferencedObject();
	VTypedObject* referenceSphere4 = neighbouringSphere4.GetReferencedObject();

	if (referenceSphere1)
		if (vdynamic_cast<NodeSphere*>(referenceSphere1))
			neighbouringSpheres.push_back((NodeSphere*) referenceSphere1);
	
	if (referenceSphere2)
		if (vdynamic_cast<NodeSphere*>(referenceSphere2))
			neighbouringSpheres.push_back((NodeSphere*) referenceSphere2);
	
	if (referenceSphere3)
		if (vdynamic_cast<NodeSphere*>(referenceSphere3))
			neighbouringSpheres.push_back((NodeSphere*) referenceSphere3);
	
	if (referenceSphere4)
		if (vdynamic_cast<NodeSphere*>(referenceSphere4))
			neighbouringSpheres.push_back((NodeSphere*) referenceSphere4);
}


void NodeSphere::initOrbs()
{
	//init the childOrbs vector by pushing an element for each team
	vector<Team*> allTeams = TeamManager::sharedInstance().getAllTeams();
	unsigned int teamsCount = allTeams.size();
	childOrbs.reserve(teamsCount);
	for (int teamIndex = 0; teamIndex != teamsCount; teamIndex++)
	{
		Team* team = allTeams[teamIndex];
		TeamedOrbs teamedOrbs;
		teamedOrbs.team = team;
		vector<Orb*> orbs; //push_back empty vector
		teamedOrbs.orbs = orbs;
		childOrbs.push_back(teamedOrbs);
	}

	//reserve the capacity for orbs starting count
	if (this->team)
	{
		vector<Orb*>& startingOrbs = getOrbsForTeam(this->team);
		startingOrbs.reserve(this->orbsStartingCount);

		for (int i = 0; i < this->getOrbsStartingCount(); i++)
		{		
			Orb* orb = new Orb();
			orb->AddRef();
			orb->SetPosition(this->GetPosition());
			orb->setParentNodeSphere(this);
			orb->initWithParentSet();
			addOrb(orb);
			this->addOrbToOrbsCount(orb);
		}
	}
}

void NodeSphere::initMetaOrbs()
{
	if (generationMetaOrbInitialFill > 0)
	{
		this->createMetaOrb(VGUIManager::GetID(ID_META_ORB_GENERATION), 0);
		int orbsCountForGenerationMetaOrbs = GeometryUtils::round(generationMetaOrbInitialFill * generationMetaOrb->getMaxOrbsValue());
		this->generationMetaOrb->setCurrentOrbsValue(orbsCountForGenerationMetaOrbs);
		this->generationMetaOrb->setTargetOrbsValue(orbsCountForGenerationMetaOrbs);
	}
	else
		generationMetaOrb = NULL;
	
	if (defenseMetaOrbInitialFill > 0)
	{
		this->createMetaOrb(VGUIManager::GetID(ID_META_ORB_DEFENSE), 0);
		int orbsCountForDefenseMetaOrbs = GeometryUtils::round(defenseMetaOrbInitialFill * defenseMetaOrb->getMaxOrbsValue());
		this->defenseMetaOrb->setCurrentOrbsValue(orbsCountForDefenseMetaOrbs);
		this->defenseMetaOrb->setTargetOrbsValue(orbsCountForDefenseMetaOrbs);
	}
	else
		defenseMetaOrb = NULL;

	if (assaultMetaOrbInitialFill > 0)
	{
		this->createMetaOrb(VGUIManager::GetID(ID_META_ORB_ASSAULT), 0);
		int orbsCountForAssaultMetaOrbs = GeometryUtils::round(assaultMetaOrbInitialFill * assaultMetaOrb->getMaxOrbsValue());
		this->assaultMetaOrb->setCurrentOrbsValue(orbsCountForAssaultMetaOrbs);
		this->assaultMetaOrb->setTargetOrbsValue(orbsCountForAssaultMetaOrbs);
	}
	else
		assaultMetaOrb = NULL;
}

void NodeSphere::initTeamCache()
{
	vector<Team*> allTeams = TeamManager::sharedInstance().getAllTeams();
	unsigned int teamsCount = allTeams.size();
	for (int teamIndex = 0; teamIndex != teamsCount; teamIndex++)
	{
		Team* team = allTeams[teamIndex];
		NodeSphereTeamCache teamCache;
		teamCache.m_pTeam = allTeams[teamIndex];
		m_vTeamCaches.push_back(teamCache);
	}
}

void NodeSphere::initBadge()
{
	if (this->team)
	{
		badge = new SphereBadge();
		VTextureObject* badgeTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\spheres_badges\\badge_shield.png");
		badge->init(badgeTexture, this);
		badge->SetColor(this->team->getColor());
	}
}

//void NodeSphere::createPortals()
//{
//	vector<NodeSphere*>::iterator neighbouringSpheresIt;
//	for (neighbouringSpheresIt = neighbouringSpheres.begin(); neighbouringSpheresIt != neighbouringSpheres.end(); neighbouringSpheresIt++)
//	{
//		NodeSphere* neighbouringSphere = *neighbouringSpheresIt;
//		Portal* portal = new Portal();
//		portal->init(this, neighbouringSphere);
//		portals.push_back(portal);
//	}
//}

void NodeSphere::destroyOrbsOnDeInit()
{	
	vector<Team*> teams = TeamManager::sharedInstance().getAllTeams();
	for (int teamIndex = 0; teamIndex != TeamManager::sharedInstance().getTeamsCount(); teamIndex++)
	{
		vector<Orb*> teamOrbs = getOrbsForTeam(teams[teamIndex]);
		destroyOrbsByTeamOnDeInit(teamOrbs);
	}
}

void NodeSphere::destroyOrbsByTeamOnDeInit(vector<Orb*> &teamOrbs)
{
	vector<Orb*>::iterator teamOrbsIt;
	for (teamOrbsIt = teamOrbs.begin(); teamOrbsIt != teamOrbs.end(); teamOrbsIt++)
	{
		(*teamOrbsIt)->Release();
	}

	teamOrbs.clear();
}

//void NodeSphere::removePortals()
//{
//	for (int i = 0; i != portals.size(); i++)
//	{
//		delete portals[i];
//	}
//}

int NodeSphere::getOrbsCountForTeam(Team* team)
{
	for (int childOrbsIndex = 0; childOrbsIndex != childOrbs.size(); childOrbsIndex++)
	{
		TeamedOrbs& teamedOrbs = childOrbs[childOrbsIndex];
		if (teamedOrbs.team->getID() == team->getID())
			return (int) teamedOrbs.orbs.size();
	}

	VASSERT_MSG(false, "Invalid team");
	return 0;
}

vector<Orb*>& NodeSphere::getOrbsForTeam(Team* team)
{
	for (int childOrbsIndex = 0; childOrbsIndex != childOrbs.size(); childOrbsIndex++)
	{
		TeamedOrbs& teamedOrbs = childOrbs[childOrbsIndex];
		if (teamedOrbs.team->getID() == team->getID())
			return teamedOrbs.orbs;
	}

	VASSERT_MSG(false, "Invalid team");
}

vector<vector<Orb*>* > NodeSphere::getAlliedOrbsForTeam(Team* team)
{
	TeamParty* party = team->getParty();
	vector<Team*> teamMates = party->getTeamMates();
	vector<vector<Orb*>* > orbs;
	orbs.reserve(teamMates.size());
	for (int teamMateIndex = 0; teamMateIndex != teamMates.size(); teamMateIndex++)
	{
		Team* teamMate = teamMates[teamMateIndex];
		vector<Orb*>& teamMateOrbs = getOrbsForTeam(teamMate);
		if (teamMateOrbs.size() != 0)
			orbs.push_back(&teamMateOrbs);
	}

	return orbs;
}

vector<vector<Orb*>* > NodeSphere::getEnemyOrbsForTeam(Team* team)
{
	TeamParty* pParty = team->getParty();
	vector<TeamParty*>& parties = TeamManager::sharedInstance().getParties();
	
	vector<vector<Orb*>* > orbs;

	//for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	//{
	//	if (parties[partyIndex]->getID() != pParty->getID()) //enemy party
	//	{
	//		TeamParty* enemyParty = parties[partyIndex];
	//		vector<Team*>& enemyTeamMates = enemyParty->getTeamMates();
	//		for (int enemyTeamIndex = 0; enemyTeamIndex != enemyTeamMates.size(); enemyTeamIndex++)
	//		{
	//			vector<Orb*>& enemyOrbs = getOrbsForTeam(enemyTeamMates[enemyTeamIndex]);
	//			orbs.push_back(&enemyOrbs);
	//		}
	//	}
	//}

	for (int iChildOrbsIndex = 0; iChildOrbsIndex != childOrbs.size(); iChildOrbsIndex++)
	{
		TeamedOrbs &teamedOrbs = childOrbs[iChildOrbsIndex];
		if (teamedOrbs.team->getParty()->getID() != pParty->getID()) //enemy party
			orbs.push_back(&teamedOrbs.orbs);
	}

	vector<TeamedOrbs> &travelingOrbs = GameManager::GlobalManager().getTravelingOrbs();
	for (int iTravelingOrbsIndex = 0; iTravelingOrbsIndex != travelingOrbs.size(); iTravelingOrbsIndex++)
	{
		TeamedOrbs &teamedOrbs = travelingOrbs[iTravelingOrbsIndex];
		if (teamedOrbs.team->getParty()->getID() != pParty->getID()) //enemy party
			orbs.push_back(&teamedOrbs.orbs);
	}

	return orbs;
}

void NodeSphere::initAtmosphere()
{
	//node spheres are scaled uniformly, so take x-coordinate
	innerRadius = STANDARD_INNER_RADIUS_PERCENTAGE * radius;
	outerRadius = innerRadius + 200.0f;
}

void NodeSphere::initSharedVisibilityObject()
{
	sharedVisibilityObject = new BatchSpriteVisibilityObject(VISTESTFLAGS_ACTIVE | VISTESTFLAGS_FRUSTUMTEST);
	sharedVisibilityObject->SetActivate(TRUE);
	sharedVisibilityObject->SetVisibleBitmask(0xffffffff);
	hkvAlignedBBox bbox;
	bbox.setCenterAndSize(hkvVec3(0,0,0), hkvVec3(outerRadius, outerRadius, outerRadius));
	sharedVisibilityObject->SetPosition(this->GetPosition());
	sharedVisibilityObject->SetLocalSpaceBoundingBox(bbox);
	//reserve the capacity for first orbs
	vector<BatchSprite*> &attachedBatchSprites = sharedVisibilityObject->getAttachedBatchSprites();
	attachedBatchSprites.reserve(this->orbsStartingCount);
}

void NodeSphere::clearVisibilityStatus()
{
	if (this->isSacrificePit())
		hkvLog::Error("CLEAR vis:%i", this->getVisibilityStatus());

	//initially hide everything
	this->setVisibilityStatus(VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY);
	hideMetaOrbs();
	if (this->badge)
		badge->SetOpacity(0);

}

void NodeSphere::initVisibilityStatus()
{
	/* debug make all spheres visible */
#ifdef MAKE_ALL_SCENE_ELEMENTS_VISIBLE
	this->setVisibilityStatus(VISIBILITY_STATUS_VISIBLE);
	return;
#endif
	/* debug make all spheres visible */

	Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();

	//set the correct visibility status
	if (this->team == defaultTeam)
	{
		this->setVisibilityStatus(VISIBILITY_STATUS_VISIBLE);
		showMetaOrbs();
		this->badge->SetOpacity(0);

		this->SetVisibleBitmask(0xffffffff);
		this->revealNeighbouringSpheresAroundVisibleSphere();
	}
}

void NodeSphere::addOrb(Orb* orb)
{
	vector<Orb*>& teamedOrbs = getOrbsForTeam(orb->getTeam());
	teamedOrbs.push_back(orb);
}

void NodeSphere::removeOrb(Orb* orb)
{
	vector<Orb*> &teamedOrbs = getOrbsForTeam(orb->getTeam());

	vector<Orb*>::iterator orbsIt;
	for (orbsIt = teamedOrbs.begin(); orbsIt != teamedOrbs.end(); orbsIt++)
	{
		if (*orbsIt == orb)
		{
			teamedOrbs.erase(orbsIt);
			return;
		}
	}
}

void NodeSphere::addOrbToOrbsCount(Orb* orb)
{
	Team* orbTeam = orb->getTeam();
	if (!orbTeam)
		return;

	orbTeam->addOrbToOrbsCount();

	if (orbTeam->isInDefaultTeamParty())
		playerAlliedOrbsCountDirty = true;
	else
		playerEnemyOrbsCountDirty = true;
}

void NodeSphere::removeOrbFromOrbsCount(Orb* orb)
{
	Team* orbTeam = orb->getTeam();
	if (!orbTeam)
		return;

	orbTeam->removeOrbFromOrbsCount();

	if (orbTeam->isInDefaultTeamParty())
		playerAlliedOrbsCountDirty = true;
	else
		playerEnemyOrbsCountDirty = true;
}

bool NodeSphere::takeControl(Team* team, bool forceTakeControl, bool generateRandomMetaOrb)
{
	if (!forceTakeControl) //check if enemies are in the area
	{
		if (this->getEnemyOrbsCountForTeam(team) > 0)
		{
			MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("cannot_take_control_while_fighting"));
			return false;
		}
		else if (this->getOrbsCountForTeam(team) < TAKE_CONTROL_COST)
		{
			MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("take_control_min_orbs"));
			return false;
		}
	}

	//make rally and eventually build buttons appear if player took control of this sphere
	if (team == TeamManager::sharedInstance().getDefaultTeam())
	{
		float scale = GameDialogManager::sharedInstance()->getScale();
		hkvVec2 finalActionBtnSize = hkvVec2(156.0f * scale, 156.0f * scale);
		GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->showRallyBtn(hkvVec2(-78.0f * scale, 0.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
		GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
	}

	this->team = team;
	this->teamId = team->getID();
	this->invalidateTechnique(team->getColor(), this->isVisibleFromDefaultTeamOrbs());
	this->launchTakeControlParticleEffect();

	return true;
}

bool NodeSphere::isDefended(Team* pAssaultingTeam)
{
	int iDefendingOrbsCount = this->getEnemyOrbsCountForTeam(pAssaultingTeam);
	return iDefendingOrbsCount > 0 &&  this->hasAtLeastOneMetaOrb();	
}

void NodeSphere::preloadTakeControlParticleEffectFile()
{
	m_pFxTakeControlParticleEffectFile = VisParticleGroupManager_cl::GlobalManager().LoadFromFile("Particles\\sphere_take_control.xml");
}

void NodeSphere::launchTakeControlParticleEffect()
{
	m_pFxTakeControlParticleEffectFile->CreateParticleEffectInstance(this->GetPosition(), hkvVec3(0,0,0), this->GetScaling().x); //scale the particle effect according to the uniform scaling of the sphere
}


void NodeSphere::generateOrbs(float dt)
{
	if (!this->team) //neutral sphere or no generator available
		return;

	if (!this->generationMetaOrb) //not a generation meta-orb
		return;

	if (this->getOrbsCountForTeam(this->team) >= GENERATION_ORBS_CAP) //sphere is full of orbs, dont generate more of them
		generationOrbsElapsedTime = 0;
	else
	{
		generationOrbsElapsedTime += dt;

		int generationMetaOrbLevel = this->generationMetaOrb->getCurrentOrbsValue();
		float orbGenerationPeriod = META_ORB_BASE_PERIOD - META_ORB_FACTOR * generationMetaOrbLevel;

		if (generationOrbsElapsedTime >= orbGenerationPeriod)
		{
			generationOrbsElapsedTime = 0; //reset the counter

			//Create the orb and assign it a parent
			Orb* orb = new Orb();
			orb->AddRef();
			//startposition is a random position inside the sphere
			hkvVec3 orbStartPosition = hkvVec3(0,0,0);
			int iSphereRadius = (int) this->radius;
			int randNumber = rand() % iSphereRadius;
			orbStartPosition.x = (float) (rand() % iSphereRadius);
			orbStartPosition.y = (float) (rand() % iSphereRadius);
			orbStartPosition.z = (float) (rand() % iSphereRadius);
			orb->setParentNodeSphere(this);
			orb->initWithParentSet();
			orb->SetPosition(this->GetPosition() + orbStartPosition);
			this->addOrb(orb);
			this->addOrbToOrbsCount(orb);

			//has this orb to be rallied to another sphere?
			if (rallySphere)
				orb->rally(rallySphere);
			else
				orb->orbit(true);
		}
	}
}

bool NodeSphere::containsNeighbouringSphere(NodeSphere* nodeSphere)
{
	for (int neighbouringSphereIndex = 0; neighbouringSphereIndex != neighbouringSpheres.size(); neighbouringSphereIndex++)
	{
		if (nodeSphere->GetPosition() == neighbouringSpheres[neighbouringSphereIndex]->GetPosition())
			return true;
	}
	return false;
}

//check if this sphere is a neighbouring sphere
//NEIGHBOURING_SPHERE_STATUS_NONE - not a neigbouring sphere
//NEIGHBOURING_SPHERE_STATUS_NORMAL - sphere linked with a bridge
//NEIGHBOURING_SPHERE_STATUS_ISOLATED - isolated sphere
int NodeSphere::getNeighbouringSphereStatus(NodeSphere* nodeSphere)
{
	if (nodeSphere->containsNeighbouringSphere(this))
	{
		if (this->containsNeighbouringSphere(nodeSphere)) //double linked, a bridge exists between them
			return NEIGHBOURING_SPHERE_STATUS_NORMAL;
		else
			return NEIGHBOURING_SPHERE_STATUS_ISOLATED;
	}
	else
		return NEIGHBOURING_SPHERE_STATUS_NONE;
}

bool NodeSphere::containsTouch(hkvVec2 touchLocation)
{
	hkvVec3 traceDirOut;
	VisRenderContext_cl::GetCurrentContext()->GetTraceDirFromScreenPos(touchLocation.x, touchLocation.y, traceDirOut, 1.0f);
	hkvVec3 nodeSphereCenter = this->GetPosition();
	float nodeSphereBBoxWidth = 2 * 100.0f;

	MainCamera* mainCamera = GameManager::GlobalManager().getMainCamera();
	return GeometryUtils::lineAndSphereIntersects(mainCamera->GetPosition(), traceDirOut, nodeSphereCenter, nodeSphereBBoxWidth);
}

bool NodeSphere::trigger(hkvVec2 touchLocation, int eventType)
{
	if (eventType == POINTER_DOWN)
	{
		if (this->containsTouch(touchLocation))
		{
			if (this != GameManager::GlobalManager().getActionTargetNodeSphere())
			{
				this->select();
				this->onPointerDown(touchLocation);
			}
			return true;
		}
		else
			return false;
	}
	else if (eventType == POINTER_MOVE)
	{
		if (this->selected)
		{
			return this->onPointerMove(touchLocation);
		}
		else
		{
			return false;
		}
		//if (this->picked && this->selected)
		//{
		//	this->onPointerMove(touchLocation);
		//	return true;
		//}
		//else if (!this->picked && this->selected)
		//{
		//	ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		//	if (!actionPanel || actionPanel->getActionMode() == ACTION_NORMAL_MODE || actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
		//	{
		//		if (this->containsTouch(touchLocation))
		//		{
		//			if (this->pick())
		//			{
		//				this->select();
		//				//GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->updateActionMode(ACTION_NORMAL_MODE);
		//				int actionMode = GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->getActionMode();
		//				GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->setActionMode(ACTION_NORMAL_MODE);
		//				GameManager::GlobalManager().getPickedNodeSphere()->removeLastTravelPath();
		//				GameManager::GlobalManager().OnPickSphere();
		//				return true;
		//			}
		//		}
		//	}
		//}
	}
	else if (eventType == POINTER_UP)
	{
		if (this->containsTouch(touchLocation))
		{
			this->onPointerUp(touchLocation);
			return true;
		}
		else
		{
			if (this->picked)
			{
				//this->onPointerUp(touchLocation);
				this->deselect();
				//GameManager::GlobalManager().getPickedNodeSphere()->removeLastTravelPath();
				return false;
			}
		}
		return false;
	}
	else if (eventType == POINTER_CANCEL)
	{
		this->deselect();
		return true;
	}
	return false;
}

void NodeSphere::onPointerDown(hkvVec2 touchLocation)
{
	Touchable::onPointerDown(touchLocation);

	this->onMoveActive = false;
	this->onPointerDownTouchLocation = touchLocation;
}

bool NodeSphere::onPointerMove(hkvVec2 touchLocation)
{
	if (touchLocation.isEqual(lastTouchLocation, ON_MOVE_DELTA_EPSILON))
	{
		return true;
	}

	int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
	if (currentLevelNumber == 1 && !TutorialsHandler::sharedInstance().isMoveBtnRevealed())
		return false;

	if (!onMoveActive)
	{
		float distanceFromPointerDownLocation = (touchLocation - onPointerDownTouchLocation).getLength();
		if (distanceFromPointerDownLocation >= NODE_SPHERE_ON_POINTER_MOVE_DELTA_THRESHOLD)
		{
			hkvLog::Error("activate onMove");
			this->onMoveActive = true;
		}
		else
		{
			hkvLog::Error("onMove still unactive");
			return false;
		}
	}

	if (this->picked)
	{
		//clear any previous set target nodesphere
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);

		ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		int actionMode = actionPanel->getActionMode();
		if (actionMode != ACTION_TRAVEL_MODE)
		{
			GameManager::GlobalManager().performMoveAction();
			actionPanel->updateActionMode(ACTION_TRAVEL_MODE);
		}

		this->lastTouchLocation = touchLocation;

		//check if touch location is over one sphere
		NodeSphere* targetSphere = NULL;
		vector<NodeSphere*> allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();

		for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
		{
			NodeSphere* sphere = allNodeSpheres[sphereIndex];
			if (sphere == this)
				continue;
			else
			{
				if (sphere->containsTouch(touchLocation))
				{
					targetSphere = sphere;
					break;
				}
			}
		}

		GameDialogManager::sharedInstance()->getHUD()->drawTravelPath(this, targetSphere, touchLocation);
	}
	else
	{
		ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		if (!actionPanel || actionPanel->getActionMode() == ACTION_NORMAL_MODE || actionPanel->getActionMode() == ACTION_TRAVEL_MODE)
		{
			if (this->containsTouch(touchLocation))
			{
				if (this->pick())
				{
					this->select();
					//GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->updateActionMode(ACTION_NORMAL_MODE);
					int actionMode = GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->getActionMode();
					GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->setActionMode(ACTION_NORMAL_MODE);
					GameDialogManager::sharedInstance()->getHUD()->removeActiveTravelPath();
					GameManager::GlobalManager().OnPickSphere();
				}
			}
		}
	}

	return true;
}

void NodeSphere::onPointerUp(hkvVec2 touchLocation) //this is actually the onClick method
{
	Touchable::onPointerUp(touchLocation);
	
	this->onMoveActive = false;

	if (this->containsTouch(touchLocation)) //onclick
	{
		if (!this->picked)
		{
			if (this != GameManager::GlobalManager().getActionTargetNodeSphere())
			{
				if (this->pick())
					GameManager::GlobalManager().OnPickSphere();
				else
					GameManager::GlobalManager().OnPickNothing(false);
			}
		}
		else
		{
			GameManager::GlobalManager().OnPickNothing(false);
		}
	}
	else
		GameManager::GlobalManager().OnPickNothing(false);
}

bool NodeSphere::pick()
{
	GameHUD* gameHUD = GameDialogManager::sharedInstance()->getHUD();
	ActionPanel* actionPanel = gameHUD->getActionPanel();
	if (!actionPanel || actionPanel->getActionMode() == ACTION_NORMAL_MODE)
	{
		NodeSphere* previousPickedNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();
		if (previousPickedNodeSphere == this) //same sphere picked
			return false;
		else
		{
			if (previousPickedNodeSphere)
				previousPickedNodeSphere->unpick();
			GameManager::GlobalManager().setPickedNodeSphere(this);
			this->picked = true;
			return true;
		}
	}
	else //every other case sets the actionTargetNodeSphere
	{
		NodeSphere* actionTargetSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
		if (this != GameManager::GlobalManager().getActionTargetNodeSphere()) //new action target sphere
		{
			GameManager::GlobalManager().setActionTargetNodeSphere(this);
			return true;
		}
		return false;
	}

	return false;
}

void NodeSphere::unpick()
{
	GameManager::GlobalManager().setPickedNodeSphere(NULL);
	this->picked = false;
}

void NodeSphere::setRallySphere(NodeSphere* rallySphere)
{
	if (rallySphere == this)
		rallySphere = NULL;
	else 
		this->rallySphere = rallySphere;
}

bool NodeSphere::isCommonNodeSphere()
{
	return !isSacrificePit() && !isArtifact();
}

bool NodeSphere::isSacrificePit()
{
	return vdynamic_cast<SacrificePit*>(this) != NULL;
}

bool NodeSphere::isArtifact()
{
	return false;
}

void NodeSphere::destroyEverything()
{
	//destroy all orbs
	for (int iTeamedOrbsIndex = 0; iTeamedOrbsIndex != childOrbs.size(); iTeamedOrbsIndex++)
	{
		vector<Orb*>& teamedOrbs = childOrbs[iTeamedOrbsIndex].orbs;
		for (int iOrbIndex = 0; iOrbIndex != teamedOrbs.size(); iOrbIndex++)
		{
			teamedOrbs[iOrbIndex]->destroy();
		}
	}

	childOrbs.clear();

	//destroy meta orbs
	if (generationMetaOrb)
		generationMetaOrb->destroy();
	if (defenseMetaOrb)
		defenseMetaOrb->destroy();
	if (assaultMetaOrb)
		assaultMetaOrb->destroy();
}

float NodeSphere::calculateProjectedRadius()
{
	MainCamera* camera = GameManager::GlobalManager().getMainCamera();
	hkvVec3 director = this->GetPosition() - camera->GetPosition();
	director.normalize();
	hkvVec3 localSurfacePoint = director * this->radius;
	localSurfacePoint = localSurfacePoint.cross(hkvVec3(0,0,1));
	localSurfacePoint.normalize();	
	localSurfacePoint.operator*=(this->radius);

	hkvVec3 worldSurfacePoint = this->GetPosition() + localSurfacePoint;

	float radius = (worldSurfacePoint - this->GetPosition()).getLength();

	hkvVec2 finalSurfacePointProjection = hkvVec2(0,0);
	VisRenderContext_cl::GetCurrentContext()->Project2D(worldSurfacePoint, finalSurfacePointProjection.x, finalSurfacePointProjection.y);

	hkvVec2 sphereCenterProjection = hkvVec2(0,0);
	VisRenderContext_cl::GetCurrentContext()->Project2D(this->GetPosition(), sphereCenterProjection.x, sphereCenterProjection.y);

	return (finalSurfacePointProjection - sphereCenterProjection).getLength();
}

int NodeSphere::getAlliedOrbsCountForTeam(Team* team)
{
	//try to find the value inside cache
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			if (teamCache.m_iAlliedOrbsCount >= 0) //value is cached
			{
				return teamCache.m_iAlliedOrbsCount;
			}

			break;
		}
	}

	//value is not cached, calculate it
	TeamParty* party = team->getParty(); //party of the node sphere team
	vector<Team*> teamMates = party->getTeamMates();

	int alliedOrbsCount = 0;
	for (int teamMateIndex = 0; teamMateIndex != teamMates.size(); teamMateIndex++)
	{
		Team* teamMate = teamMates[teamMateIndex];
		alliedOrbsCount += getOrbsCountForTeam(teamMate);
	}

	this->setCachedAlliedOrbsCountForTeam(team, alliedOrbsCount);
	return alliedOrbsCount;
}

int NodeSphere::getEnemyOrbsCountForTeam(Team* team)
{
	//try to find the value inside cache
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			if (teamCache.m_iEnemyOrbsCount >= 0) //value is cached
				return teamCache.m_iEnemyOrbsCount;

			break;
		}
	}

	//value is not cached, calculate it
	TeamParty* party = team->getParty();
	vector<TeamParty*>& parties = TeamManager::sharedInstance().getParties();

	int enemyOrbsCount = 0;
	for (int partyIndex = 0; partyIndex != parties.size(); partyIndex++)
	{
		if (parties[partyIndex]->getID() != party->getID())	
		{
			TeamParty* enemyParty = parties[partyIndex];
			vector<Team*>& enemyTeamMates = enemyParty->getTeamMates();
			for (int enemyTeamIndex = 0; enemyTeamIndex != enemyTeamMates.size(); enemyTeamIndex++)
			{
				enemyOrbsCount += getOrbsCountForTeam(enemyTeamMates[enemyTeamIndex]);
			}
		}
	}

	this->setCachedEnemyOrbsCountForTeam(team, enemyOrbsCount);
	return enemyOrbsCount;
}

float NodeSphere::getDefensiveOrbsRatio()
{
	int alliedOrbsEnemyCount = this->getAlliedOrbsCountForTeam(this->team);
	int enemyOrbsEnemyCount = this->getEnemyOrbsCountForTeam(this->team);

	return alliedOrbsEnemyCount / (float) (enemyOrbsEnemyCount + alliedOrbsEnemyCount);
}

vector<vector<Orb*>* >& NodeSphere::getCachedAlliedOrbsForTeam(Team* team, bool &isCached)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID() && teamCache.m_bAlliedOrbsCached)
		{
			isCached = true;
			return m_vTeamCaches[teamCacheIndex].m_vAlliedOrbs;
		}
	}

	isCached = false;
	vector<vector<Orb*>* > emptyVector;
	return emptyVector;
}

vector<vector<Orb*>* >& NodeSphere::getCachedEnemyOrbsForTeam(Team* team, bool &isCached)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID() && teamCache.m_bEnemyOrbsCached)
		{
			isCached = true;
			return m_vTeamCaches[teamCacheIndex].m_vEnemyOrbs;
		}
	}

	isCached = false;
	vector<vector<Orb*>* > emptyVector;
	return emptyVector;
}

void NodeSphere::setCachedEnemyOrbsForTeam(Team* team, vector<vector<Orb*>* > orbs)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			teamCache.m_vEnemyOrbs = orbs;
			teamCache.m_bEnemyOrbsCached = true;
		}
	}
}

void NodeSphere::setCachedAlliedOrbsForTeam(Team* team, vector<vector<Orb*>* > orbs)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			teamCache.m_vAlliedOrbs = orbs;
		}
	}
}

void NodeSphere::setCachedEnemyOrbsCountForTeam(Team* team, int count)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			teamCache.m_iEnemyOrbsCount = count;
		}
	}
}

void NodeSphere::setCachedAlliedOrbsCountForTeam(Team* team, int count)
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		if (teamCache.m_pTeam->getID() == team->getID())
		{
			teamCache.m_iAlliedOrbsCount = count;
			teamCache.m_bAlliedOrbsCached = true;
		}
	}
}

void NodeSphere::resetOrbsCache()
{
	for (int teamCacheIndex = 0; teamCacheIndex != m_vTeamCaches.size(); teamCacheIndex++)
	{
		NodeSphereTeamCache &teamCache = m_vTeamCaches[teamCacheIndex];
		teamCache.m_bAlliedOrbsCached = false;
		teamCache.m_bEnemyOrbsCached = false;
		teamCache.m_iAlliedOrbsCount = -1;
		teamCache.m_iEnemyOrbsCount = -1;
	}
}

void NodeSphere::createMetaOrb(int metaOrbID, int initialNumberOfOrbs)
{
	MetaOrb* metaOrb = NULL;
	if (metaOrbID == VGUIManager::GetID(ID_META_ORB_GENERATION))
	{
		metaOrb = (MetaOrb*) Vision::Game.CreateEntity("MetaOrb", hkvVec3(0,0,0), "Models\\jupiter.model");
		this->generationMetaOrb = (MetaOrb*) metaOrb;
		generationMetaOrb->init(initialNumberOfOrbs, this, VGUIManager::GetID(ID_META_ORB_GENERATION));													
	}
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
	{
		metaOrb = (MetaOrb*) Vision::Game.CreateEntity("MetaOrb", hkvVec3(0,0,0), "Models\\jupiter.model");
		this->defenseMetaOrb = (MetaOrb*) metaOrb;
		defenseMetaOrb->init(initialNumberOfOrbs, this, VGUIManager::GetID(ID_META_ORB_DEFENSE));
	}
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
	{
		metaOrb = (MetaOrb*) Vision::Game.CreateEntity("MetaOrb", hkvVec3(0,0,0), "Models\\jupiter.model");
		this->assaultMetaOrb = (MetaOrb*) metaOrb;
		assaultMetaOrb->init(initialNumberOfOrbs, this, VGUIManager::GetID(ID_META_ORB_ASSAULT));
	}

	if (metaOrb)
	{
		//VisSurfaceTextureSet_cl* metaOrbTextureSet = metaOrb->CreateCustomTextureSet(true);
		//VisSurface_cl** surfaces = metaOrbTextureSet->AsSurfaceArray();
		//int surfacesCount = metaOrbTextureSet->GetSurfaceCount();
		//VisSurface_cl* metaOrbSurface = surfaces[0];
		//metaOrbSurface->SetTransparencyType(VIS_TRANSP_NONE);
		//metaOrbSurface->m_spDiffuseTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
		//metaOrbSurface->m_spNormalMap = Vision::TextureManager.Load2DTexture("Textures\\bump_noise.dds");
		//Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		//VTechniqueConfig techConfig;
		//techConfig.SetInclusionTags("LIGHTGRIDSIMPLE;LIGHTGRID;");
		//string strParamString;
		//strParamString += string("ScrollSpeed=0.2,0;");
		//strParamString += string("DiffuseTintColor=");
		//hkvVec4 colorVec4 = metaOrb->getColor().getAsVec4();
		//colorVec4.x *= 0.1f;
		//colorVec4.y *= 0.1f;
		//colorVec4.z *= 0.1f;
		//strParamString += stringFromFloat(colorVec4.x) + string(",") + stringFromFloat(colorVec4.y) + string(",") + stringFromFloat(colorVec4.z) + string(",") + stringFromFloat(colorVec4.w) + string(";");
		//VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("BumpTintColor", strParamString.c_str(), &techConfig);
		//metaOrbSurface->SetTechnique(pTechnique);
		//metaOrbSurface->SetLightingMode(VIS_LIGHTING_LIGHTGRID);
		//metaOrb->SetCustomTextureSet(metaOrbTextureSet);
		//metaOrbSurface->ReapplyShader(true);
	}

	if (metaOrb)
		createMetaOrbLink(metaOrb);
}

void NodeSphere::createMetaOrbLink(MetaOrb* metaOrb)
{
	MetaOrbLink* metaOrbLink = new MetaOrbLink();
	metaOrbLink->AddRef();
	metaOrbLink->init(this, metaOrb);

	metaOrbLinks.push_back(metaOrbLink);
}

void NodeSphere::removeMetaOrb(int metaOrbID)
{
	if (metaOrbID == VGUIManager::GetID(ID_META_ORB_GENERATION))
		Vision::Game.RemoveEntity(generationMetaOrb);
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_DEFENSE))
		Vision::Game.RemoveEntity(defenseMetaOrb);
	else if (metaOrbID == VGUIManager::GetID(ID_META_ORB_ASSAULT))
		Vision::Game.RemoveEntity(assaultMetaOrb);
}

void NodeSphere::removeMetaOrbLinkForMetaOrb(MetaOrb* metaOrb)
{
	vector<MetaOrbLink*>::iterator metaOrbLinksIt;
	for (metaOrbLinksIt = metaOrbLinks.begin(); metaOrbLinksIt != metaOrbLinks.end(); metaOrbLinksIt++)
	{
		if ((*metaOrbLinksIt)->getLinkedMetaOrb() == metaOrb)
		{
			metaOrbLinks.erase(metaOrbLinksIt);
			metaOrb->Release();
		}
	}
}

void NodeSphere::removeMetaOrbs()
{
	if (generationMetaOrb)
		Vision::Game.RemoveEntity(generationMetaOrb);
	if (defenseMetaOrb)
		Vision::Game.RemoveEntity(defenseMetaOrb);
	if (assaultMetaOrb)
		Vision::Game.RemoveEntity(assaultMetaOrb);

	removeMetaOrbLinks();
}

void NodeSphere::removeMetaOrbLinks()
{
	vector<MetaOrbLink*>::iterator metaOrbLinksIt;
	for (metaOrbLinksIt = metaOrbLinks.begin(); metaOrbLinksIt != metaOrbLinks.end(); metaOrbLinksIt++)
	{
		(*metaOrbLinksIt)->Release();
	}

	metaOrbLinks.clear();
}

void NodeSphere::hideMetaOrbs()
{
	if (generationMetaOrb)
		generationMetaOrb->SetVisibleBitmask(0);
	if (defenseMetaOrb)
		defenseMetaOrb->SetVisibleBitmask(0);
	if (assaultMetaOrb)
		assaultMetaOrb->SetVisibleBitmask(0);
}

void NodeSphere::showMetaOrbs()
{
	if (generationMetaOrb)
	{
		generationMetaOrb->SetVisibleBitmask(0xffffffff);
		generationMetaOrb->invalidateTechnique(this->team->getColor(), true);
	}
	if (defenseMetaOrb)
	{
		defenseMetaOrb->SetVisibleBitmask(0xffffffff);
		defenseMetaOrb->invalidateTechnique(this->team->getColor(), true);
		//defenseMetaOrb->invalidateTechnique(V_RGBA_WHITE, true);
	}
	if (assaultMetaOrb)
	{
		assaultMetaOrb->SetVisibleBitmask(0xffffffff);
		assaultMetaOrb->invalidateTechnique(this->team->getColor(), true);
		//assaultMetaOrb->invalidateTechnique(V_RGBA_WHITE, true);
	}
}

bool NodeSphere::hasAtLeastOneMetaOrb()
{
	if (this->isSacrificePit() || this->isArtifact())
		return false;

	if (generationMetaOrb)
		return true;
	if (defenseMetaOrb)
		return true;
	if (assaultMetaOrb)
		return true;

	return false;
}

void NodeSphere::updateAggregatingOrbs(int metaOrbID, int number)
{
	int currentAggregatingOrbsCount = 0;
	vector<Orb*> &orbsOfSphereTeam = this->getOrbsForTeam(this->team);
	for (int orbIndex = 0; orbIndex != orbsOfSphereTeam.size(); orbIndex++)
	{
		Orb* orb = orbsOfSphereTeam[orbIndex];
		if (orb->getStatus() == ORB_STATUS_AGGREGATING && orb->getAggregatingMetaOrb()->getID() == metaOrbID)
		{
			currentAggregatingOrbsCount++;
		}
	}

	int delta = number - currentAggregatingOrbsCount;

	if (delta > 0)
	{
		for (int orbIndex = 0; orbIndex != orbsOfSphereTeam.size(); orbIndex++)
		{
			Orb* orb = orbsOfSphereTeam[orbIndex];
			if (orb->getStatus() == ORB_STATUS_ORBITING)
			{
				orb->aggregateToMetaOrb(metaOrbID);
				delta--;
			}
			if (delta == 0)
				break;
		}
	}
	else if (delta < 0)
	{
		for (int orbIndex = 0; orbIndex != orbsOfSphereTeam.size(); orbIndex++)
		{
			Orb* orb = orbsOfSphereTeam[orbIndex];
			if (orb->getStatus() == ORB_STATUS_AGGREGATING && orb->getAggregatingMetaOrb()->getID() == metaOrbID)
			{
				orb->orbit(false);
				delta++;
			}
			if (delta == 0)
				break;
		}
	}
}

void NodeSphere::addPendingTravelingOrb(Orb* travelingOrb)
{
	this->pendingTravelingOrbs.push(travelingOrb);
}

void NodeSphere::addPendingTravelingOrbs(vector<Orb*> &travelingOrbs)
{
	for (int orbIndex = 0; orbIndex != travelingOrbs.size(); orbIndex++)
	{
		this->pendingTravelingOrbs.push(travelingOrbs[orbIndex]);
	}
}

void NodeSphere::processPendingOrbs()
{
	int processedOrbsCount = 0;
	while (this->pendingTravelingOrbs.size() != 0 && processedOrbsCount < PENDING_ORBS_PROCESSED_BY_FRAME)
	{
		Orb* pendingOrb = pendingTravelingOrbs.top();
		this->removeOrb(pendingOrb);
		pendingOrb->travel();
		pendingTravelingOrbs.pop();

		processedOrbsCount++;
	}
}

//void NodeSphere::takeBombHit(Bomb* bomb)
//{
//	int damage = GeometryUtils::round(bomb->getDamage());
//	this->life -= damage;
//	if (this->life <= 0)
//	{
//		this->life = (float) GeometryUtils::round(0.5f * maxLife); //the sphere gets back 50% of its max health points
//		Team* lastBombTeam = bomb->getTeam();
//		this->takeControl(lastBombTeam, true); //team that hits the sphere
//		if (GameManager::GlobalManager().getPickedNodeSphere())
//			GameDialogManager::sharedInstance()->getHUD()->showCorePanel();
//	}
//
//	this->removeTrackingBomb(bomb);
//	bomb->destroySelf();
//}
//
//void NodeSphere::addTrackingBomb(Bomb* bomb)
//{
//	this->trackingBombs.push_back(bomb);
//}
//
//void NodeSphere::removeTrackingBomb(Bomb* bomb)
//{
//	vector<Bomb*>::iterator trackingBombsIt;
//	for (trackingBombsIt = trackingBombs.begin(); trackingBombsIt != trackingBombs.end(); trackingBombsIt++)
//	{
//		if (bomb == *trackingBombsIt)
//		{
//			trackingBombs.erase(trackingBombsIt);
//			return;
//		}
//	}
//}
//
//void NodeSphere::destroyAllTrackingBombs()
//{
//	for (int trackingBombIndex = 0; trackingBombIndex != trackingBombs.size(); trackingBombIndex++)
//	{
//		trackingBombs[trackingBombIndex]->destroySelf();
//	}
//
//	//clear the vector (not really needed as the soldier is destroyed then)
//	this->trackingBombs.clear();
//}

vector<Bridge*> NodeSphere::findPotentialBridges()
{
	vector<Bridge*> potentialBridges;

	vector<NodeSphere*> &allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		bool pushNodeSphere = true;
		NodeSphere* currentNodeSphere = allNodeSpheres[sphereIndex];

		//same sphere
		if (this == currentNodeSphere)
		{
			pushNodeSphere = false;
			continue;
		}

		if (currentNodeSphere->containsNeighbouringSphere(this) && !this->containsNeighbouringSphere(currentNodeSphere)
			||
			this->containsNeighbouringSphere(currentNodeSphere) && !currentNodeSphere->containsNeighbouringSphere(this))
		{
			//check if a bridge is not already in construction
			bool bridgeAlreadyExists = false;
			vector<Bridge*> &allBridges = GameManager::GlobalManager().getBridges();
			for (int bridgeIndex = 0; bridgeIndex != allBridges.size(); bridgeIndex++)
			{
				if (allBridges[bridgeIndex]->equals(this, currentNodeSphere))
				{
					bridgeAlreadyExists = true;
					break;
				}
			}
			
			if (!bridgeAlreadyExists)
			{
				//potential bridge sphere found
				float bridgeLength = (this->GetPosition() - currentNodeSphere->GetPosition()).getLength() 
					- this->getRadius() 
					- currentNodeSphere->getRadius();
				int bridgeCost = GeometryUtils::round(bridgeLength * BRIDGE_COST_FACTOR);
				Bridge* potentialBridge = new PotentialBridge();
				potentialBridge->AddRef();
				potentialBridge->init(this, currentNodeSphere);
				potentialBridges.push_back(potentialBridge);
			}
		}
	}

	return potentialBridges;
}

bool NodeSphere::buildBridge(NodeSphere* endSphere)
{
	float bridgeLength = (endSphere->GetPosition() - this->GetPosition()).getLength();
	int bridgeCost = GeometryUtils::round(bridgeLength * BRIDGE_COST_FACTOR);

	vector<Orb*> &sphereTeamOrbs  = this->getOrbsForTeam(this->team);
	int sphereTeamOrbsCount = 0;
	for (int orbIndex = 0; orbIndex != sphereTeamOrbs.size(); orbIndex++)
	{
		if (sphereTeamOrbs[orbIndex]->getStatus() == ORB_STATUS_ORBITING)
			sphereTeamOrbsCount++;
	}
	bool canBuild = (sphereTeamOrbsCount >= bridgeCost);

	if (canBuild)
	{
		Bridge* bridge = new Bridge();
		bridge->AddRef();
		bridge->init(this, endSphere, false);
		bridge->setRevealed(true);
		GameManager::GlobalManager().addBridge(bridge);
		
		vector<Orb*> &alliedOrbs = this->getOrbsForTeam(TeamManager::sharedInstance().getDefaultTeam());
		int orbIndex = 0;
		int orbsCount = 0;
		while (orbsCount != bridgeCost)
		{
			Orb* alliedOrb = alliedOrbs[orbIndex];
			if (alliedOrbs[orbIndex]->getStatus() == ORB_STATUS_ORBITING)
			{
				alliedOrb->startBuildingBridge(bridge);
				orbsCount++;
			}

			orbIndex++;
		}
	}
	else
		MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("need_more_orbs"));

	return canBuild;
}

void NodeSphere::addNeighbouringSphere(NodeSphere* sphere)
{
	this->neighbouringSpheres.push_back(sphere);
}

void NodeSphere::purgeParticleEffects()
{
	this->m_pFxTakeControlParticleEffectFile = NULL;
}

void NodeSphere::setVisibilityStatus(int visibilityStatus)
{
	this->visibilityStatus = visibilityStatus;
	if (visibilityStatus == VISIBILITY_STATUS_VISIBLE || visibilityStatus == VISIBILITY_STATUS_HIDDEN || visibilityStatus == VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR)
	{
		bool bVisibleFromOrbs = (visibilityStatus == VISIBILITY_STATUS_VISIBLE); //visibility from orbs verification has already been done in updateVisibilityStatus function
		if (this->isSacrificePit())
		{
		}
		else
			this->invalidateTechnique(this->team->getColor(), bVisibleFromOrbs);
	}
	else if (visibilityStatus == VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY)
	{
		this->SetVisibleBitmask(0);
	}
}

void NodeSphere::updateVisibilityStatus()
{
#ifdef MAKE_ALL_SCENE_ELEMENTS_VISIBLE
	return;
#endif


	if (this->team && this->team == TeamManager::sharedInstance().getDefaultTeam()) //common node sphere
	{
		this->visibilityStatus = VISIBILITY_STATUS_VISIBLE;
		return;
	}

	int previousVisibilityStatus = this->getVisibilityStatus();
	if (isVisibleFromDefaultTeamOrbs())
	{
		this->setVisibilityStatus(VISIBILITY_STATUS_VISIBLE); //make it visible

		if (previousVisibilityStatus == VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR || previousVisibilityStatus == VISIBILITY_STATUS_HIDDEN) //we just revealed this sphere
		{
			if (this == GameManager::GlobalManager().getPickedNodeSphere())
			{
				GameDialogManager::sharedInstance()->getHUD()->showCorePanel();
			}

			if (previousVisibilityStatus == VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR) //reveal nearby spheres and bridges
				this->revealNeighbouringSpheresAroundVisibleSphere();

			//show meta orbs and badge
			showMetaOrbs();
			if (this->badge)
				this->badge->SetOpacity(0);
		}
	}
	else
	{
		if (previousVisibilityStatus == VISIBILITY_STATUS_VISIBLE) //orbs just left this sphere
		{
			if (this == GameManager::GlobalManager().getPickedNodeSphere())
			{
				GameDialogManager::sharedInstance()->getHUD()->dismissCorePanel();
				GameDialogManager::sharedInstance()->getHUD()->dismissInfoPanel();
			}

			this->setVisibilityStatus(VISIBILITY_STATUS_HIDDEN);
			hideMetaOrbs();
		}
	}
}

void NodeSphere::revealNeighbouringSpheresAroundVisibleSphere()
{
	vector<NodeSphere*> &allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		NodeSphere* nodeSphere = allNodeSpheres[sphereIndex];
		if (nodeSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
			continue;
		if (this->getNeighbouringSphereStatus(nodeSphere) == NEIGHBOURING_SPHERE_STATUS_NORMAL || 
			this->getNeighbouringSphereStatus(nodeSphere) == NEIGHBOURING_SPHERE_STATUS_ISOLATED)
		{
			nodeSphere->setVisibilityStatus(VISIBILITY_STATUS_UNDISCOVERED_NEIGHBOR);
			nodeSphere->SetVisibleBitmask(0xffffffff);
			if (badge)
				badge->SetOpacity(0);

			if (this->getNeighbouringSphereStatus(nodeSphere) == NEIGHBOURING_SPHERE_STATUS_NORMAL)
			{
				Bridge* bridge = GameManager::GlobalManager().findBridge(this, nodeSphere);
				if (bridge)
					bridge->setRevealed(true);
			}
		}
	}
}

bool NodeSphere::isVisibleFromDefaultTeamOrbs()
{
	Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
	vector<Orb*>& defaultTeamOrbs = this->getOrbsForTeam(defaultTeam);
	for (int orbIndex = 0; orbIndex != defaultTeamOrbs.size(); orbIndex++)
	{
		hkvVec3 orbPosition = defaultTeamOrbs[orbIndex]->GetPosition();
		float distanceToNodeSphereCenter = (orbPosition - this->GetPosition()).getLength();
		if (distanceToNodeSphereCenter < this->getOuterRadius())
		{
			return true;
		}
	}

	vector<TeamedOrbs> &defaultTeamTravelingOrbs = GameManager::GlobalManager().getTravelingOrbs();
	for (int teamedOrbsIndex = 0; teamedOrbsIndex != defaultTeamTravelingOrbs.size(); teamedOrbsIndex++)
	{
		if (defaultTeamTravelingOrbs[teamedOrbsIndex].team == defaultTeam)
		{
			vector<Orb*> &travelingOrbs = defaultTeamTravelingOrbs[teamedOrbsIndex].orbs;
			for (int orbIndex = 0; orbIndex != travelingOrbs.size(); orbIndex++)
			{
				hkvVec3 orbPosition = travelingOrbs[orbIndex]->GetPosition();
				float distanceToNodeSphereCenter = (orbPosition - this->GetPosition()).getLength();
				if (distanceToNodeSphereCenter < this->getOuterRadius())
				{
					return true;
				}
			}
		}
	}
	return false;
}

//void NodeSphere::invalidateStandardTechnique()
//{
//	Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
//	VTechniqueConfig techConfig;
//	//techConfig.SetInclusionTags("LIGHTGRIDSIMPLE;LIGHTGRID;CUSTOM_STATIC_LIGHT_DIR");
//	techConfig.SetInclusionTags("LIGHTGRIDSIMPLE;LIGHTGRID");
//
//	//parameters
//	string strParamString;
//	string strDiffuseTintColor;
//	if (this->team)
//	{
//		hkvVec4 sphereTeamColor;
//		VColorRef::RGBA_To_Float(this->team->getColor(), sphereTeamColor);
//		strDiffuseTintColor = string("DiffuseTintColor=") + 
//			stringFromFloat(sphereTeamColor.x) + string(",") +
//			stringFromFloat(sphereTeamColor.y) + string(",") +
//			stringFromFloat(sphereTeamColor.z) + string(",") +
//			stringFromFloat(sphereTeamColor.w) + string(";");	
//	}
//	else //neutral sphere
//	{
//		hkvVec4 neutralTeamColor;
//		VColorRef::RGBA_To_Float(TEAM_NEUTRAL_COLOR, neutralTeamColor);
//		strDiffuseTintColor = string("DiffuseTintColor=") + 
//			stringFromFloat(neutralTeamColor.x) + string(",") +
//			stringFromFloat(neutralTeamColor.y) + string(",") +
//			stringFromFloat(neutralTeamColor.z) + string(",") +
//			stringFromFloat(neutralTeamColor.w) + string(";");	
//	}
//
//	//TODO set the correct color
//	strDiffuseTintColor = "DiffuseTintColor=1,1,1,1;";
//
//	strParamString += strDiffuseTintColor;
//
//	float scrollSpeedX = 0.0f;
//	float scrollSpeedY = 0.0f;
//	string strScrollSpeed = string("ScrollSpeed=") + stringFromFloat(scrollSpeedX) + string(",") + stringFromFloat(scrollSpeedY) + string(";");
//	strParamString += strScrollSpeed;
//	//string strStaticLightDirValue = string("StaticLightDirValue=") + stringFromFloat(cameraDirection.x) + string(",") + stringFromFloat(cameraDirection.y) + string(",") + stringFromFloat(cameraDirection.z) + string(";");		
//	//strParamString += strStaticLightDirValue;
//	//hkvLog::Error("sphereTeam:%i paramString:%s", this->team->getID(), strParamString.c_str());
//
//	this->m_pStdTechnique = Vision::Shaders.CreateTechnique("BumpTintColor", strParamString.c_str(), &techConfig);
//}

void NodeSphere::update(float dt)
{
	//reset the cache
	resetOrbsCache();

	//process any pending orbs on traveling mode
	processPendingOrbs();

	//apply core effects on orbs
	generateOrbs(dt);

	//update visibility status
	updateVisibilityStatus();

	//update position of the badge
	//if (badge)
	//	badge->updatePosition();
}

/////////////////////////////////SphereBadge/////////////////////////////////
SphereBadge::SphereBadge() : parentNodeSphere(NULL)
{
	Vision::Callbacks.OnRenderHook += this;
}

SphereBadge::~SphereBadge()
{
	Vision::Callbacks.OnRenderHook -= this;
}

void SphereBadge::init(VTextureObject* textureObj, NodeSphere* parentNodeSphere)
{
	this->parentNodeSphere = parentNodeSphere;

	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 badgeSize = hkvVec2(256.0f * scale, 128.0f * scale);
	SpriteScreenMask::init(hkvVec2(0,0), badgeSize, textureObj);
	this->SetOrder(SPHERE_BADGE_ORDER);
	updatePosition();
}

void SphereBadge::updatePosition()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	float screenProjectionX, screenProjectionY;
	VisRenderContext_cl::GetCurrentContext()->Project2D(this->parentNodeSphere->GetPosition(), screenProjectionX, screenProjectionY);
	hkvVec2 parentSphereProjectedPosition = hkvVec2(screenProjectionX, screenProjectionY);
	float zoomPercentage = GameManager::GlobalManager().getMainCamera()->getZoomPercentage();
	float verticalOffset = 150.0f * scale * (2 - zoomPercentage);
	hkvVec2 badgePosition = hkvVec2(parentSphereProjectedPosition.x, parentSphereProjectedPosition.y - verticalOffset);
	this->SetPosition(badgePosition);
}

void SphereBadge::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
		if (pRHDO->m_iEntryConst == VRH_PRE_SCREENMASKS)
		{
			updatePosition();
		}
	}
}

/////////////////////////////////SphereGlowRenderLoop/////////////////////////////////
class SphereGlowRenderLoop : public VisionRenderLoop_cl
{
public:
	// Constructor / Destructor
	SphereGlowRenderLoop()
	{
		LoadShaders();
		m_spFont = Vision::Fonts.LoadFont("Fonts\\Arial_22.fnt");
	}
	virtual ~SphereGlowRenderLoop()
	{
	}

	// Render Loop implementation
	virtual void OnDoRenderLoop(void *pUserData) HKV_OVERRIDE
	{
		IVisVisibilityCollector_cl *pCollector = Vision::Contexts.GetCurrentContext()->GetVisibilityCollector();
		// fall back to original renderloop
		if (pCollector==NULL || (m_spThermalImageLightmap==NULL && m_spThermalImageLightGrid==NULL))
		{
			VisionRenderLoop_cl::OnDoRenderLoop(pUserData);
			return;
		}

		Vision::RenderLoopHelper.ClearScreen();

		// render all visible entities
		const VisEntityCollection_cl *pEntities = pCollector->GetVisibleEntities();
		if (pEntities!=NULL && m_spThermalImageLightGrid!=NULL)
		{
			for (int i=0;i<(int)pEntities->GetNumEntries();i++)
				Vision::RenderLoopHelper.RenderEntityWithShaders(pEntities->GetEntry(i),
				m_spThermalImageLightGrid->GetShaderCount(), m_spThermalImageLightGrid->GetShaderList());
		}

		//// render all visible static geometry (does not consider terrain!)
		//const VisStaticGeometryInstanceCollection_cl *pStaticGeom = pCollector->GetVisibleStaticGeometryInstances();
		//if (pStaticGeom!=NULL && m_spThermalImageLightmap!=NULL)
		//{
		//  for (int i=0;i<m_spThermalImageLightmap->GetShaderCount();i++)
		//    Vision::RenderLoopHelper.RenderStaticGeometryWithShader(*pStaticGeom, *m_spThermalImageLightmap->GetShader(i));
		//}

		//// render some text overlay:
		//hkvVec2 vPos(1.f,1.f);
		//IVRender2DInterface *pRI = Vision::RenderLoopHelper.BeginOverlayRendering();
		//  m_spFont->PrintText(pRI, vPos, "Thermal Image", V_RGBA_WHITE);
		//Vision::RenderLoopHelper.EndOverlayRendering();
	}

private:
	void LoadShaders()
	{
		// load shaders that should be used in this renderloop
		BOOL bRes = Vision::Shaders.LoadShaderLibrary("Shaders\\OrbsShaders.ShaderLib") != NULL;
		VASSERT_MSG(bRes, "failed to load OrbsShaders.ShaderLib");

		m_spThermalImageLightmap = Vision::Shaders.CreateTechnique("ThermalImageLightmap", NULL);
		m_spThermalImageLightGrid = Vision::Shaders.CreateTechnique("ThermalImageLightGrid", NULL);
	}

	VCompiledTechniquePtr m_spThermalImageLightmap;
	VCompiledTechniquePtr m_spThermalImageLightGrid;
	VisFontPtr m_spFont;
};

/////////////////////////////SphereGlowHalo/////////////////////////////
void SphereGlowHalo::init(hkvVec3 position, hkvVec2 size, VisBaseEntity_cl* parentSphereEntity)
{
	BatchSprite::init(position, size);
	this->m_pParentSphereEntity = parentSphereEntity;
}

void SphereGlowHalo::faceCamera()
{
	MainCamera* mainCamera = GameManager::GlobalManager().getMainCamera();
	hkvVec3 cameraVec = this->GetPosition() - mainCamera->GetPosition();
	hkvVec3 projectionAlongX = hkvVec3(0, cameraVec.y, cameraVec.z);
	hkvVec3 projectionAlongY = hkvVec3(cameraVec.x, 0, cameraVec.z);
	hkvVec3 projectionAlongZ = hkvVec3(cameraVec.x, cameraVec.y, 0);
	float yaw = atan2(projectionAlongZ.y, projectionAlongZ.x) * HKVMATH_RAD_TO_DEG;
	float pitch = atan2(projectionAlongZ.z - cameraVec.z, projectionAlongZ.getLength()) * HKVMATH_RAD_TO_DEG;
	float roll = 0; //roll cannot be defined, but we do not need it because the halo is symmetrical, so set it to zero

	VSmartPtr<VisObject3D_cl> tmp3DObject = new VisObject3D_cl();
	tmp3DObject->SetUseEulerAngles(TRUE);
	tmp3DObject->SetOrientation(yaw, pitch, 0);
	this->rotationMatrix = tmp3DObject->GetRotationMatrix();

	hkvVec3 zAxis = this->rotationMatrix.transformDirection(hkvVec3(0,0,1));
	hkvMat3 localZRotationMatrix = hkvMat3(hkvNoInitialization);
	localZRotationMatrix.setRotationMatrixZ(-90);
	this->rotationMatrix = this->rotationMatrix.multiply(localZRotationMatrix);
}