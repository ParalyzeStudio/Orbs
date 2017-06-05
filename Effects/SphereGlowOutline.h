#pragma once

class SphereGlowOutlineRenderLoop;

class SphereGlowOutlineRenderer : public IVisCallbackHandler_cl
{
public:
	SphereGlowOutlineRenderer() : m_pMainContext(NULL),
		m_pReferenceContext(NULL),
		m_pSourceRenderTarget(NULL),
		m_pFinalMask(NULL),
		m_pHorBlurMask(NULL),
		m_pVertBlurMask(NULL),
		m_pRenderLoop(NULL),
		m_pOutlineTechnique(NULL),
		m_pBlurTechnique(NULL),
		m_bMaskRendered(false){};

	~SphereGlowOutlineRenderer(){};

	void init();
	void deInit();
	void Execute();

	void LoadShaders();
	VisScreenMask_cl* CreateScreenMask(int iSizeX, int iSizeY, VTextureObject *pTexture);

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	VisScreenMask_cl* getFinalMask(){return this->m_pFinalMask;};
	VisRenderContext_cl* getReferenceContext(){return this->m_pReferenceContext;};
	void setMaskRendered(bool maskRendered){this->m_bMaskRendered = maskRendered;};

private:
	VisRenderContextPtr m_pMainContext;
	VisRenderContext_cl* m_pReferenceContext;
	VisRenderContextPtr m_pPingPongContexts[2];
	VisRenderableTexturePtr m_pSourceRenderTarget;
	VisRenderableTexturePtr m_pPingPongRenderTargets[2];
	VisScreenMaskPtr m_pFinalMask;
	VisScreenMaskPtr m_pHorBlurMask;
	VisScreenMaskPtr m_pVertBlurMask;

	VSmartPtr<SphereGlowOutlineRenderLoop> m_pRenderLoop;

	VCompiledTechniquePtr m_pOutlineTechnique;
	VCompiledTechniquePtr m_pBlurTechnique;
	VConstantBufferRegister m_regOutlineColor;
	VConstantBufferRegister m_regOutlineWidth;
	VConstantBufferRegister m_regBlurStep;

	bool m_bMaskRendered;
};

class SphereGlowOutlineRenderLoop : public VisionRenderLoop_cl
{
public:
	SphereGlowOutlineRenderLoop() : m_pParentRenderer(NULL){};
	virtual ~SphereGlowOutlineRenderLoop(){};

	// Render Loop implementation
	virtual void OnDoRenderLoop(void *pUserData) HKV_OVERRIDE;

	void SetParentRenderer(SphereGlowOutlineRenderer* renderer){this->m_pParentRenderer = renderer;};
	SphereGlowOutlineRenderer* GetParentRenderer(){return this->m_pParentRenderer;};

private:
	SphereGlowOutlineRenderer* m_pParentRenderer;
};

