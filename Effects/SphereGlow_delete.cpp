#include "OrbsPluginPCH.h"
//#include "SphereGlow.h"
//
//V_IMPLEMENT_DYNAMIC(VPostProcessingBaseComponent, IVObjectComponent, &g_myComponentModule);
//
//SphereGlow::SphereGlow(GlowDownscale_e downscaleMode)
//{
//	m_iRequiredBufferFlags = VBUFFERFLAG_FINALCOLOR;
//	m_fPriority = VPOSTPROCESSOR_PRIORITY_GLOW;
//	m_bufferUsageFlags = SAMPLES_ACCUMULATION_BUFFER | DRAWS_FULLSCREEN_QUAD | USES_BLENDING;
//}
//
//SphereGlow::~SphereGlow()
//{
//
//}
//
//// Initialization
//void SphereGlow::InitializePostProcessor()
//{
//	if (m_bIsInitialized)
//    return;
//
//  SetupContext();
//
//  // Load glow shader library
//  BOOL bResult = Vision::Shaders.LoadShaderLibrary("\\Shaders\\PostProcess.ShaderLib", SHADERLIBFLAG_HIDDEN) != NULL;
//  VASSERT(bResult); // file not found?
//  
//  GetTargetContext()->GetSize(m_iScreenX,m_iScreenY);
//  m_iScreenTexX = m_iScreenX;
//  m_iScreenTexY = m_iScreenY;
//  
//  int iSizeDivider, iSizeRemainderMask; 
//  if (downscaleMode == GLOW_DOWNSCALE_4X)
//  {
//    iSizeDivider = 4; iSizeRemainderMask = 3;
//  }
//  else
//  {
//    iSizeDivider = 2; iSizeRemainderMask = 1;
//  }
//
//  m_iBlurX = (m_iScreenX+(iSizeDivider>>1)) / iSizeDivider;
//  m_iBlurY = (m_iScreenY+(iSizeDivider>>1)) / iSizeDivider;
//
//  // make sure resolution is multiple of iSizeDivider
//  m_iBlurY &= ~iSizeRemainderMask;
//  m_iBlurX &= ~iSizeRemainderMask;
//  m_iBlurTexX = m_iBlurX;
//  m_iBlurTexY = m_iBlurY;
//
//  VTextureLoader::VTextureFormat_e eFormat = VTextureLoader::DEFAULT_RENDERTARGET_FORMAT_32BPP;
//  if (m_spSourceTextures[0]->IsRenderable())
//  {
//    const VisRenderableTextureConfig_t *pConfig = 
//      static_cast<VisRenderableTexture_cl*>(m_spSourceTextures[0].m_pPtr)->GetConfig();
//    eFormat = pConfig->m_eFormat;
//  }
//    
//  m_spPRT[0] = ScratchTexturePool_cl::GlobalManager().GetScratchTexture(
//    m_iBlurTexX, m_iBlurTexY, eFormat, 0, 0, false, 0, 0, false, false);
//  m_spPRT[1] = ScratchTexturePool_cl::GlobalManager().GetScratchTexture(
//    m_iBlurTexX, m_iBlurTexY, eFormat, 1, 0, false, 0, 0, false, false);
//
//  VASSERT(m_spPRT[0]!=NULL && m_spPRT[1]!=NULL);
//  int i;
//  for (i=0; i<2; i++)
//  {
//    m_spPingPongContext[i] = new VisRenderContext_cl();
//    m_spPingPongContext[i]->SetCamera(GetTargetContext()->GetCamera());
//    m_spPingPongContext[i]->SetUsageHint(VIS_CONTEXTUSAGE_NONE);
//    m_spPingPongContext[i]->SetRenderFlags(VIS_RENDERCONTEXT_FLAG_NO_SCENEELEMENTS);
//    m_spPingPongContext[i]->SetRenderTarget(0, m_spPRT[i]);
//    m_spPingPongContext[i]->SetDepthStencilTarget(NULL);
//    m_spPingPongContext[i]->SetName("PostProcessGlow:PingPong");
//  }
//
//  // Next, create the screen masks we'll use for the glow effect:
//  // - m_spDownsampleMask: Mask used to perform the downsampling step of the glow geometry
//  // - m_spHorBlurMask, m_pVertBlurMask: Masks used for a box blur of the downsampled glow mask
//  // - m_spSceneMask: Mask used to draw the main scene previously copied into the SceneTexture
//  // - m_spFinalMask: Used for an additive pass of the finished blurred glow mask over the scene mask
//  m_spDownsampleMask = CreateScreenMask(m_iBlurTexX, m_iBlurTexY, m_spSourceTextures[0]); 
//  m_spHorBlurMask = CreateScreenMask(m_iBlurTexX, m_iBlurTexY, m_spPRT[0]);
//  m_spVertBlurMask = CreateScreenMask(m_iBlurTexX, m_iBlurTexY, m_spPRT[1]);
//  m_spSceneMask = CreateScreenMask(m_iScreenX, m_iScreenY, m_spSourceTextures[0]);
//  m_spFinalMask = CreateScreenMask(m_iScreenX, m_iScreenY, m_spPRT[0]);
//
//  m_spSceneMask->SetTextureRange(0.0f, 0.0f, (float)m_iScreenX, (float)m_iScreenY);
//  m_spDownsampleMask->SetTextureRange(0.0f, 0.0f, (float)m_iScreenX, (float)m_iScreenY);
//  m_spHorBlurMask->SetTextureRange(0.0f, 0.0f, (float)m_iBlurX, (float)m_iBlurY);
//  m_spVertBlurMask->SetTextureRange(0.0f, 0.0f, (float)m_iBlurX, (float)m_iBlurY);
//  m_spFinalMask->SetTextureRange(0.0f, 0.0f, (float)m_iBlurX, (float)m_iBlurY);
//
//#if defined(_VR_DX9)
//  m_spSceneMask->SetUseOpenGLTexelShift(TRUE);
//  m_spDownsampleMask->SetUseOpenGLTexelShift(TRUE);
//  m_spHorBlurMask->SetUseOpenGLTexelShift(TRUE);
//  m_spVertBlurMask->SetUseOpenGLTexelShift(TRUE);
//  m_spFinalMask->SetUseOpenGLTexelShift(TRUE);
//#endif
//
//  m_spSceneMask->SetFiltering(FALSE);
//
//  //m_spBlurMask->SetTransparency(VIS_TRANSP_ADDITIVE);
//  m_spHorBlurMask->SetTransparency(VIS_TRANSP_NONE);
//  m_spVertBlurMask->SetTransparency(VIS_TRANSP_NONE);
//  m_spDownsampleMask->SetTransparency(VIS_TRANSP_NONE);
//
//  VSimpleRenderState_t maskState(VIS_TRANSP_ADDITIVE, 
//    RENDERSTATEFLAG_DOUBLESIDED | RENDERSTATEFLAG_ALWAYSVISIBLE);
//  m_spFinalMask->SetRenderState(maskState);
//
//  m_bIsInitialized = true;
//
//  // Set default offset for the box blur in pixels
//  SetBlurOffset(BlurValue);
//
//  // Set bias, exponent and scale variables for the glow effect.
//  // The operation performed per pixel is: 
//  // fragment = (color + bias)^exponent * scale
//  SetGlowParameters(Bias, Exponent, Scale);
//
//  // Create PositionTextureEmulateBilinear effect of fixed function shaders
//  bool bFilteringSupported = Vision::Video.IsTextureFilteringSupported(
//    m_spPRT[0]->GetResolvedTextureFormat(), *Vision::Video.GetCurrentConfig());
//#if defined(_VISION_XENON)
//  bFilteringSupported = (bFilteringSupported || m_spPRT[0]->GetConfig()->m_bExpand);
//#endif
//
//  if (!bFilteringSupported)
//  {
//    // bilinear filtering not supported for the given texture format
//    char szParamString[128];
//    sprintf(szParamString, "invTexSize=%f,%f;", 
//      1.0f / static_cast<float>(m_spPRT[0]->GetTextureWidth()),
//      1.0f / static_cast<float>(m_spPRT[0]->GetTextureHeight()));
//
//    IVisShaderProvider_cl* pProvider = Vision::GetApplication()->GetShaderProvider();
//    VASSERT_MSG(pProvider != NULL, "A shader provider must be set");
//
//    // TODO: do this automatically when rendering screen masks and bilinear filtering is
//    // required, but not supported for specific texture formats. (see HS#7308)
//    // (Only if RENDERSTATEFLAG_FILTERING is enabled)
//    m_spPositionTexBilinearTechnique = Vision::Shaders.CreateTechnique(
//      "PositionTextureEmulateBilinear", szParamString, NULL, EFFECTCREATEFLAG_FORCEUNIQUE,
//      pProvider->LoadFixedFunctionShaderLib());
//
//    m_spFinalMask->SetTechnique(m_spPositionTexBilinearTechnique);
//
//    // Blend state needs to be overridden 
//    // (may be removed if screen masks take care of this automatically)
//    VASSERT(m_spPositionTexBilinearTechnique->GetShaderCount() > 0);
//    VStateGroupBlend& blendState = 
//      m_spPositionTexBilinearTechnique->m_Shaders.GetAt(0)->GetRenderState()->GetBlendState();
//    blendState.m_cBlendEnable = 1;
//    blendState.m_cBlendOp = BLEND_OP_ADD;
//    blendState.m_cSrcBlend = BLEND_ONE;
//    blendState.m_cDestBlend = BLEND_ONE;
//    blendState.ComputeHash();
//  }
//
//  VCompiledTechnique *pTech;
//  if (downscaleMode == GLOW_DOWNSCALE_4X)
//  {
//    pTech = Vision::Shaders.CreateTechnique(
//      bFilteringSupported ? "GlowDownsample4x4": "GlowDownsample4x4Point", NULL);
//  }
//  else
//  {
//    pTech = Vision::Shaders.CreateTechnique(
//      bFilteringSupported ? "GlowDownsample2x2" : "GlowDownsample2x2Point", NULL);
//  }
//
//  if (!pTech)
//  {
//    m_bValid = false;
//    return;
//  }
//
//  m_spDownsampleMask->SetTechnique(pTech);
//
//  m_regDownsampleGlowparams.Init(pTech->GetShader(0), "GlowParams");
//  m_regDownsampleStepSize.Init(pTech->GetShader(0), "StepSize");
//
//  // Create blur technique
//  pTech = Vision::Shaders.CreateTechnique(
//    bFilteringSupported ? "Blur" : "BlurPoint", "", NULL);
//  if (!pTech)
//  {
//    m_bValid = false;
//    return;
//  }
//  m_spHorBlurMask->SetTechnique(pTech);
//  m_spVertBlurMask->SetTechnique(pTech);
//
//  // Get constant register for step size
//  VASSERT(pTech->GetShaderCount() > 0);
//  VCompiledShaderPass* pPass = pTech->GetShader(0);
//  VASSERT(pPass != NULL);
//  m_regBlurStep.Init(pPass, "StepSize");
//  VASSERT(m_regBlurStep.IsValid());
//
//}
//
//// Deinitialization
//void SphereGlow::DeInitializePostProcessor()
//{
//	if(!m_bIsInitialized)
//	{
//		return;
//	}
//
//	RemoveShaders();
//	RemoveContext();
//
//	m_bIsInitialized = false;
//}
//
//// Main function of the post-processor
//void SphereGlow::Execute()
//{
//
//}
//
//// Internal helper functions to setup and tear down our target context
//void SphereGlow::SetupContext()
//{
//	// This is the context provided by the renderer node
//	VisRenderContext_cl* pTargetContext = GetTargetContext();
//
//	pTargetContext->SetRenderFlags(VIS_RENDERCONTEXT_FLAG_SHOW_DEBUGOUTPUT);
//	pTargetContext->SetCamera(GetOwner()->GetReferenceContext()->GetCamera());
//
//	// Set any other context properties as desired
//	pTargetContext->SetName("SphereGlowContext");
//
//	// By default, the post-processor shares the projection matrix with the renderer node,
//	// so we need to create a separate view properties object if we want to modify it:
//	// pTargetContext->SetViewProperties(new VisContextViewProperties());
//	// pTargetContext->SetCustomProjectionMatrix(pMyProj);
//
//	pTargetContext->SetUserData(this);
//	pTargetContext->SetPriority(m_fPriority);
//
//	// Register the context to be executed inside the renderer node
//	GetOwner()->AddContext(pTargetContext);
//
//	pTargetContext->SetRenderLoop(new PostProcessRenderLoop_cl(this));
//	pTargetContext->SetVisibilityCollector(NULL);
//
//	// Fetch a reference to the source texture we'd like to process
//	m_spSourceTextures[0] = GetOwner()->GetGBuffer(IVRendererNode::VGBT_Accumulation);
//	m_iNumSourceTextures = 1;
//}
//
//void SphereGlow::RemoveContext()
//{
//	// Free source texture references
//	m_spSourceTextures[0] = NULL;
//
//	// Unregister the target context
//	GetOwner()->RemoveContext(GetTargetContext());
//
//}
//
//// Internal helper function to create and remove our shader resources
//void SphereGlow::CreateShaders()
//{
//
//}
//
//
//void SphereGlow::RemoveShaders()
//{
//
//}
//
////------------------------------Serialize------------------------------//
//V_IMPLEMENT_SERIAL(SphereGlow, VPostProcessingBaseComponent, 0, &g_myComponentModule);
//
//
////VTypedObject* SphereGlow::CreateObject()
////{ 
////	return new SphereGlow; 
////}
////
////VType SphereGlow::classSphereGlow = 
////{
////	"class_name", SphereGlow::CreateObject, sizeof(class SphereGlow), 0,
////	/* m_pBaseClass     = */ ((VType*)(&VPostProcessingBaseComponent::classVPostProcessingBaseComponent)),
////	/* m_pModule        = */ &g_myComponentModule,
////	/* m_pParamDesc     = */ NULL,
////	/* GetFullParamDesc = */ NULL,
////	/* m_bFullParamInit = */ FALSE,
////	/* m_pVarList       = */ NULL,
////	/* m_iReserved  = */ 0,
////	/* m_pNextType      = */ NULL 
////};
////
////VType *SphereGlow::GetClassTypeId() 
////{
////	V_COMPILE_ASSERT( sizeof(SphereGlow::classSphereGlow)== EXPECTED_VTYPE_SIZE );
////	V_COMPILE_ASSERT( offsetof(VType,m_pBaseClass) == EXPECTED_VTYPE_OFFSET );
////	return V_RUNTIME_CLASS(SphereGlow);
////}
////
////VType *SphereGlow::GetTypeId() const
////{ 
////	return V_RUNTIME_CLASS(SphereGlow); 
////}
////
////V_REGISTER_CLASS _init_SphereGlow(V_RUNTIME_CLASS(SphereGlow)); 
////
////VArchive& operator>>( VArchive& ar, SphereGlow* &pObj )
////{
////	pObj = (SphereGlow*)ar.ReadObject( V_RUNTIME_CLASS(SphereGlow) );
////	return ar; 
////} 
//
//
//
//
//void SphereGlow::Serialize( VArchive &ar )
//{
//	if (ar.IsLoading())
//	{
//		ar >> m_bActive;
//		int iDownscaleMode;
//		ar >> iDownscaleMode;
//		downscaleMode = static_cast<GlowDownscale_e>(iDownscaleMode);
//	}
//	else
//	{
//		ar << m_bActive;
//		ar << static_cast<int>(downscaleMode);
//	}
//}
//
//
//START_VAR_TABLE(SphereGlow, VPostProcessingBaseComponent, "SphereGlow", 0, "SphereGlow")  
//  DEFINE_VAR_ENUM(SphereGlow, downscaleMode, "Downscale mode (4x is slightly faster, 2x produces higher quality)", "DOWNSCALE_2X", "DOWNSCALE_2X,DOWNSCALE_4X", 0, 0);
//END_VAR_TABLE
