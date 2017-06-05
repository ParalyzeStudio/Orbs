#include "OrbsPluginPCH.h"
#include "MeshRenderer.h"
#include "GameManager.h"

MeshRenderer MeshRenderer::instance;

void MeshRenderer::OneTimeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
	Vision::Callbacks.OnVisibilityPerformed += this;
	Vision::Callbacks.OnRenderHook += this;

	initDescriptor();
}


void MeshRenderer::OneTimeDeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
	Vision::Callbacks.OnVisibilityPerformed -= this;
	Vision::Callbacks.OnRenderHook -= this;
}

void MeshRenderer::initDescriptor()
{
	m_descriptor.m_iStride = sizeof(MeshVertex);
	m_descriptor.m_iPosOfs = offsetof(MeshVertex, vertexCoords)/* | VERTEXDESC_FORMAT_FLOAT3*/;
	m_descriptor.m_iNormalOfs = offsetof(MeshVertex, normal);
	m_descriptor.m_iColorOfs  = offsetof(MeshVertex, color);
	m_descriptor.m_iTexCoordOfs[0] = offsetof(MeshVertex, texCoords)/* | VERTEXDESC_FORMAT_FLOAT2*/;
}

void MeshRenderer::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (!SceneManager::sharedInstance().isCurrentSceneLevel())
		return;
	if (pData->m_pSender == &Vision::Callbacks.OnVisibilityPerformed)
	{
		VisRenderContext_cl *pContext = VisRenderContext_cl::GetCurrentContext();
		IVisVisibilityCollector_cl *pVisCollector = pContext->GetVisibilityCollector();

		if (pVisCollector)
		{
			obtainAllMeshes();
			clearMeshesVisibilityStatus();

			const VisVisibilityObjectCollection_cl* pVisObjectCollection = pVisCollector->GetVisibleVisObjects();
			if (pVisObjectCollection != NULL && pVisObjectCollection->GetNumEntries()>0)
			{
				int iNumEntries = pVisObjectCollection->GetNumEntries();
				VisVisibilityObject_cl **pVisObjects = pVisObjectCollection->GetDataPtr();
				for (int iVisObjectIndex = 0; iVisObjectIndex < iNumEntries; iVisObjectIndex++)
				{
					VisVisibilityObject_cl* pVisObject = pVisObjects[iVisObjectIndex];
					if (vdynamic_cast<BatchSpriteVisibilityObject*>(pVisObject))
					{
						BatchSpriteVisibilityObject* pBatchSpriteVisObject = (BatchSpriteVisibilityObject*) pVisObject;
						vector<BatchSprite*> &attachedBatchSprites = pBatchSpriteVisObject->getAttachedBatchSprites();
						for (int spriteIndex = 0; spriteIndex != attachedBatchSprites.size(); spriteIndex++)
						{
							attachedBatchSprites[spriteIndex]->passVisibilityTest();
						}
					}
				}
			}
		}
	}
	else if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		//hkvLog::Error("+++++Vision::Callbacks.OnUpdateSceneBegin");
	}
	else if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
		if (pRHDO->m_iEntryConst == VRH_PRE_SCREENMASKS)
		{	
			renderAllMeshes();
		}
	}
}

void MeshRenderer::obtainAllMeshes()
{
	//clear all vectors
	metaOrbLinks.clear();
	bullets.clear();
	teamedOrbs.clear();
	//bombs.clear();

	//meta-orb links
	this->obtainMetaOrbsLinks(metaOrbLinks);
	//orbs and bullets
	vector<Team*> teams = TeamManager::sharedInstance().getAllTeams();
	int teamCount = TeamManager::sharedInstance().getTeamsCount();
	teamedOrbs.reserve(teamCount);
	for (int teamIndex = 0; teamIndex!= teams.size(); teamIndex++)
	{
		vector<vector<Orb*>* > orbs;
		this->obtainOrbsAndBullets(teams[teamIndex], orbs, bullets);
		teamedOrbs.push_back(orbs);
	}
	//bombs
	//this->obtainBombs(bombs);
}

void MeshRenderer::clearMeshesVisibilityStatus()
{
	//potential bridges
	GameHUD* gameHUD = GameDialogManager::sharedInstance()->getHUD();
	if (gameHUD)
	{
		vector<Bridge*> &potentialBridges = GameDialogManager::sharedInstance()->getHUD()->getPotentialBridges();
		for (int potentialBridgeIndex = 0; potentialBridgeIndex != potentialBridges.size(); potentialBridgeIndex++)
		{
			potentialBridges[potentialBridgeIndex]->clearVisibilityTestValue();
		}
	}
	//meta-orb links
	for (int metaOrbLinksVecIndex = 0; metaOrbLinksVecIndex != metaOrbLinks.size(); metaOrbLinksVecIndex++)
	{
		vector<MetaOrbLink*>& metaOrbLinksVec = *metaOrbLinks[metaOrbLinksVecIndex];
		for (int metaOrbLinkIndex = 0; metaOrbLinkIndex != metaOrbLinksVec.size(); metaOrbLinkIndex++)
		{
			metaOrbLinksVec[metaOrbLinkIndex]->clearVisibilityTestValue();
		}
	}
	//traveling orbs
	vector<TeamedOrbs> &travelingOrbs = GameManager::GlobalManager().getTravelingOrbs();
	for (int travelingOrbIndex = 0; travelingOrbIndex != travelingOrbs.size(); travelingOrbIndex++)
	{
		vector<Orb*>& orbs = travelingOrbs[travelingOrbIndex].orbs;
		for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
		{
			orbs[orbIndex]->clearVisibilityTestValue();
		}
	}
	//orbs
	for (int teamedOrbsIndex = 0; teamedOrbsIndex != teamedOrbs.size(); teamedOrbsIndex++)
	{
		vector<vector<Orb*>* >& sphereOrbsVec = teamedOrbs[teamedOrbsIndex];
		for (int sphereOrbsVecIndex = 0; sphereOrbsVecIndex != sphereOrbsVec.size(); sphereOrbsVecIndex++)
		{
			vector<Orb*> orbs = *sphereOrbsVec[sphereOrbsVecIndex];
			for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
			{
				orbs[orbIndex]->clearVisibilityTestValue();
			}
		}
	}
	//bullets
	for (int bulletsVecIndex = 0; bulletsVecIndex != bullets.size(); bulletsVecIndex++)
	{
		vector<Bullet*>& bulletsVec = *bullets[bulletsVecIndex];
		for (int bulletIndex = 0; bulletIndex != bulletsVec.size(); bulletIndex++)
		{
			bulletsVec[bulletIndex]->clearVisibilityTestValue();
		}
	}
	//bombs
	//for (int bombsVecIndex = 0; bombsVecIndex != bombs.size(); bombsVecIndex++)
	//{
	//	vector<Bomb*>& bombsVec= *bombs[bombsVecIndex];
	//	for (int bombIndex = 0; bombIndex != bombsVec.size(); bombIndex++)
	//	{
	//		bombsVec[bombIndex]->clearVisibilityTestValue();
	//	}
	//}
	//bridges
	vector<Bridge*> &allBridges = GameManager::GlobalManager().getBridges();
	for (int bridgeIndex = 0; bridgeIndex != allBridges.size(); bridgeIndex++)
	{
		allBridges[bridgeIndex]->clearVisibilityTestValue();
	}
}

void MeshRenderer::renderAllMeshes()
{
	//render spheres halos
	vector<NodeSphere*> &allSpheres = GameManager::GlobalManager().getNodeSpheres();
	vector<Team*> teams = TeamManager::sharedInstance().getAllTeams();
	for (int teamIndex = 0; teamIndex!= teams.size(); teamIndex++)
	{
		renderHalosByTeam(teams[teamIndex], allSpheres);
	}

	//render potential bridges
	GameHUD* gameHUD = GameDialogManager::sharedInstance()->getHUD();
	if (gameHUD)
	{
		vector<Bridge*> &potentialBridges = GameDialogManager::sharedInstance()->getHUD()->getPotentialBridges();
		renderPotentialBridges(potentialBridges);
	}

	//render meta orbs links
	//renderMetaOrbLinks();

	//Vision::Message.Print(0, 10, 688, "linksRendered:%i", linksRenderedCount);
	//Vision::Message.Print(0, 10, 708, "linksNotRenderedCount:%i", linksNotRenderedCount);

	//render orbs (1 meshbuffer per team + meshbuffer for traveling orbs)
	vector<TeamedOrbs> &travelingOrbs = GameManager::GlobalManager().getTravelingOrbs();

	orbsNotRenderedCount = 0;
	orbsRenderedCount = 0;

	for (int teamIndex = 0; teamIndex!= teams.size(); teamIndex++)
	{
		renderOrbsByTeam(teams[teamIndex], teamedOrbs[teamIndex]);
		renderTravelingOrbsByTeam(teams[teamIndex], travelingOrbs);
	}
	
	//Vision::Message.Print(0, 10, 648, "orbsRendered:%i", orbsRenderedCount);
	//Vision::Message.Print(0, 10, 668, "orbsNotRendered:%i", orbsNotRenderedCount);

	//render bullets
	renderBullets();

	//render bombs
	//renderBombs();

	//render bridges
	vector<Bridge*> &allBridges = GameManager::GlobalManager().getBridges();
	renderBridges(allBridges);

	//Vision::Message.Print(0, 10, 608, "bridgesRendered:%i", bridgesRenderedCount);
	//Vision::Message.Print(0, 10, 628, "bridgesNotRendered:%i", bridgesNotRenderedCount);
}

