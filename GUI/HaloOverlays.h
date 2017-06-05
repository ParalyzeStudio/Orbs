#pragma once

class HaloArrow;

#define SELECTION_HALO_ANGULAR_SPEED 30.0f //30 deg/sec

class SelectionHalo : public RotatingSpriteScreenMask
{
public:
	SelectionHalo() : RotatingSpriteScreenMask(),
		fixedArrow1(NULL),
		fixedArrow2(NULL),
		mobileArrow(NULL){};

	void init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0);
	void buildArrows();
	void updateForActionMode(int actionMode);
	void updateMobileArrowAngle(float newAngle);
	void updateSize();

	//float getScaleForCameraZoom();
	virtual void SetColor(VColorRef iColor);

	HaloArrow* getFixedArrow1(){return this->fixedArrow1;};
	HaloArrow* getFixedArrow2(){return this->fixedArrow2;};
	HaloArrow* getMobileArrow(){return this->mobileArrow;};

private:
	//arrows
	HaloArrow* fixedArrow1;
	HaloArrow* fixedArrow2;
	HaloArrow* mobileArrow;
};

#define DEFAULT_ARROW_MOVE_DURATION 0.3f

class HaloArrow : public SpriteScreenMask
{
public:
	HaloArrow() : SpriteScreenMask(),
		parentHalo(NULL),
		distanceFromHaloCenter(0),
		angle(0),
		moving(false),
		startAngle(0),
		targetAngle(0),
		angleTotalVariation(0),
		moveDuration(0),
		moveElapsedTime(0){};

	void init(SelectionHalo* parentHalo, float distanceFromHaloCenter, float angle);
	void updatePositionAndAngle();
	void moveToAngle(float targetAngle);
	float angleLinearVariation(float dt);

	void update(float dt);

	void setDistanceFromHaloCenter(float distanceFromHaloCenter){this->distanceFromHaloCenter = distanceFromHaloCenter;};
private:
	SelectionHalo* parentHalo;
	float distanceFromHaloCenter;
	float angle;

	//move
	bool moving;
	float startAngle;
	float targetAngle;
	float angleTotalVariation;
	float moveDuration;
	float moveElapsedTime;
};

class EndpointHalo : public SpriteScreenMask
{
public:
	EndpointHalo() : SpriteScreenMask() {};
	void init(hkvVec2 position, hkvVec2 size, VColorRef tintColor, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0);
};

#define PATH_LINK_THICKNESS 16.0f

class Path : public VRefCounter, public IVisCallbackHandler_cl
{
public:
	Path();
	~Path();

	void init(vector<NodeSphere*> &spheresPath, VColorRef color);
	void init(NodeSphere* startSphere, hkvVec2 endPoint, VColorRef color);

	void buildSingleLinePath(NodeSphere* startSphere, hkvVec2 endPoint, bool drawHalos, bool drawFirstPoint);
	void buildSpheresPath(vector<NodeSphere*> &spheresPath);

	void updateSingleLinePath(int lineNumber, hkvVec2 newEndPointPosition);
	void updateSpheresPath();

	void removeSelf();
	void removeLinks();
	void removePoints();
	void removeHalos();

	void fadeOut(float fadeDuration);
	void SetOpacity(float opacity);

	void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	void update(float dt);

	void SetColor(VColorRef color);
	VColorRef GetColor(){return this->color;};
	const vector<SpriteScreenMask*> &getHalos(){return this->halos;};
	const vector<SpriteScreenMask*> &getPoints(){return this->points;};
	const vector<SpriteScreenMask*> &getLinks(){return this->links;};

private:
	vector<NodeSphere*> spheresPath;
	vector<SpriteScreenMask*> links;
	vector<SpriteScreenMask*> points;
	vector<SpriteScreenMask*> halos;
	VColorRef color;
	float opacity;

	bool fadingOut;
	float fadeElapsedTime;
	float fadeDuration;
	float fadeVariation;
};