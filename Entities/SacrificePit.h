#pragma once

#define SACRIFICE_PIT_LIFE 1000
#define DEFAULT_SACRIFICE_PERIOD 5.0f

class SacrificePit : public NodeSphere
{
public:
	SacrificePit(void) : NodeSphere(),
		sacrificeElapsedTime(0),
		sacrificePeriod(0)
	{};
	~SacrificePit(void) {};

	virtual void InitFunction();
	virtual void ThinkFunction();
	virtual void Serialize( VArchive &ar );

	V_DECLARE_SERIAL_DLLEXP(SacrificePit, DECLSPEC_DLLEXPORT ); 
	IMPLEMENT_OBJ_CLASS(SacrificePit);

	virtual void init();
	void initAtmosphere();

	void addSacrifice(Team* orbTeam);

	void animateCube(float dt);
	virtual void update(float dt);

private:
	float sacrificeElapsedTime;
	float sacrificePeriod;

	VisParticleEffectFile_cl* fxVerticalStretchedParticles;
};