void MeshRenderer::renderHalosByTeam(Team* team, const vector<NodeSphere*> &allNodeSpheres)
{
	if (team == TeamManager::sharedInstance().findTeamById(TEAM_0))
		int a = 1;

	int numSpritesToRender = 0;
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		NodeSphere* nodeSphere = allNodeSpheres[sphereIndex];
		if (nodeSphere->getTeam() == team)
		{
			SphereGlowHalo* halo = nodeSphere->getHalo();
			if (halo->hasPassedVisibilityTest() && nodeSphere->getVisibilityStatus() != VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY)
			{
				numSpritesToRender++;
				MetaOrb* generationMetaOrb = nodeSphere->getGenerationMetaOrb();
				MetaOrb* defenseMetaOrb = nodeSphere->getDefenseMetaOrb();
				MetaOrb* assaultMetaOrb = nodeSphere->getAssaultMetaOrb();
				if (generationMetaOrb && generationMetaOrb->GetVisibleBitmask() == 0xffffffff)
					numSpritesToRender++;
				else if (defenseMetaOrb && defenseMetaOrb->GetVisibleBitmask() == 0xffffffff)
					numSpritesToRender++;
				else if (assaultMetaOrb && assaultMetaOrb->GetVisibleBitmask() == 0xffffffff)
					numSpritesToRender++;
			}
		}
	}

	if (numSpritesToRender > 0)
	{
		int textureWidth, textureHeight, textureDepth;
		VTextureObject* haloTexture = Vision::TextureManager.Load2DTexture("Textures\\sphere_halo.png");
		haloTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
		hkvVec4 textureRange = hkvVec4(0, 0, textureSize.x, textureSize.y);
		VSmartPtr<VisMeshBuffer_cl> haloVertexBuffer = new VisMeshBuffer_cl();
		this->allocateHalosVerticesForTeam(team, haloVertexBuffer, allNodeSpheres, numSpritesToRender, textureSize, textureRange);
		VSmartPtr<VisMeshBuffer_cl> haloIndexBuffer = new VisMeshBuffer_cl();
		this->allocateQuadIndices(haloIndexBuffer, 6 * numSpritesToRender);

		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(haloVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(haloIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(haloTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		hkvVec4 floatColor;
		VColorRef::RGBA_To_Float(team->getColor(), floatColor);
		string colorParamString = string("Color=") + 
			stringFromFloat(floatColor.x) + string(",") +
			stringFromFloat(floatColor.y) + string(",") +
			stringFromFloat(floatColor.z) + string(",") +
			stringFromFloat(floatColor.w);

		//TODO remove and set team color
		//colorParamString = "Color=1,1,1,1;";

		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlend", colorParamString.c_str());
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
}

void MeshRenderer::renderOrbsByTeam(Team* team, const vector<vector<Orb*>* > &orbs)
{
	int numSpritesToRender = 0;
	for (int orbsVecIndex = 0; orbsVecIndex != orbs.size(); orbsVecIndex++)
	{
		vector<Orb*>& orbsVec = *orbs[orbsVecIndex];
		for (int orbIndex = 0; orbIndex != orbsVec.size(); orbIndex++)
		{
			if (orbsVec[orbIndex]->hasPassedVisibilityTest() && orbsVec[orbIndex]->isVisibleForPlayer())
			{
				orbsRenderedCount++;
				numSpritesToRender++;
			}
			else
				orbsNotRenderedCount++;
		}
	}

	if (numSpritesToRender > 0)
	{
		int textureWidth, textureHeight, textureDepth;
		VTextureObject* orbTexture = Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
		orbTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
		hkvVec4 textureRange = hkvVec4(0, 0, textureSize.x, textureSize.y);
		VSmartPtr<VisMeshBuffer_cl> orbVertexBuffer = new VisMeshBuffer_cl();
		this->allocateOrbsVertices(orbVertexBuffer, orbs, numSpritesToRender, textureSize, textureRange);
		VSmartPtr<VisMeshBuffer_cl> orbIndexBuffer = new VisMeshBuffer_cl();
		this->allocateQuadIndices(orbIndexBuffer, 6 * numSpritesToRender);

		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(orbVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(orbIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(orbTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		hkvVec4 floatColor;
		VColorRef::RGBA_To_Float(team->getColor(), floatColor);
		string colorParamString = string("Color=") + 
			stringFromFloat(floatColor.x) + string(",") +
			stringFromFloat(floatColor.y) + string(",") +
			stringFromFloat(floatColor.z) + string(",") +
			stringFromFloat(floatColor.w);
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlend", colorParamString.c_str());
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
}

void MeshRenderer::renderTravelingOrbsByTeam(Team* team, vector<TeamedOrbs> &travelingOrbs)
{
	int numSpritesToRender = 0;
	for (int travelingOrbsIndex = 0; travelingOrbsIndex != travelingOrbs.size(); travelingOrbsIndex++)
	{
		TeamedOrbs& teamedOrbs = travelingOrbs[travelingOrbsIndex];
		if (teamedOrbs.team->getID() == team->getID())
		{
			vector<Orb*> &orbsVec = teamedOrbs.orbs;
			for (int orbIndex = 0; orbIndex != orbsVec.size(); orbIndex++)
			{
				Orb* orb = orbsVec[orbIndex];
				//at least one of the two endpoint spheres is visible
				if (orb->hasPassedVisibilityTest() && orb->isVisibleForPlayer())
				{
					orbsRenderedCount++;
					numSpritesToRender++;
				}
				else
					orbsNotRenderedCount++;
			}
			break;	
		}
	}

	if (numSpritesToRender > 0)
	{
		int textureWidth, textureHeight, textureDepth;
		VTextureObject* orbBulletTexture = Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
		orbBulletTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
		hkvVec4 textureRange = hkvVec4(0, 0, textureSize.x, textureSize.y);
		VSmartPtr<VisMeshBuffer_cl> orbVertexBuffer = new VisMeshBuffer_cl();
		this->allocateTravelingOrbsVerticesForTeam(orbVertexBuffer, team, travelingOrbs, numSpritesToRender);
		VSmartPtr<VisMeshBuffer_cl> orbIndexBuffer = new VisMeshBuffer_cl();
		this->allocateQuadIndices(orbIndexBuffer, 6 * numSpritesToRender);

		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(orbVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(orbIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(orbBulletTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		hkvVec4 floatColor;
		VColorRef::RGBA_To_Float(team->getColor(), floatColor);
		string colorParamString = string("Color=") + 
			stringFromFloat(floatColor.x) + string(",") +
			stringFromFloat(floatColor.y) + string(",") +
			stringFromFloat(floatColor.z) + string(",") +
			stringFromFloat(floatColor.w);
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlend", colorParamString.c_str());
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
}

void MeshRenderer::renderBullets()
{
	int numSpritesToRender = 0;
	for (int bulletsVecIndex = 0; bulletsVecIndex != bullets.size(); bulletsVecIndex++)
	{
		vector<Bullet*>& bulletsVec = *bullets[bulletsVecIndex];
		for (int bulletIndex = 0; bulletIndex != bulletsVec.size(); bulletIndex++)
		{
			Bullet* bullet = bulletsVec[bulletIndex];
			NodeSphere* bulletParentNodeSphere = bullet->getTrackedObject()->getParentNodeSphere();
			if (bullet->hasPassedVisibilityTest() && bulletParentNodeSphere->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
				numSpritesToRender++;
		}
	}

	if (numSpritesToRender > 0)
	{
		int textureWidth, textureHeight, textureDepth;
		VTextureObject* bulletTexture = Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
		bulletTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
		hkvVec4 textureRange = hkvVec4(0, 0, textureSize.x, textureSize.y);
		VSmartPtr<VisMeshBuffer_cl> bulletVertexBuffer = new VisMeshBuffer_cl();
		this->allocateBulletsVertices(bulletVertexBuffer, bullets, numSpritesToRender, textureSize, textureRange);
		VSmartPtr<VisMeshBuffer_cl> bulletIndexBuffer = new VisMeshBuffer_cl();
		this->allocateQuadIndices(bulletIndexBuffer, 6 * numSpritesToRender);

		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(bulletVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(bulletIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(bulletTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlend", "Color=1,1,1,1");
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
}

//void MeshRenderer::renderBombs()
//{
//	int numSpritesToRender = 0;
//	for (int bombsVecIndex = 0; bombsVecIndex != bombs.size(); bombsVecIndex++)
//	{
//		vector<Bomb*>& bombsVec = *bombs[bombsVecIndex];
//		for (int bombIndex = 0; bombIndex != bombsVec.size(); bombIndex++)
//		{
//			if (bombsVec[bombIndex]->hasPassedVisibilityTest() && bombsVec[bombIndex]->getTargetSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
//				numSpritesToRender++;
//		}
//	}
//	if (numSpritesToRender > 0)
//	{
//		int textureWidth, textureHeight, textureDepth;
//		VTextureObject* bombTexture = Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
//		bombTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
//		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
//		hkvVec4 textureRange = hkvVec4(0, 0, textureSize.x, textureSize.y);
//		VSmartPtr<VisMeshBuffer_cl> bombVertexBuffer = new VisMeshBuffer_cl();
//		this->allocateBombsVertices(bombVertexBuffer, bombs, numSpritesToRender, textureSize, textureRange);
//		VSmartPtr<VisMeshBuffer_cl> bombIndexBuffer = new VisMeshBuffer_cl();
//		this->allocateQuadIndices(bombIndexBuffer, 6 * numSpritesToRender);
//
//		Vision::RenderLoopHelper.BeginMeshRendering();
//		Vision::RenderLoopHelper.AddMeshStreams(bombVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
//		Vision::RenderLoopHelper.AddMeshStreams(bombIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
//		Vision::RenderLoopHelper.BindMeshTexture(bombTexture,0);
//
//		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
//		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlend", "Color=0,1,0,1");
//		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
//		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
//		Vision::RenderLoopHelper.EndMeshRendering();
//	}
//}

void MeshRenderer::renderBridges(const vector<Bridge*> &bridges)
{
	int numSpritesToRender = 0;
	for (int bridgeIndex = 0; bridgeIndex != bridges.size(); bridgeIndex++)
	{
		if (bridges[bridgeIndex]->hasPassedVisibilityTest() && bridges[bridgeIndex]->isRevealed())
			numSpritesToRender++;
	}
	
	bridgesRenderedCount = 0;
	bridgesNotRenderedCount = 0;

	if (numSpritesToRender > 0)
	{
		VTextureObject* bridgeTexture = Vision::TextureManager.Load2DTexture("Textures\\bridge_texture.dds");
		int bridgeTextureSizeX, bridgeTextureSizeY, bridgeTextureDepth;
		bridgeTexture->GetTextureDimensions(bridgeTextureSizeX, bridgeTextureSizeY, bridgeTextureDepth);
		hkvVec2 bridgeTextureSize = hkvVec2((float) bridgeTextureSizeX, (float) bridgeTextureSizeY);
		hkvVec4 textureRange = hkvVec4(0, 0, bridgeTextureSize.x, bridgeTextureSize.y);
		
		int numCylindersPerBridge = 4;
		int numVerticalSlicesPerCylinder = 12;
		float distanceBetweenTwoHorizontalSlices = 350.0f;

		//calculate the number of vertices, indices and triangles to allocate in their respective buffers
		int numVertices = 0;
		int numIndices = 0;
		int numPrimitives = 0;
		vector<int> numHorizontalSlices;
		numHorizontalSlices.reserve(numSpritesToRender);
		for (int bridgeIndex = 0; bridgeIndex != bridges.size(); bridgeIndex++)
		{
			Bridge* bridge = bridges[bridgeIndex];
			if (!bridge->hasPassedVisibilityTest() || !bridge->isRevealed())
			{
				bridgesNotRenderedCount++;
				continue;
			}
			bridgesRenderedCount++;
			int numHorizontalSlicesPerCylinder = GeometryUtils::round(bridge->getLength() / distanceBetweenTwoHorizontalSlices) - 1;
			if (numHorizontalSlicesPerCylinder < 0)
				numHorizontalSlicesPerCylinder = 0;
			numHorizontalSlices.push_back(numHorizontalSlicesPerCylinder);
			numVertices += numCylindersPerBridge * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 2);
			numIndices += 6 * numCylindersPerBridge * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 1);
			numPrimitives += 2 * numCylindersPerBridge * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 1);
		}

		VSmartPtr<VisMeshBuffer_cl> bridgeVertexBuffer = new VisMeshBuffer_cl();
		this->allocateBridgesVertices(bridgeVertexBuffer, bridges, numVertices, numSpritesToRender, numCylindersPerBridge, numVerticalSlicesPerCylinder, distanceBetweenTwoHorizontalSlices, bridgeTextureSize);
		VSmartPtr<VisMeshBuffer_cl> bridgeIndexBuffer = new VisMeshBuffer_cl();
		this->allocateBridgesIndices(bridgeIndexBuffer, numCylindersPerBridge, numVerticalSlicesPerCylinder, numHorizontalSlices);	

		
		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(bridgeVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_NORMAL | VERTEX_STREAM_COLOR | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(bridgeIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(bridgeTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		//VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("VolumEffect", NULL);
		//VTechniqueConfig config;
		//config.SetInclusionTags("FULLBRIGHT;MIRROR;VERTEXCOLOR;OVERRIDE_VERTEX_COLOR");
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("VolumEffectTextured", "EdgeFadeFalloff=4;OverrideVertexColorValue=1,1,1,1;ScrollSpeed=0.1,0.1;", NULL);
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numPrimitives,numVertices);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
	else
		bridgesNotRenderedCount = bridges.size() - bridgesRenderedCount;
}

//void MeshRenderer::renderBridges(const vector<Bridge*> &bridges)
//{ 
//	int numSpritesToRender = (int) bridges.size();
//	if (numSpritesToRender > 0)
//	{
//		int textureWidth, textureHeight, textureDepth;
//		VTextureObject* bridgeTexture = Vision::TextureManager.Load2DTexture("Textures\\path_texture.png");
//		bridgeTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
//		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
//
//		VSmartPtr<VisMeshBuffer_cl> bridgeVertexBuffer = new VisMeshBuffer_cl();
//		this->allocateBridgesVertices(bridgeVertexBuffer, bridges, numSpritesToRender, textureSize);
//		VSmartPtr<VisMeshBuffer_cl> bridgeIndexBuffer = new VisMeshBuffer_cl();
//		this->allocateQuadIndices(bridgeIndexBuffer, 6 * numSpritesToRender);
//
//		Vision::RenderLoopHelper.BeginMeshRendering();
//		Vision::RenderLoopHelper.AddMeshStreams(bridgeVertexBuffer, VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
//		Vision::RenderLoopHelper.AddMeshStreams(bridgeIndexBuffer, VERTEX_STREAM_INDEXBUFFER);
//		Vision::RenderLoopHelper.BindMeshTexture(bridgeTexture, 0);
//
//		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
//		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("TintColorAlphaBlendDepthWrite", "Color=1,1,1,1");
//		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
//		Vision::RenderLoopHelper.RenderMeshes(pShaderPass, VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST, 0, 2 * numSpritesToRender, 4 * numSpritesToRender);
//		Vision::RenderLoopHelper.EndMeshRendering();
//	}
//}

void MeshRenderer::renderPotentialBridges(vector<Bridge*> potentialBridges)
{
	int numSpritesToRender = (int) potentialBridges.size();
	if (numSpritesToRender > 0)
	{
		int textureWidth, textureHeight, textureDepth;
		VTextureObject* bridgeTexture = Vision::TextureManager.Load2DTexture("Textures\\potential_bridge_texture.png");
		bridgeTexture->GetTextureDimensions(textureWidth, textureHeight, textureDepth);
		hkvVec2 textureSize = hkvVec2((float) textureWidth, (float) textureHeight);
		VSmartPtr<VisMeshBuffer_cl> bridgeVertexBuffer = new VisMeshBuffer_cl();
		this->allocatePotentialBridgesVertices(bridgeVertexBuffer, potentialBridges, numSpritesToRender, textureSize);
		VSmartPtr<VisMeshBuffer_cl> bridgeIndexBuffer = new VisMeshBuffer_cl();
		this->allocateQuadIndices(bridgeIndexBuffer, 6 * numSpritesToRender);
		
		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(bridgeVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(bridgeIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(bridgeTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		hkvVec4 floatColor;
		VColorRef::RGBA_To_Float(BUILD_MODE_COLOR, floatColor);
		string colorParamString = string("Color=") + 
			stringFromFloat(floatColor.x) + string(",") +
			stringFromFloat(floatColor.y) + string(",") +
			stringFromFloat(floatColor.z) + string(",") +
			stringFromFloat(floatColor.w);
		string UVSpeedParamString = "UVSpeed=-2,0";
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("UVScroll", (colorParamString + string(";") + UVSpeedParamString).c_str());
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);
		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numSpritesToRender*2,numSpritesToRender*4);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
}


void MeshRenderer::renderMetaOrbLinks()
{
	linksRenderedCount = 0;
	linksNotRenderedCount = 0;

	int numSpritesToRender = 0;
	int numMetaOrbLinks = 0;
	for (int metaOrbLinksVecIndex = 0; metaOrbLinksVecIndex != metaOrbLinks.size(); metaOrbLinksVecIndex++)
	{
		vector<MetaOrbLink*>& metaOrbLinksVec = *metaOrbLinks[metaOrbLinksVecIndex];
		for (int metaOrbLinkIndex = 0; metaOrbLinkIndex != metaOrbLinksVec.size(); metaOrbLinkIndex++)
		{
			MetaOrbLink* metaOrbLink = metaOrbLinksVec[metaOrbLinkIndex];
			numMetaOrbLinks++;
			if (/*metaOrbLink->hasPassedVisibilityTest() && */metaOrbLink->getLinkedNodeSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE)
				numSpritesToRender++;
		}
	}

	if (numSpritesToRender > 0)
	{
		VTextureObject* metaOrbLinkTexture = Vision::TextureManager.Load2DTexture("Textures\\bridge_texture.dds");
		int metaOrbLinkTextureX, metaOrbLinkTextureY, metaOrbLinkTextureDepth;
		metaOrbLinkTexture->GetTextureDimensions(metaOrbLinkTextureX, metaOrbLinkTextureY, metaOrbLinkTextureDepth);
		hkvVec2 metaOrbLinkTextureSize = hkvVec2((float) metaOrbLinkTextureX, (float) metaOrbLinkTextureY);
		hkvVec4 textureRange = hkvVec4(0, 0, metaOrbLinkTextureSize.x, metaOrbLinkTextureSize.y);
		
		int numCylindersPerMetaOrbLink = 9;
		int numVerticalSlicesPerCylinder = 9;
		float distanceBetweenTwoHorizontalSlices = 300.0f;

		//calculate the number of vertices, indices and triangles to allocate in their respective buffers
		int numVertices = 0;
		int numIndices = 0;
		int numPrimitives = 0;
		vector<int> numHorizontalSlices;
		numHorizontalSlices.reserve(numSpritesToRender);
		for (int metaOrbLinksVecIndex = 0; metaOrbLinksVecIndex != metaOrbLinks.size(); metaOrbLinksVecIndex++)
		{
			vector<MetaOrbLink*>& metaOrbLinksVec = *metaOrbLinks[metaOrbLinksVecIndex];
			for (int metaOrbLinkIndex = 0; metaOrbLinkIndex != metaOrbLinksVec.size(); metaOrbLinkIndex++)
			{
				MetaOrbLink* metaOrbLink = metaOrbLinksVec[metaOrbLinkIndex];
				if (!(/*metaOrbLink->hasPassedVisibilityTest() && */metaOrbLink->getLinkedNodeSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE))
				{
					linksNotRenderedCount++;
					continue;
				}
				linksRenderedCount++;
				int numHorizontalSlicesPerCylinder = GeometryUtils::round(metaOrbLink->GetSize().x / distanceBetweenTwoHorizontalSlices) - 1;
				numHorizontalSlices.push_back(numHorizontalSlicesPerCylinder);
				numVertices += numCylindersPerMetaOrbLink * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 2);
				numIndices += 6 * numCylindersPerMetaOrbLink * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 1);
				numPrimitives += 2 * numCylindersPerMetaOrbLink * numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 1);
			}
		}

		VSmartPtr<VisMeshBuffer_cl> metaOrbLinkVertexBuffer = new VisMeshBuffer_cl();
		this->allocateMetaOrbLinksVertices(metaOrbLinkVertexBuffer, metaOrbLinks, numVertices, numSpritesToRender, numCylindersPerMetaOrbLink, numVerticalSlicesPerCylinder, distanceBetweenTwoHorizontalSlices, metaOrbLinkTextureSize);
		VSmartPtr<VisMeshBuffer_cl> metaOrbLinkIndexBuffer = new VisMeshBuffer_cl();
		this->allocateBridgesIndices(metaOrbLinkIndexBuffer, numCylindersPerMetaOrbLink, numVerticalSlicesPerCylinder, numHorizontalSlices);	

		Vision::RenderLoopHelper.BeginMeshRendering();
		Vision::RenderLoopHelper.AddMeshStreams(metaOrbLinkVertexBuffer,VERTEX_STREAM_POSITION | VERTEX_STREAM_NORMAL | VERTEX_STREAM_COLOR | VERTEX_STREAM_TEX0);
		Vision::RenderLoopHelper.AddMeshStreams(metaOrbLinkIndexBuffer,VERTEX_STREAM_INDEXBUFFER);
		Vision::RenderLoopHelper.BindMeshTexture(metaOrbLinkTexture,0);

		Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
		//VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("VolumEffect", NULL);
		//VTechniqueConfig config;
		//config.SetInclusionTags("FULLBRIGHT;OVERRIDE_VERTEX_COLOR");	
		VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("VolumEffectTextured", "EdgeFadeFalloff=4;OverrideVertexColorValue=1,1,1,1;ScrollSpeed=0.1,0.1;", NULL);
		VCompiledShaderPass *pShaderPass = pTechnique->GetShader(0);

		Vision::RenderLoopHelper.RenderMeshes(pShaderPass,VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,0,numPrimitives,numVertices);
		Vision::RenderLoopHelper.EndMeshRendering();
	}
	else
		linksNotRenderedCount = numMetaOrbLinks - linksRenderedCount;
}

void MeshRenderer::obtainMetaOrbsLinks(vector<vector<MetaOrbLink*>* > &metaOrbLinks)
{
	vector<NodeSphere*> allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	int allNodeSpheresSize = allNodeSpheres.size();
	for (int nodeSphereIndex = 0; nodeSphereIndex != allNodeSpheresSize; nodeSphereIndex++)
	{
		NodeSphere* nodeSphere = allNodeSpheres[nodeSphereIndex];
		metaOrbLinks.push_back(&nodeSphere->getMetaOrbLinks());
	}
}

void MeshRenderer::obtainOrbsAndBullets(Team* team, vector<vector<Orb*>* > &orbs, vector<vector<Bullet*>* > &bullets)
{
	vector<NodeSphere*> allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	//populate the vector
	int allNodeSpheresSize = allNodeSpheres.size();
	for (int nodeSphereIndex = 0; nodeSphereIndex != allNodeSpheresSize; nodeSphereIndex++)
	{
		NodeSphere* nodeSphere = allNodeSpheres[nodeSphereIndex];

		vector<Orb*> &teamOrbs = nodeSphere->getOrbsForTeam(team);
		int teamOrbsSize = teamOrbs.size();
		//bullets tracking orbs
		for (int teamOrbIndex = 0; teamOrbIndex != teamOrbsSize; teamOrbIndex++)
		{
			vector<Bullet*> &trackingBullets = teamOrbs[teamOrbIndex]->getTrackingBullets();
			if (trackingBullets.size() > 0)
				bullets.push_back(&trackingBullets);
		}
		if (teamOrbsSize > 0)
			orbs.push_back(&teamOrbs);

		//bullets tracking meta-orbs
		MetaOrb* generationMetaOrb = nodeSphere->getGenerationMetaOrb();
		MetaOrb* defenseMetaOrb = nodeSphere->getDefenseMetaOrb();
		MetaOrb* assaultMetaOrb = nodeSphere->getAssaultMetaOrb();
		if (generationMetaOrb)
		{
			vector<Bullet*> &generationMetaOrbTrackingBullets = generationMetaOrb->getTrackingBullets();
			if (generationMetaOrbTrackingBullets.size() > 0)
				bullets.push_back(&generationMetaOrbTrackingBullets);
		}
		if (defenseMetaOrb)
		{
			vector<Bullet*> &defenseMetaOrbTrackingBullets = defenseMetaOrb->getTrackingBullets();
			if (defenseMetaOrbTrackingBullets.size() > 0)
				bullets.push_back(&defenseMetaOrbTrackingBullets);
		}
		if (assaultMetaOrb)
		{
			vector<Bullet*> &assaultMetaOrbTrackingBullets = assaultMetaOrb->getTrackingBullets();
			if (assaultMetaOrbTrackingBullets.size() > 0)
				bullets.push_back(&assaultMetaOrbTrackingBullets);
		}
	}
}

//void MeshRenderer::obtainBombs(vector<vector<Bomb*>* > &bombs)
//{
//	vector<NodeSphere*> allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
//	//populate the vector
//	int allNodeSpheresSize = allNodeSpheres.size();
//	for (int nodeSphereIndex = 0; nodeSphereIndex != allNodeSpheresSize; nodeSphereIndex++)
//	{
//		vector<Bomb*> &trackingBombs = allNodeSpheres[nodeSphereIndex]->getTrackingBombs();
//		if (trackingBombs.size() > 0)
//			bombs.push_back(&trackingBombs);
//	}
//}

void MeshRenderer::allocatePotentialBridgesVertices(VisMeshBuffer_cl* vertexBuffer, const vector<Bridge*> &potentialBridges, int potentialBridgesCount, hkvVec2 textureSize)
{
	vertexBuffer->AllocateVertices(m_descriptor, 4 * potentialBridgesCount, VIS_MEMUSAGE_STATIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	for (int bridgeIndex = 0; bridgeIndex != potentialBridgesCount; bridgeIndex++)
	{
		Bridge* bridge = potentialBridges[bridgeIndex];
		hkvVec2 size = bridge->GetSize();
		hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
		hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
		hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
		hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
		//transform vertices using the sprite transformation matrix
		hkvMat4 transformationMatrix = bridge->getTransformationMatrix();
		vertex0 = transformationMatrix.transformPosition(vertex0);
		vertex1 = transformationMatrix.transformPosition(vertex1);
		vertex2 = transformationMatrix.transformPosition(vertex2);
		vertex3 = transformationMatrix.transformPosition(vertex3);
		pVertex[4*bridgeIndex].vertexCoords = vertex0;
		pVertex[4*bridgeIndex+1].vertexCoords = vertex1;
		pVertex[4*bridgeIndex+2].vertexCoords = vertex2;
		pVertex[4*bridgeIndex+3].vertexCoords = vertex3;

		hkvVec4 textureRange = hkvVec4(0, 0, size.x, textureSize.y);

		float uLower = textureRange.x;
		float vLower = textureRange.y;
		float uUpper = textureRange.z;
		float vUpper = textureRange.w;

		pVertex[4*bridgeIndex].texCoords = hkvVec2(uLower / textureSize.x, vUpper / textureSize.y);
		pVertex[4*bridgeIndex+1].texCoords = hkvVec2(uLower / textureSize.x, vLower / textureSize.y);
		pVertex[4*bridgeIndex+2].texCoords = hkvVec2(uUpper / textureSize.x, vUpper / textureSize.y);
		pVertex[4*bridgeIndex+3].texCoords = hkvVec2(uUpper / textureSize.x, vLower / textureSize.y);
	}
	vertexBuffer->UnLockVertices();
}

void MeshRenderer::allocateBridgesVertices(VisMeshBuffer_cl* vertexBuffer, const vector<Bridge*> &allBridges, int numVertices, int bridgesCount, int numCylindersPerBridge, int numVerticalSlicesPerCylinder, float distanceBetweenTwoHorizontalSlices, hkvVec2 textureSize)
{
	//hkvLog::Error("+++++allocateBridgeVertices");
	vertexBuffer->AllocateVertices(m_descriptor, numVertices, VIS_MEMUSAGE_STATIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int cylinderGlobalIndex = 0;
	
	int bufferVertexIndex = 0; //global index to fill the vertex buffer

	for (int bridgeIndex = 0; bridgeIndex != allBridges.size(); bridgeIndex++)
	{
		Bridge* bridge = allBridges[bridgeIndex];
		if (!bridge->hasPassedVisibilityTest() || !bridge->isRevealed())
			continue;

		float bridgeRadius = 0.5f * bridge->GetSize().y;
		int radiusAmplitude = numCylindersPerBridge;
		float minBridgeRadius =	bridgeRadius - radiusAmplitude; //radius of the smallest cylinder
		float maxBridgeRadius = bridgeRadius + radiusAmplitude; //radius of the largest cylinder

		float bridgeLength = bridge->getLength();
		int numHorizontalSlicesPerCylinder = GeometryUtils::round(bridgeLength / distanceBetweenTwoHorizontalSlices) - 1;
		if (numHorizontalSlicesPerCylinder < 0)
			numHorizontalSlicesPerCylinder = 0;
		int numVerticesPerCylinder = numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 2);
		float sliceAngle = (float) (2 * M_PI / (float) numVerticalSlicesPerCylinder); //angle between 2 slices

		for (int cylinderIndex = 0; cylinderIndex != numCylindersPerBridge; cylinderIndex++)
		{
			float cylinderRadius = (numCylindersPerBridge > 1) ? minBridgeRadius + cylinderIndex * (maxBridgeRadius - minBridgeRadius) / (float) (numCylindersPerBridge - 1)
																: minBridgeRadius;

			for (int verticalSliceIndex = 0; verticalSliceIndex != numVerticalSlicesPerCylinder; verticalSliceIndex++)
			{
				float sliceYCoords = cylinderRadius * cos(verticalSliceIndex * sliceAngle);
				float sliceZCoords = cylinderRadius * sin(verticalSliceIndex * sliceAngle);

				//vertex positions along the vertical slice
				vector<hkvVec3> vertexPositions;
				vertexPositions.reserve(numHorizontalSlicesPerCylinder + 2);
				hkvMat4 transformationMatrix = bridge->getTransformationMatrix();
				hkvMat3 rotationMatrix = bridge->GetRotationMatrix();

				hkvVec3 startPoint;
				startPoint.x = 0.5f * bridgeLength;
				startPoint.y = sliceYCoords;
				startPoint.z = sliceZCoords;
				vertexPositions.push_back(startPoint);

				float horizontalLengthStep = bridgeLength / (float) (numHorizontalSlicesPerCylinder + 1);
				for (int horizontalSliceIndex = 0; horizontalSliceIndex != numHorizontalSlicesPerCylinder; horizontalSliceIndex++)
				{
					hkvVec3 vertexPosition;
					vertexPosition.x = 0.5f * bridgeLength - (horizontalSliceIndex + 1) * horizontalLengthStep;
					vertexPosition.y = sliceYCoords;
					vertexPosition.z = sliceZCoords;
					vertexPositions.push_back(vertexPosition);
				}

				hkvVec3 endPoint;
				endPoint.x = -0.5f * bridgeLength;
				endPoint.y = sliceYCoords;
				endPoint.z = sliceZCoords;
				vertexPositions.push_back(endPoint);

				Team* startSphereTeam = bridge->getStartSphere()->getTeam();
				Team* endSphereTeam = bridge->getEndSphere()->getTeam();
				hkvVec4 startSphereColor = startSphereTeam ? startSphereTeam->getColor().getAsVec4() : TEAM_NEUTRAL_COLOR.getAsVec4();
				hkvVec4 endSphereColor = endSphereTeam ? endSphereTeam->getColor().getAsVec4() : V_RGBA_WHITE.getAsVec4();

				for (int vertexPositionIndex = 0; vertexPositionIndex != vertexPositions.size(); vertexPositionIndex++)
				{
					hkvVec3 vertexPosition = vertexPositions[vertexPositionIndex];
					hkvVec3 vertexNormal = hkvVec3(0, vertexPosition.y, vertexPosition.z);
					vertexPosition = transformationMatrix.transformPosition(vertexPosition); //MtoW transform
					vertexNormal = rotationMatrix.transformDirection(vertexNormal);
					vertexNormal.normalize();
					pVertex[bufferVertexIndex].vertexCoords = vertexPosition;
					float yPositionRatio = vertexPositionIndex / (float) (numHorizontalSlicesPerCylinder + 1);
					hkvVec4 colorVec4 = yPositionRatio * (startSphereColor - endSphereColor) + endSphereColor;
					pVertex[bufferVertexIndex].color = VColorRef::Float_To_RGBA(colorVec4);
					float texCoordsX = (float) (2 * M_PI * cylinderRadius) * (verticalSliceIndex / (float) numVerticalSlicesPerCylinder) / textureSize.x;
					float texCoordsY = (yPositionRatio * bridgeLength) / textureSize.y;
					pVertex[bufferVertexIndex].texCoords = hkvVec2(texCoordsX, texCoordsY);
					pVertex[bufferVertexIndex].normal = vertexNormal;

					bufferVertexIndex++;
				}
			}
			cylinderGlobalIndex++;
		}
	}
	vertexBuffer->UnLockVertices();
}

void MeshRenderer::allocateMetaOrbLinksVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<MetaOrbLink*>* > &metaOrbLinks, int numVertices, int metaOrbLinksCount, int numCylindersPerMetaOrbLink, int numVerticalSlicesPerCylinder, float distanceBetweenTwoHorizontalSlices, hkvVec2 textureSize)
{
	vertexBuffer->AllocateVertices(m_descriptor, numVertices, VIS_MEMUSAGE_STATIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int cylinderGlobalIndex = 0;
	
	int bufferVertexIndex = 0; //global index to fill the vertex buffer

	for (int metaOrbLinksVecIndex = 0; metaOrbLinksVecIndex != metaOrbLinks.size(); metaOrbLinksVecIndex++)
	{
		vector<MetaOrbLink*>& metaOrbLinksVec = *metaOrbLinks[metaOrbLinksVecIndex];
		for (int metaOrbLinkIndex = 0; metaOrbLinkIndex != metaOrbLinksVec.size(); metaOrbLinkIndex++)
		{
			MetaOrbLink* metaOrbLink = metaOrbLinksVec[metaOrbLinkIndex];
			if (!(/*metaOrbLink->hasPassedVisibilityTest() && */metaOrbLink->getLinkedNodeSphere()->getVisibilityStatus() == VISIBILITY_STATUS_VISIBLE))
				continue;

			float metaOrbLinkRadius = 0.5f * metaOrbLink->GetSize().y;
			int radiusAmplitude = numCylindersPerMetaOrbLink / 2;
			float minMetaOrbLinkRadius = metaOrbLinkRadius - radiusAmplitude; //radius of the smallest cylinder
			float maxMetaOrbLinkRadius = metaOrbLinkRadius + radiusAmplitude; //radius of the largest cylinder

			float metaOrbLinkLength = metaOrbLink->GetSize().x;
			int numHorizontalSlicesPerCylinder = GeometryUtils::round(metaOrbLinkLength / distanceBetweenTwoHorizontalSlices) - 1;
			int numVerticesPerCylinder = numVerticalSlicesPerCylinder * (numHorizontalSlicesPerCylinder + 2);
			float sliceAngle = (float) (2 * M_PI / (float) numVerticalSlicesPerCylinder); //angle between 2 slices

			VColorRef metaOrbLinkColor = metaOrbLink->getLinkedMetaOrb()->getColor();			

			for (int cylinderIndex = 0; cylinderIndex != numCylindersPerMetaOrbLink; cylinderIndex++)
			{
				float cylinderRadius = (numCylindersPerMetaOrbLink > 1) ? minMetaOrbLinkRadius + cylinderIndex * (maxMetaOrbLinkRadius - minMetaOrbLinkRadius) / (float) (numCylindersPerMetaOrbLink - 1)
																		: maxMetaOrbLinkRadius;

				for (int verticalSliceIndex = 0; verticalSliceIndex != numVerticalSlicesPerCylinder; verticalSliceIndex++)
				{
					float sliceYCoords = cylinderRadius * cos(verticalSliceIndex * sliceAngle);
					float sliceZCoords = cylinderRadius * sin(verticalSliceIndex * sliceAngle);

					//vertex positions along the vertical slice
					vector<hkvVec3> vertexPositions;
					vertexPositions.reserve(numHorizontalSlicesPerCylinder + 2);
					hkvMat4 transformationMatrix = metaOrbLink->getTransformationMatrix();
					hkvMat3 rotationMatrix = metaOrbLink->GetRotationMatrix();

					hkvVec3 startPoint;
					startPoint.x = 0.5f * metaOrbLinkLength;
					startPoint.y = sliceYCoords;
					startPoint.z = sliceZCoords;
					vertexPositions.push_back(startPoint);

					float horizontalLengthStep = metaOrbLinkLength / (float) (numHorizontalSlicesPerCylinder + 1);
					for (int horizontalSliceIndex = 0; horizontalSliceIndex != numHorizontalSlicesPerCylinder; horizontalSliceIndex++)
					{
						hkvVec3 vertexPosition;
						vertexPosition.x = 0.5f * metaOrbLinkLength - (horizontalSliceIndex + 1) * horizontalLengthStep;
						vertexPosition.y = sliceYCoords;
						vertexPosition.z = sliceZCoords;
						vertexPositions.push_back(vertexPosition);
					}

					hkvVec3 endPoint;
					endPoint.x = -0.5f * metaOrbLinkLength;
					endPoint.y = sliceYCoords;
					endPoint.z = sliceZCoords;
					vertexPositions.push_back(endPoint);

					for (int vertexPositionIndex = 0; vertexPositionIndex != vertexPositions.size(); vertexPositionIndex++)
					{
						hkvVec3 vertexPosition = vertexPositions[vertexPositionIndex];
						hkvVec3 vertexNormal = hkvVec3(0, vertexPosition.y, vertexPosition.z);
						vertexPosition = transformationMatrix.transformPosition(vertexPosition); //MtoW transform
						vertexNormal = rotationMatrix.transformDirection(vertexNormal);
						vertexNormal.normalize();
						pVertex[bufferVertexIndex].vertexCoords = vertexPosition;
						float yPositionRatio = vertexPositionIndex / (float) (numHorizontalSlicesPerCylinder + 1);
						float texCoordsX = (float) (2 * M_PI * cylinderRadius) * (verticalSliceIndex / (float) numVerticalSlicesPerCylinder) / textureSize.x;
						float texCoordsY = (yPositionRatio * metaOrbLinkLength) / textureSize.y;
						pVertex[bufferVertexIndex].texCoords = hkvVec2(texCoordsX, texCoordsY);
						pVertex[bufferVertexIndex].normal = vertexNormal;
						pVertex[bufferVertexIndex].color = metaOrbLinkColor;

						bufferVertexIndex++;
					}
				}
				cylinderGlobalIndex++;
			}
		}
	}
	vertexBuffer->UnLockVertices();
}

//void MeshRenderer::allocateMetaOrbLinksVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<MetaOrbLink*>* > &metaOrbLinks, int metaOrbLinksCount, int numSlicesPerLink)
//{
//	vertexBuffer->AllocateVertices(m_descriptor, 2 * numSlicesPerLink * metaOrbLinksCount, VIS_MEMUSAGE_DYNAMIC, false);
//
//	//fill the buffer
//	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));
//
//	int metaOrbLinkGlobalIndex = 0;
//	for (int metaOrbLinksVecIndex = 0; metaOrbLinksVecIndex != metaOrbLinks.size(); metaOrbLinksVecIndex++)
//	{
//		vector<MetaOrbLink*> metaOrbLinksVec = *metaOrbLinks[metaOrbLinksVecIndex];
//		for (int metaOrbLinkIndex = 0; metaOrbLinkIndex != metaOrbLinksVec.size(); metaOrbLinkIndex++)
//		{
//			MetaOrbLink* metaOrbLink = metaOrbLinksVec[metaOrbLinkIndex];
//
//			int numVertices = 2 * numSlicesPerLink;
//			float sliceAngle = (float) (2 * M_PI / (float) numSlicesPerLink); //angle between 2 slices
//
//			float linkLength = metaOrbLink->GetSize().x;
//			float linkRadius = 0.5f * metaOrbLink->GetSize().y;
//
//			for (int sliceIndex = 0; sliceIndex != numSlicesPerLink; sliceIndex++)
//			{
//				float sliceYCoords = linkRadius * cos(sliceIndex * sliceAngle);
//				float sliceZCoords = linkRadius * sin(sliceIndex * sliceAngle);
//
//				//point1
//				hkvVec3 point1;
//				point1.x = 0.5f * linkLength;
//				point1.y = sliceYCoords;
//				point1.z = sliceZCoords;
//
//				//point2
//				hkvVec3 point2;
//				point2.x = -0.5f * linkLength;
//				point2.y = sliceYCoords;
//				point2.z = sliceZCoords;
//
//				hkvMat4 transformationMatrix = metaOrbLink->getTransformationMatrix();
//				point1 = transformationMatrix.transformPosition(point1);
//				point2 = transformationMatrix.transformPosition(point2);
//				
//
//				pVertex[numVertices * metaOrbLinkGlobalIndex + 2 * sliceIndex].vertexCoords = point1;
//				pVertex[numVertices * metaOrbLinkGlobalIndex + 2 * sliceIndex + 1].vertexCoords = point2;
//
//				pVertex[numVertices * metaOrbLinkGlobalIndex + 2 * sliceIndex].texCoords = hkvVec2((float) sliceIndex, 1.0f);
//				pVertex[numVertices * metaOrbLinkGlobalIndex + 2 * sliceIndex + 1].texCoords = hkvVec2((float) sliceIndex, 0.0f);
//			}
//
//			metaOrbLinkGlobalIndex++;
//		}
//	}
//	vertexBuffer->UnLockVertices();
//}

void MeshRenderer::allocateHalosVerticesForTeam(Team* team, VisMeshBuffer_cl* vertexBuffer, const vector<NodeSphere*> &allNodeSpheres, int halosCount, hkvVec2 textureSize, hkvVec4 textureRange)
{
	vertexBuffer->AllocateVertices(m_descriptor, 4 * halosCount, VIS_MEMUSAGE_DYNAMIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int haloGlobalIndex = 0;
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		NodeSphere* nodeSphere = allNodeSpheres[sphereIndex];
		if (nodeSphere->getTeam() != team)
			continue;
		SphereGlowHalo* sphereHalo = nodeSphere->getHalo();

		if (sphereHalo->hasPassedVisibilityTest() && nodeSphere->getVisibilityStatus() != VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY)
		{
			buildVertexAndTexCoordsForHalo(sphereHalo, pVertex, haloGlobalIndex, textureSize, textureRange);
			haloGlobalIndex++;

			//build one halo for each of its meta orbs
			MetaOrb* generationMetaOrb = nodeSphere->getGenerationMetaOrb();
			MetaOrb* defenseMetaOrb = nodeSphere->getDefenseMetaOrb();
			MetaOrb* assaultMetaOrb = nodeSphere->getAssaultMetaOrb();

			if (generationMetaOrb && generationMetaOrb->GetVisibleBitmask() == 0xffffffff)
			{
				buildVertexAndTexCoordsForHalo(generationMetaOrb->getHalo(), pVertex, haloGlobalIndex, textureSize, textureRange);
				haloGlobalIndex++;
			}
			else if (defenseMetaOrb && defenseMetaOrb->GetVisibleBitmask() == 0xffffffff)
			{
				buildVertexAndTexCoordsForHalo(defenseMetaOrb->getHalo(), pVertex, haloGlobalIndex, textureSize, textureRange);
				haloGlobalIndex++;
			}
			else if (assaultMetaOrb && assaultMetaOrb->GetVisibleBitmask() == 0xffffffff)
			{
				buildVertexAndTexCoordsForHalo(assaultMetaOrb->getHalo(), pVertex, haloGlobalIndex, textureSize, textureRange);
				haloGlobalIndex++;
			}
		}
	}
	vertexBuffer->UnLockVertices();
}

void MeshRenderer::buildVertexAndTexCoordsForHalo(SphereGlowHalo* sphereHalo, MeshVertex* pVertex, int haloGlobalIndex, hkvVec2 textureSize, hkvVec4 textureRange)
{
	hkvVec2 size = sphereHalo->GetSize();
	hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
	hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
	hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
	hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
	//transform vertices using the sprite transformation matrix
	hkvMat4 transformationMatrix = sphereHalo->getTransformationMatrix();
	vertex0 = transformationMatrix.transformPosition(vertex0);
	vertex1 = transformationMatrix.transformPosition(vertex1);
	vertex2 = transformationMatrix.transformPosition(vertex2);
	vertex3 = transformationMatrix.transformPosition(vertex3);
	pVertex[4*haloGlobalIndex].vertexCoords = vertex0;
	pVertex[4*haloGlobalIndex+1].vertexCoords = vertex1;
	pVertex[4*haloGlobalIndex+2].vertexCoords = vertex2;
	pVertex[4*haloGlobalIndex+3].vertexCoords = vertex3;

	float uLower = textureRange.x;
	float vLower = textureRange.y;
	float uUpper = textureRange.z;
	float vUpper = textureRange.w;

	pVertex[4*haloGlobalIndex].texCoords = hkvVec2(uLower / textureSize.x, vUpper / textureSize.y);
	pVertex[4*haloGlobalIndex+1].texCoords = hkvVec2(uLower / textureSize.x, vLower / textureSize.y);
	pVertex[4*haloGlobalIndex+2].texCoords = hkvVec2(uUpper / textureSize.x, vUpper / textureSize.y);
	pVertex[4*haloGlobalIndex+3].texCoords = hkvVec2(uUpper / textureSize.x, vLower / textureSize.y);
}

void MeshRenderer::allocateOrbsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Orb*>* > &allOrbs, int orbsCount, hkvVec2 textureSize, hkvVec4 textureRange)
{
	vertexBuffer->AllocateVertices(m_descriptor, 4 * orbsCount, VIS_MEMUSAGE_DYNAMIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int orbGlobalIndex = 0;
	for (int orbsVecIndex = 0; orbsVecIndex != allOrbs.size(); orbsVecIndex++)
	{
		vector<Orb*> orbs = *allOrbs[orbsVecIndex];
		for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
		{
			Orb* orb = orbs[orbIndex];
			//hkvLog::Error("orb %s", orb->isVisible() ? "VISIBLE" : "HIDDEN");
			if (!orb->hasPassedVisibilityTest() || !orb->isVisibleForPlayer())
			{
				continue;
			}
			hkvVec2 size = orb->GetSize();
			hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
			hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
			//transform vertices using the sprite transformation matrix
			hkvMat4 transformationMatrix = orb->getTransformationMatrix();
			vertex0 = transformationMatrix.transformPosition(vertex0);
			vertex1 = transformationMatrix.transformPosition(vertex1);
			vertex2 = transformationMatrix.transformPosition(vertex2);
			vertex3 = transformationMatrix.transformPosition(vertex3);
			pVertex[4*orbGlobalIndex].vertexCoords = vertex0;
			pVertex[4*orbGlobalIndex+1].vertexCoords = vertex1;
			pVertex[4*orbGlobalIndex+2].vertexCoords = vertex2;
			pVertex[4*orbGlobalIndex+3].vertexCoords = vertex3;

			float uLower = textureRange.x;
			float vLower = textureRange.y;
			float uUpper = textureRange.z;
			float vUpper = textureRange.w;

			pVertex[4*orbGlobalIndex].texCoords = hkvVec2(uLower / textureSize.x, vUpper / textureSize.y);
			pVertex[4*orbGlobalIndex+1].texCoords = hkvVec2(uLower / textureSize.x, vLower / textureSize.y);
			pVertex[4*orbGlobalIndex+2].texCoords = hkvVec2(uUpper / textureSize.x, vUpper / textureSize.y);
			pVertex[4*orbGlobalIndex+3].texCoords = hkvVec2(uUpper / textureSize.x, vLower / textureSize.y);

			orbGlobalIndex++;
		}
	}

	vertexBuffer->UnLockVertices();
}

void MeshRenderer::allocateTravelingOrbsVerticesForTeam(VisMeshBuffer_cl* vertexBuffer, Team* team, vector<TeamedOrbs> &travelingOrbs, int travelingOrbsCount)
{
	vertexBuffer->AllocateVertices(m_descriptor, 4 * travelingOrbsCount, VIS_MEMUSAGE_DYNAMIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int orbGlobalIndex = 0;
	for (int orbsVecIndex = 0; orbsVecIndex != travelingOrbs.size(); orbsVecIndex++)
	{
		Team* orbsVecTeam = travelingOrbs[orbsVecIndex].team;
		if (orbsVecTeam->getID() != team->getID())
			continue;
		vector<Orb*> &orbs = travelingOrbs[orbsVecIndex].orbs;
		for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
		{
			Orb* orb = orbs[orbIndex];
			if (!orb->hasPassedVisibilityTest() || !orb->isVisibleForPlayer())
				continue;
			hkvVec2 size = orb->GetSize();
			hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
			hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
			//transform vertices using the sprite transformation matrix
			hkvMat4 transformationMatrix = orb->getTransformationMatrix();
			vertex0 = transformationMatrix.transformPosition(vertex0);
			vertex1 = transformationMatrix.transformPosition(vertex1);
			vertex2 = transformationMatrix.transformPosition(vertex2);
			vertex3 = transformationMatrix.transformPosition(vertex3);
			pVertex[4*orbGlobalIndex].vertexCoords = vertex0;
			pVertex[4*orbGlobalIndex+1].vertexCoords = vertex1;
			pVertex[4*orbGlobalIndex+2].vertexCoords = vertex2;
			pVertex[4*orbGlobalIndex+3].vertexCoords = vertex3;

			pVertex[4*orbGlobalIndex].texCoords = hkvVec2(0.0f, 1.0f);
			pVertex[4*orbGlobalIndex+1].texCoords = hkvVec2(0.0f, 0.0f);
			pVertex[4*orbGlobalIndex+2].texCoords = hkvVec2(1.0f, 1.0f);
			pVertex[4*orbGlobalIndex+3].texCoords = hkvVec2(1.0f, 0.0f);

			orbGlobalIndex++;
		}
	}

	vertexBuffer->UnLockVertices();
}

void MeshRenderer::allocateBulletsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Bullet*>* > &allBullets, int bulletsCount, hkvVec2 textureSize, hkvVec4 textureRange)
{
	vertexBuffer->AllocateVertices(m_descriptor, 4 * bulletsCount, VIS_MEMUSAGE_DYNAMIC, false);

	//fill the buffer
	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));

	int bulletGlobalIndex = 0;
	for (int bulletsVecIndex = 0; bulletsVecIndex != allBullets.size(); bulletsVecIndex++)
	{
		vector<Bullet*> bullets = *allBullets[bulletsVecIndex];
		for (int bulletIndex = 0; bulletIndex != bullets.size(); bulletIndex++)
		{
			Bullet* bullet = bullets[bulletIndex];
			NodeSphere* bulletParentNodeSphere = bullet->getTrackedObject()->getParentNodeSphere();

			if (!bullet->hasPassedVisibilityTest() || bulletParentNodeSphere->getVisibilityStatus() != VISIBILITY_STATUS_VISIBLE)
				continue;
			hkvVec2 size = bullet->GetSize();
			hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
			hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
			hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
			//transform vertices using the sprite transformation matrix
			hkvMat4 transformationMatrix = bullet->getTransformationMatrix();
			vertex0 = transformationMatrix.transformPosition(vertex0);
			vertex1 = transformationMatrix.transformPosition(vertex1);
			vertex2 = transformationMatrix.transformPosition(vertex2);
			vertex3 = transformationMatrix.transformPosition(vertex3);
			pVertex[4*bulletGlobalIndex].vertexCoords = vertex0;
			pVertex[4*bulletGlobalIndex+1].vertexCoords = vertex1;
			pVertex[4*bulletGlobalIndex+2].vertexCoords = vertex2;
			pVertex[4*bulletGlobalIndex+3].vertexCoords = vertex3;

			float uLower = textureRange.x;
			float vLower = textureRange.y;
			float uUpper = textureRange.z;
			float vUpper = textureRange.w;

			pVertex[4*bulletGlobalIndex].texCoords = hkvVec2(uLower / textureSize.x, vUpper / textureSize.y);
			pVertex[4*bulletGlobalIndex+1].texCoords = hkvVec2(uLower / textureSize.x, vLower / textureSize.y);
			pVertex[4*bulletGlobalIndex+2].texCoords = hkvVec2(uUpper / textureSize.x, vUpper / textureSize.y);
			pVertex[4*bulletGlobalIndex+3].texCoords = hkvVec2(uUpper / textureSize.x, vLower / textureSize.y);

			bulletGlobalIndex++;
		}
	}

	vertexBuffer->UnLockVertices();
}

//void MeshRenderer::allocateBombsVertices(VisMeshBuffer_cl* vertexBuffer, const vector<vector<Bomb*>* > &allBombs, int bombsCount, hkvVec2 textureSize, hkvVec4 textureRange)
//{
//	vertexBuffer->AllocateVertices(m_descriptor, 4 * bombsCount, VIS_MEMUSAGE_DYNAMIC, false);
//
//	//fill the buffer
//	MeshVertex* pVertex = static_cast<MeshVertex*>(vertexBuffer->LockVertices(VIS_LOCKFLAG_DISCARDABLE));
//
//	int bulletGlobalIndex = 0;
//	for (int bombsVecIndex = 0; bombsVecIndex != allBombs.size(); bombsVecIndex++)
//	{
//		vector<Bomb*> bombs = *allBombs[bombsVecIndex];
//		for (int bombIndex = 0; bombIndex != bombs.size(); bombIndex++)
//		{
//			Bomb* bomb = bombs[bombIndex];
//			if (!bomb->hasPassedVisibilityTest() || bomb->getTargetSphere()->getVisibilityStatus() != VISIBILITY_STATUS_VISIBLE)
//				continue;
//			hkvVec2 size = bomb->GetSize();
//			hkvVec3 vertex0 = hkvVec3(-0.5f * size.x, 0.0f, -0.5f * size.y);
//			hkvVec3 vertex1 = hkvVec3(-0.5f * size.x, 0.0f, 0.5f * size.y);
//			hkvVec3 vertex2 = hkvVec3(0.5f * size.x, 0.0f, -0.5f * size.y);
//			hkvVec3 vertex3 = hkvVec3(0.5f * size.x, 0.0f, 0.5f * size.y);
//			//transform vertices using the sprite transformation matrix
//			hkvMat4 transformationMatrix = bomb->getTransformationMatrix();
//			vertex0 = transformationMatrix.transformPosition(vertex0);
//			vertex1 = transformationMatrix.transformPosition(vertex1);
//			vertex2 = transformationMatrix.transformPosition(vertex2);
//			vertex3 = transformationMatrix.transformPosition(vertex3);
//			pVertex[4*bulletGlobalIndex].vertexCoords = vertex0;
//			pVertex[4*bulletGlobalIndex+1].vertexCoords = vertex1;
//			pVertex[4*bulletGlobalIndex+2].vertexCoords = vertex2;
//			pVertex[4*bulletGlobalIndex+3].vertexCoords = vertex3;
//
//			float uLower = textureRange.x;
//			float vLower = textureRange.y;
//			float uUpper = textureRange.z;
//			float vUpper = textureRange.w;
//
//			pVertex[4*bulletGlobalIndex].texCoords = hkvVec2(uLower / textureSize.x, vUpper / textureSize.y);
//			pVertex[4*bulletGlobalIndex+1].texCoords = hkvVec2(uLower / textureSize.x, vLower / textureSize.y);
//			pVertex[4*bulletGlobalIndex+2].texCoords = hkvVec2(uUpper / textureSize.x, vUpper / textureSize.y);
//			pVertex[4*bulletGlobalIndex+3].texCoords = hkvVec2(uUpper / textureSize.x, vLower / textureSize.y);
//
//			bulletGlobalIndex++;
//		}
//	}
//	vertexBuffer->UnLockVertices();
//}

void MeshRenderer::allocateBridgesIndices(VisMeshBuffer_cl* indexBuffer, int numCylindersPerBridge, int numVerticalSlicesPerCylinder, vector<int> &numHorizontalSlices)
{
	//hkvLog::Error("++++allocateBridgesIndices");
	int numIndices = 0;
	for (int horizontalSlicesIndex = 0; horizontalSlicesIndex != numHorizontalSlices.size(); horizontalSlicesIndex++)
	{
		numIndices += 6 * numCylindersPerBridge * numVerticalSlicesPerCylinder * (numHorizontalSlices[horizontalSlicesIndex] + 1);
	}

	indexBuffer->AllocateIndexList(numIndices, VIS_MEMUSAGE_STATIC);

	unsigned short *pIndex = static_cast<unsigned short*>(indexBuffer->LockIndices(VIS_LOCKFLAG_DISCARDABLE));
	

	int globalBaseIndex = 0;
	int bridgeStartIndex = 0;
	for (int horizontalSlicesIndex = 0; horizontalSlicesIndex != numHorizontalSlices.size(); horizontalSlicesIndex++) //1 loop = 1 bridge
	{
		int numHorizontalSlicesForBridge = numHorizontalSlices[horizontalSlicesIndex];
		int numVerticesForVerticalSlice = numHorizontalSlicesForBridge + 2;
		int numVerticesForCylinder = numVerticalSlicesPerCylinder * numVerticesForVerticalSlice;
		int numVerticesForBridge = numCylindersPerBridge * numVerticesForCylinder;
		//hkvLog::Error("HorizontalSlices:%i VerticalSlices:%i", numHorizontalSlicesForBridge, numVerticalSlicesPerCylinder);
		for (int cylinderIndex = 0; cylinderIndex != numCylindersPerBridge; cylinderIndex++) //1 loop = 1 cylinder
		{
			//hkvLog::Error("+++new cylinder");
			for (int verticalSliceIndex = 0; verticalSliceIndex != numVerticalSlicesPerCylinder; verticalSliceIndex++) //1 loop = 1 vertical slice
			{
				//hkvLog::Error("+++++++new vertical slice");
				for (int horizontalSliceIndex = 0; horizontalSliceIndex != numHorizontalSlicesForBridge + 1; horizontalSliceIndex++) //1 loop = 1 horizontal slice
				{
					int baseValue = bridgeStartIndex + cylinderIndex * numVerticesForCylinder + verticalSliceIndex * numVerticesForVerticalSlice + horizontalSliceIndex;
					//hkvLog::Error("++++NEW SET OF INDICES");

					if (verticalSliceIndex == numVerticalSlicesPerCylinder - 1)
					{
						int firstSliceBaseValue = bridgeStartIndex + cylinderIndex * numVerticesForCylinder + horizontalSliceIndex;
						pIndex[globalBaseIndex] = baseValue ;
						pIndex[globalBaseIndex + 1] = baseValue + 1;
						pIndex[globalBaseIndex + 2] = firstSliceBaseValue;
						pIndex[globalBaseIndex + 3] = firstSliceBaseValue;
						pIndex[globalBaseIndex + 4] = baseValue + 1;
						pIndex[globalBaseIndex + 5] = firstSliceBaseValue + 1;
					}
					else
					{
						pIndex[globalBaseIndex] = baseValue ;
						pIndex[globalBaseIndex + 1] = baseValue + 1;
						pIndex[globalBaseIndex + 2] = baseValue + numHorizontalSlicesForBridge + 2;
						pIndex[globalBaseIndex + 3] = baseValue + numHorizontalSlicesForBridge + 2;
						pIndex[globalBaseIndex + 4] = baseValue + 1;
						pIndex[globalBaseIndex + 5] = baseValue + numHorizontalSlicesForBridge + 3;

					}
					//hkvLog::Error("index:%i value:%i", globalBaseIndex, pIndex[globalBaseIndex]);
					//hkvLog::Error("index:%i value:%i", globalBaseIndex+1, pIndex[globalBaseIndex+1]);
					//hkvLog::Error("index:%i value:%i", globalBaseIndex+2, pIndex[globalBaseIndex+2]);
					//hkvLog::Error("index:%i value:%i", globalBaseIndex+3, pIndex[globalBaseIndex+3]);
					//hkvLog::Error("index:%i value:%i", globalBaseIndex+4, pIndex[globalBaseIndex+4]);
					//hkvLog::Error("index:%i value:%i", globalBaseIndex+5, pIndex[globalBaseIndex+5]);
					globalBaseIndex += 6;
				}
			}
		}
		bridgeStartIndex += numVerticesForBridge;
	}

	indexBuffer->UnLockIndices();
}

void MeshRenderer::allocateQuadIndices(VisMeshBuffer_cl* indexBuffer, int numIndices)
{
	indexBuffer->AllocateIndexList(numIndices, VIS_MEMUSAGE_STATIC);
	unsigned short *pIndex = static_cast<unsigned short*>(indexBuffer->LockIndices(VIS_LOCKFLAG_DISCARDABLE));
	
	int indexStart = 0;
	for (int i = 0; i != numIndices; i+=6)
	{
		pIndex[i] = 4*indexStart;
		pIndex[i+1] = 4*indexStart + 2;
		pIndex[i+2] = 4*indexStart + 1;
		pIndex[i+3] = 4*indexStart + 2;
		pIndex[i+4] = 4*indexStart + 3;
		pIndex[i+5] = 4*indexStart + 1;

		indexStart++;
	}
	indexBuffer->UnLockIndices();
}