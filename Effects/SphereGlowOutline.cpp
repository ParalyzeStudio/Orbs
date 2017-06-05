#include "OrbsPluginPCH.h"
#include "SphereGlowOutline.h"

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/RenderingHelpers/ScratchTexturePool.hpp>


//////////////////////////SphereGlowOutlineRenderer//////////////////////////
void SphereGlowOutlineRenderer::init()
{
	Vision::Callbacks.OnRenderHook += this;

	LoadShaders();

	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	int iScreenX = (int) screenSize.x;
	int iScreenY = (int) screenSize.y;
	int iBlurTexX = iScreenX;
	int iBlurTexY = iScreenY;
	VTextureLoader::VTextureFormat_e eFormat = VTextureLoader::DEFAULT_RENDERTARGET_FORMAT_32BPP;
	m_pSourceRenderTarget = ScratchTexturePool_cl::GlobalManager().GetScratchTexture(iBlurTexX, iBlurTexY, eFormat, 0, 0, false, 0, 0, false, false);
	m_pPingPongRenderTargets[0] = ScratchTexturePool_cl::GlobalManager().GetScratchTexture(iBlurTexX, iBlurTexY, eFormat, 0, 0, false, 0, 0, false, false);
	m_pPingPongRenderTargets[1] = ScratchTexturePool_cl::GlobalManager().GetScratchTexture(iBlurTexX, iBlurTexY, eFormat, 0, 0, false, 0, 0, false, false);

	m_pMainContext = new VisRenderContext_cl();
	m_pMainContext->SetCamera(Vision::Contexts.GetCurrentContext()->GetCamera());
	m_pMainContext->SetUsageHint(VIS_CONTEXTUSAGE_NONE);
	m_pMainContext->SetRenderTarget(0, m_pSourceRenderTarget);
	m_pMainContext->SetDepthStencilTarget(NULL);
	m_pMainContext->SetName("SphereGlowOutlineRenderer:MainContext");
	VisionVisibilityCollector_cl* pVisColl = new VisionVisibilityCollector_cl();
	m_pMainContext->SetVisibilityCollector(pVisColl);
	m_pRenderLoop = new SphereGlowOutlineRenderLoop();
	m_pRenderLoop->SetParentRenderer(this);
	m_pMainContext->SetRenderLoop(m_pRenderLoop);
	//m_pMainContext->SetRenderingEnabled(false);
	pVisColl->SetOcclusionQueryRenderContext(m_pMainContext);
	Vision::Contexts.AddContext(m_pMainContext);

	for (int i = 0; i < 2; i++)
	{
		m_pPingPongContexts[i] = new VisRenderContext_cl();
		m_pPingPongContexts[i]->SetCamera(Vision::Contexts.GetCurrentContext()->GetCamera());
		m_pPingPongContexts[i]->SetUsageHint(VIS_CONTEXTUSAGE_NONE);
		m_pPingPongContexts[i]->SetRenderFlags(VIS_RENDERCONTEXT_FLAG_NO_SCENEELEMENTS);
		m_pPingPongContexts[i]->SetRenderTarget(0, m_pPingPongRenderTargets[i]);
		m_pPingPongContexts[i]->SetDepthStencilTarget(NULL);
		m_pPingPongContexts[i]->SetName("SphereGlowOutlineRenderer:PingPong");
	}

	m_pFinalMask = CreateScreenMask(iScreenX, iScreenY, m_pPingPongRenderTargets[0]);
	m_pHorBlurMask = CreateScreenMask(iScreenX, iScreenY, m_pPingPongRenderTargets[0]);
	m_pVertBlurMask = CreateScreenMask(iScreenX, iScreenY, m_pPingPongRenderTargets[1]);

	//set blur technique to relevant masks
	m_pHorBlurMask->SetTechnique(m_pBlurTechnique);
	m_pVertBlurMask->SetTechnique(m_pBlurTechnique);

	// Get constant registers
	VCompiledShaderPass* pBlurPass = m_pBlurTechnique->GetShader(0);
	bool result = m_regBlurStep.Init(pBlurPass, "BlurSize");

	VCompiledShaderPass* pOutlinePass = m_pOutlineTechnique->GetShader(0);
	result = m_regOutlineColor.Init(pOutlinePass, "OutlineColor");
	result = m_regOutlineWidth.Init(pOutlinePass, "OutlineWidth");
}

