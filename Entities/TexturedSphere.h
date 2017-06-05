#pragma once

class SphereGlowHalo;

class TexturedSphere : public VisBaseEntity_cl
{
public:
	TexturedSphere(void) : VisBaseEntity_cl(), 
		m_pStdTechnique(NULL),
		radius(0),
		m_pHalo(NULL){};
	~TexturedSphere(void);

	V_DECLARE_SERIAL_DLLEXP(TexturedSphere, DECLSPEC_DLLEXPORT); 
	IMPLEMENT_OBJ_CLASS(TexturedSphere);

	void init();

	void invalidateTechnique(VColorRef tintColor, bool isVisibleByOrbs);
	void replaceTexture(const char* pDiffuseTexture, const char* pNormalTexture);

	float getRadius(){return this->radius;};
	SphereGlowHalo* getHalo(){return this->m_pHalo;};

protected:
	VCompiledTechnique* m_pStdTechnique;

	float radius; //the radius of the sphere
	VSmartPtr<SphereGlowHalo> m_pHalo;
};

