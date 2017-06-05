#pragma once

#define BRIDGE_UV_SCROLL_DEFAULT_SPEED hkvVec2(0.2f, 0.2f)

struct TeamedOrbs;

struct MeshVertex
{
	hkvVec3 vertexCoords;
	hkvVec3 normal;
	VColorRef color;
	hkvVec2 texCoords;
};

class MeshRenderer :  public IVisCallbackHandler_cl
{
public:
	MeshRenderer(){};
	void OneTimeInit();
	void OneTimeDeInit();

	static MeshRenderer& sharedInstance(){return instance;};
	void initDescriptor();

	void obtainAllMeshes();
	void clearMeshesVisibilityStatus();
	void renderAllMeshes();

	void renderHalosByTeam(Team* team, const vector<NodeSphere*> &allNodeSpheres);
	void renderOrbsByTeam(Team* team, const vector<vector<Orb*>* > &orbs);
	void renderTravelingOrbsByTeam(Team* team, vector<TeamedOrbs> &travelingOrbs);
	void renderBullets();
	//void renderBombs();
	void renderBridges(const vector<Bridge*> &bridges);
	void renderPotentialBridges(vector<Bridge*> potentialBridges);
	void renderMetaOrbLinks();

	void obtainMetaOrbsLinks(vector<vector<MetaOrbLink*>* > &metaOrbLinks);
	void obtainOrbsAndBullets(Team * team, vector<vector<Orb*>* > &orbs, vector<vector<Bullet*>* > &bullets);
	//void obtainBombs(vector<vector<Bomb*>* > &bombs);

	//void allocateVertices(VisMeshBuffer_cl* vertexBuffer, const vector<BatchSprite*> &allSprites, hkvVec2 textureSize, vector<hkvVec4> &textureRanges);
	void allocatePotentialBridgesVertices(VisMeshBuffer_cl* vertexBuffer, const vector<Bridge*> &potentialBridges, int potentialBridgesCount, hkvVec2 textureSize);
	void allocateBridgesVertices(VisMeshBuffer_cl* vertexBuffer, const vector<Bridge*> &allBridges, int numVertices, int bridgesCount, int numCylindersPerBridge, int numVerticalSlicesPerCylinder, float distanceBetweenTwoHorizontalSlices, hkvVec2 textureSize);
	void allocateMetaOrbLinksVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<MetaOrbLink*>* > &metaOrbLinks, int numVertices, int metaOrbLinksCount, int numCylindersPerMetaOrbLink, int numVerticalSlicesPerCylinder, float distanceBetweenTwoHorizontalSlices, hkvVec2 textureSize);
	//void allocateMetaOrbLinksVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<MetaOrbLink*>* > &metaOrbLinks, int metaOrbLinksCount, int numSlicesPerLink);
	void allocateHalosVerticesForTeam(Team* team, VisMeshBuffer_cl* vertexBuffer, const vector<NodeSphere*> &allNodeSpheres, int halosCount, hkvVec2 textureSize, hkvVec4 textureRange);
	void buildVertexAndTexCoordsForHalo(SphereGlowHalo* sphereHalo, MeshVertex* pVertex, int haloGlobalIndex, hkvVec2 textureSize, hkvVec4 textureRange);
	void allocateOrbsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Orb*>* > &allOrbs, int orbsCount, hkvVec2 textureSize, hkvVec4 textureRange); //special case for orbs
	void allocateTravelingOrbsVerticesForTeam(VisMeshBuffer_cl* vertexBuffer, Team* team, vector<TeamedOrbs> &travelingOrbs, int travelingOrbsCount);
	void allocateBulletsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Bullet*>* > &allBullets, int bulletsCount, hkvVec2 textureSize, hkvVec4 textureRange); //same for bullets
	//void allocateBombsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Bomb*>* > &allBombs, int bombsCount, hkvVec2 textureSize, hkvVec4 textureRange); //and bombs

	void allocateBridgesIndices(VisMeshBuffer_cl* indexBuffer, int numCylindersPerBridge, int numVerticalSlicesPerCylinder, vector<int> &numHorizontalSlices);
	void allocateQuadIndices(VisMeshBuffer_cl* indexBuffer, int numIndices);

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

private:
	static MeshRenderer instance;

	VisMBVertexDescriptor_t m_descriptor;

	//all meshes that will enter visibility test
	vector<vector<MetaOrbLink*>* > metaOrbLinks;
	vector<vector<vector<Orb*>* > > teamedOrbs;
	vector<vector<Bullet*>* > bullets;
	//vector<vector<Bomb*>* > bombs;

	//tmp DEBUG
	int orbsRenderedCount;
	int orbsNotRenderedCount;
	int bridgesRenderedCount;
	int bridgesNotRenderedCount;
	int	linksRenderedCount;
	int	linksNotRenderedCount;
};