void SphereGlowOutlineRenderer::deInit()
{
	Vision::Contexts.RemoveContext(m_pMainContext);
	m_pMainContext = NULL;
	m_pPingPongContexts[0] = NULL;
	m_pPingPongContexts[1] = NULL;
	m_pSourceRenderTarget = NULL;
	m_pPingPongRenderTargets[0] = NULL;
	m_pPingPongRenderTargets[1] = NULL;
	m_pFinalMask = NULL;
	m_pHorBlurMask = NULL;
	m_pVertBlurMask = NULL;
	m_pRenderLoop = NULL;

	Vision::Callbacks.OnRenderHook -= this;
}

void SphereGlowOutlineRenderer::Execute()
{
	return;

	IVisVisibilityCollector_cl *pCollector = Vision::Contexts.GetCurrentContext()->GetVisibilityCollector();

	Vision::RenderLoopHelper.ClearScreen();

	m_pPingPongContexts[0]->Activate();
	VCompiledShaderPass* pOutlinePass = m_pOutlineTechnique->GetShader(0);
	const VisEntityCollection_cl *pEntities = pCollector->GetVisibleEntities();
	if (pEntities)
	{
		for (int i = 0; i < (int)pEntities->GetNumEntries(); i++)
		{
			VisBaseEntity_cl* pEntity = pEntities->GetEntry(i);
			if (vdynamic_cast<NodeSphere*>(pEntity))
			{
				if (vdynamic_cast<SacrificePit*>(pEntity)) //Sacrifice altars have no outer glow
					continue;

				NodeSphere* nodeSphere = (NodeSphere*) pEntity;
				//set color
				VColorRef sphereTeamColor = nodeSphere->getTeam() ? nodeSphere->getTeam()->getColor() : TEAM_NEUTRAL_COLOR;
				hkvVec4 sphereTeamColorVec4 = sphereTeamColor.getAsVec4();
				const float fRegOutlineColorValue[4] = {sphereTeamColorVec4.x, sphereTeamColorVec4.y, sphereTeamColorVec4.z, sphereTeamColorVec4.w};
				m_regOutlineColor.SetRegisterValueF(pOutlinePass, fRegOutlineColorValue);
				//and width
				float fRegOutlineWidth = 0.5f / pEntity->GetScaling().x; //uniform scaling take the x-coords
				m_regOutlineWidth.SetSingleValueF(pOutlinePass, fRegOutlineWidth);
				//render entities
				//Vision::RenderLoopHelper.RenderEntityWithShaders(pEntity, m_pOutlineTechnique->GetShaderCount(), m_pOutlineTechnique->GetShaderList());
				Vision::RenderLoopHelper.RenderEntityWithShaders(pEntity, 1, &pOutlinePass);
			}
		}
	}

	this->m_bMaskRendered = false;

	VCompiledShaderPass* pPass = m_pHorBlurMask->GetTechnique()->GetShader(0);
	VASSERT(pPass != NULL);

	float fHorBlurStepValue = 6.0f / 1024.0f;
	float fVertBlurStepValue = 6.0f / 768.0f;
	const float horBlurStep[4] = { fHorBlurStepValue, 0.0f, 0.0f, 0.0f };
	const float vertBlurStep[4] = { 0.0f, fVertBlurStepValue, 0.0f, 0.0f };
	int blurPasses = 1;

	for (int i = 0; i < blurPasses; i++)
	{
		// Horizontal blur pass
		m_regBlurStep.SetRegisterValueF(pPass, horBlurStep);
		m_pPingPongContexts[1]->Activate();
		VisRenderContext_cl::PerformPendingContextSwitch();

		VisScreenMask_cl* pMask = m_pHorBlurMask;
		Vision::RenderLoopHelper.RenderScreenMasks(&pMask, 1, NULL);

		// Vertical blur pass
		m_regBlurStep.SetRegisterValueF(pPass, vertBlurStep);
		m_pPingPongContexts[0]->Activate();
		VisRenderContext_cl::PerformPendingContextSwitch();

		pMask = m_pVertBlurMask;
		Vision::RenderLoopHelper.RenderScreenMasks(&pMask, 1, NULL);
	}

	////VisRenderContext_cl* referenceCtx = Vision::Renderer.GetCurrentRendererNode()->GetReferenceContext();
	////IVRendererNode* rendererNode = Vision::Renderer.GetCurrentRendererNode();
	//IVRendererNode* rendererNode = Vision::Renderer.GetRendererNode(0);
	//int count = Vision::Renderer.GetRendererNodeCount();
	//if (rendererNode)
	//{
	//	VisRenderContext_cl* referenceCtx = rendererNode->GetReferenceContext();
	//	//VisRenderContext_cl* referenceCtx = this->GetParentRenderer()->getReferenceContext();
	//	referenceCtx->Activate();
	//	VisRenderContext_cl* activeCtx = Vision::Contexts.GetCurrentContext();
	//	VisScreenMask_cl* pMask = this->GetParentRenderer()->getFinalMask();
	//	Vision::RenderLoopHelper.RenderScreenMasks(&pMask, 1, NULL);
	//}
}

