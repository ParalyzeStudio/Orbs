#include "OrbsPluginPCH.h"
#include "TexturedSphere.h"

V_IMPLEMENT_SERIAL(TexturedSphere, VisBaseEntity_cl, 0, &g_myComponentModule);

TexturedSphere::~TexturedSphere(void)
{
	this->m_pHalo = NULL;
}

void TexturedSphere::init()
{
	//set radius of the sphere
	this->radius = 50.0f * this->GetScaling().x; //50.0f is the radius of the sphere model
	
	//init halo
	this->m_pHalo = new SphereGlowHalo();
	float haloRadius = 1.45f * radius;
	m_pHalo->init(this->GetPosition(), hkvVec2(2 * haloRadius, 2 * haloRadius), this);
}

void TexturedSphere::invalidateTechnique(VColorRef tintColor, bool isVisibleByOrbs)
{
	Vision::Shaders.LoadShaderLibrary("shaders\\OrbsShaders.ShaderLib");
	VTechniqueConfig techConfig;
	techConfig.SetInclusionTags("LIGHTGRIDSIMPLE;LIGHTGRID;");
	string strParamString;
	strParamString += string("ScrollSpeed=0,0;");
	strParamString += string("DiffuseTintColor=");
	hkvVec4 colorVec4 = tintColor.getAsVec4();
	//colorVec4 = hkvVec4(1,1,1,1);

	if (!isVisibleByOrbs) //desaturate the sphere color
	{
		//float greyColor = 0.3f * colorVec4.x + 0.59f * colorVec4.y + 0.11f * colorVec4.z;
		//colorVec4.x = greyColor * 0.3f;
		//colorVec4.y = greyColor * 0.3f;
		//colorVec4.z = greyColor * 0.3f;

		//darken the color
		colorVec4.x *= 0.3f;
		colorVec4.y *= 0.3f;
		colorVec4.z *= 0.3f;
	}

	strParamString += stringFromFloat(colorVec4.x) + string(",") + stringFromFloat(colorVec4.y) + string(",") + stringFromFloat(colorVec4.z) + string(",") + stringFromFloat(colorVec4.w) + string(";");
	this->m_pStdTechnique = Vision::Shaders.CreateTechnique("BumpTintColor", strParamString.c_str(), &techConfig);

	VDynamicMesh *pModel = this->GetMesh();
	if (pModel && m_pStdTechnique)
	{
		VisShaderSet_cl *pSet = new VisShaderSet_cl();
		pSet->BuildForDynamicMesh(pModel,NULL,m_pStdTechnique);
		this->SetShaderSet(pSet);
	}
}

void TexturedSphere::replaceTexture(const char* pDiffuseTexture, const char* pNormalTexture)
{
	VisSurfaceTextureSet_cl* pTextureSet = this->CreateCustomTextureSet(true);
	VisSurface_cl** ppSurfaces = pTextureSet->AsSurfaceArray();
	int iSurfacesCount = pTextureSet->GetSurfaceCount();
	VisSurface_cl* pSurface = ppSurfaces[0];
	pSurface->SetTransparencyType(VIS_TRANSP_NONE);
	pSurface->m_spDiffuseTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	pSurface->m_spNormalMap = Vision::TextureManager.Load2DTexture("Textures\\bump_noise.dds");

	pSurface->SetTechnique(m_pStdTechnique);
	pSurface->SetLightingMode(VIS_LIGHTING_LIGHTGRID);
	this->SetCustomTextureSet(pTextureSet);
	pSurface->ReapplyShader(true);
}