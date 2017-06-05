//#pragma once
//
//
//#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Postprocessing/PostProcessBase.hpp>
//#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/RendererNode/VRendererNodeCommon.hpp>
//
//
//enum GlowDownscale_e
//{
//	GLOW_DOWNSCALE_2X = 0,      ///< Glow texture has 1/4 of the size of the input texture (looks slightly better)
//	GLOW_DOWNSCALE_4X = 1       ///< Glow texture has 1/16 of the size of the input texture (faster)
//};
//
//class SphereGlow : public VPostProcessingBaseComponent
//{
//public:
//	SphereGlow(){};
//	SphereGlow(GlowDownscale_e downscaleMode);
//	virtual ~SphereGlow();
//
//	// Declaration for serialization
//	V_DECLARE_SERIAL_DLLEXP(SphereGlow, DECLSPEC_DLLEXPORT);
//	//VARTABLE declaration
//	IMPLEMENT_OBJ_CLASS(SphereGlow);
//	virtual void Serialize(VArchive &ar);
//
//protected:
//	// Initialization
//	virtual void InitializePostProcessor() HKV_OVERRIDE;
//
//	// Deinitialization
//	virtual void DeInitializePostProcessor() HKV_OVERRIDE;
//
//	// Main function of the post-processor
//	virtual void Execute() HKV_OVERRIDE;
//
//	// Internal helper functions to setup and tear down our target context
//	void SetupContext();
//	void RemoveContext();
//
//	// Internal helper function to create and remove our shader resources
//	void CreateShaders();
//	void RemoveShaders();
//
//	// The technique we use for rendering
//	VCompiledTechniquePtr m_spCustomTechnique;
//
//	//VARTABLE variables
//	GlowDownscale_e downscaleMode;
//
//	// dimensions
//	int m_iScreenX, m_iScreenY;
//	int m_iScreenTexX, m_iScreenTexY;
//	int m_iBlurX, m_iBlurY;
//	int m_iBlurTexX, m_iBlurTexY;
//	float m_fHorBlurStep, m_fVertBlurStep;
//	VConstantBufferRegister m_regBlurStep;
//};