void SphereGlowOutlineRenderer::LoadShaders()
{
	// load shaders that should be used in this renderloop
	VShaderEffectLib* outlineShaderLib = Vision::Shaders.LoadShaderLibrary("Shaders\\Outline.ShaderLib");
	VShaderEffectLib* postProcessShaderLib = Vision::Shaders.LoadShaderLibrary("Shaders\\PostProcess.ShaderLib");
	m_pOutlineTechnique =  Vision::Shaders.CreateTechnique("OutlineEffect", "");
	m_pBlurTechnique = Vision::Shaders.CreateTechnique("GaussianBlur", "");
}

VisScreenMask_cl* SphereGlowOutlineRenderer::CreateScreenMask(int iSizeX, int iSizeY, VTextureObject *pTexture)
{
  VisScreenMask_cl *pNewMask = new VisScreenMask_cl();
  pNewMask->SetTextureObject(pTexture);

  pNewMask->SetPos(0,0);
  pNewMask->SetTargetSize((float)iSizeX,(float)iSizeY);
  pNewMask->SetTextureRange(0.f, 0, (float)iSizeX, (float)iSizeY);
  pNewMask->SetTransparency(VIS_TRANSP_ADDITIVE);
  pNewMask->SetVisible(FALSE);
  pNewMask->SetUseOpenGLTexelShift(FALSE);
  pNewMask->SetDepthWrite(FALSE);
  pNewMask->SetWrapping(FALSE, FALSE);

  return pNewMask;
}

void SphereGlowOutlineRenderer::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	return;

	//VisRenderContext_cl* currentContext = Vision::Contexts.GetCurrentContext();
	//VisRenderContext_cl* mainRenderContext = Vision::Contexts.GetMainRenderContext();

	//VisRenderContext_cl* finalCtx = Vision::Renderer.GetCurrentRendererNode()->GetFinalTargetContext();
	//VisRenderContext_cl* referenceCtx = Vision::Renderer.GetCurrentRendererNode()->GetReferenceContext();
	if (pData->m_pSender==&Vision::Callbacks.OnRenderHook) 
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);

		//hkvLog::Error("onHandleCallback %i", pRHDO->m_iEntryConst);

		if (pRHDO->m_iEntryConst == VRH_POST_TRANSPARENT_PASS_GEOMETRY /*&& !m_bMaskRendered*/)
		{	
			hkvLog::Error("onHandleCallback VRH_POST_TRANSPARENT_PASS_GEOMETRY");

			VisScreenMask_cl* pMask = m_pFinalMask;
			Vision::RenderLoopHelper.RenderScreenMasks(&pMask, 1, NULL);
			this->m_bMaskRendered = true;

			//render spheres with standard technique
			IVisVisibilityCollector_cl *pCollector = Vision::Contexts.GetCurrentContext()->GetVisibilityCollector();
			if (pCollector)
			{
				const VisEntityCollection_cl *pEntities = pCollector->GetVisibleEntities();
				if (pEntities)
				{
					for (int i = 0; i < (int)pEntities->GetNumEntries(); i++)
					{
						VisBaseEntity_cl* pEntity = pEntities->GetEntry(i);
						if (vdynamic_cast<NodeSphere*>(pEntity))
						{
							if (vdynamic_cast<SacrificePit*>(pEntity))
								continue;

							NodeSphere* nodeSphere = (NodeSphere*) pEntity;
							VCompiledTechnique* stdTechnique = nodeSphere->getStandardTechnique();
							Vision::RenderLoopHelper.RenderEntityWithShaders(nodeSphere, stdTechnique->GetShaderCount(), stdTechnique->GetShaderList());
						}
					}
				}
			}
		}

		return;
	}
}

//////////////////////////SphereGlowOutlineRenderLoop//////////////////////////
void SphereGlowOutlineRenderLoop::OnDoRenderLoop(void *pUserData)
{
	return;
	IVisVisibilityCollector_cl *pCollector = Vision::Contexts.GetCurrentContext()->GetVisibilityCollector();
	if (pCollector==NULL)
		return;

	RenderHook(NULL, NULL, VRH_PRE_RENDERING, true);

	this->m_pParentRenderer->Execute();

	RenderHook(NULL, NULL, VRH_POST_TRANSPARENT_PASS_GEOMETRY, true);
}