#pragma once
#include "SceneGUIManager.h"

#define BRIDGE_DEFAULT_RADIUS 8.0f
#define BRIDGE_CONSTRUCTION_SPEED 50.0f //100 units per second

class Bridge : public BatchSprite
{
public:
	Bridge() : BatchSprite(), 
		startSphere(NULL), 
		endSphere(NULL),
		length(0),
		targetLength(0),
		maxLength(0),
		currentSegmentBuilt(0),
		activeOrbsCount(0), 
		totalOrbsCount(0),
		sharedVisibilityObject(NULL),
		revealed(false){};
	~Bridge();

	void init(NodeSphere* startSphere, NodeSphere* endSphere, bool completed);
	void init(NodeSphere* startSphere, NodeSphere* endSphere);
	virtual void initVisibilityObject();
	bool equals(Bridge* bridge);
	bool equals(NodeSphere* startSphere, NodeSphere* endSphere);

	void addOrb();
	void onSegmentEnded(); //when a segment is finished
	void increaseLength(float deltaLength);

	virtual void destroySelf();

	NodeSphere* getStartSphere(){return this->startSphere;};
	NodeSphere* getEndSphere(){return this->endSphere;};
	hkvVec3 getStartPoint(){return this->startSphere->GetPosition();};
	hkvVec3 getEndPoint(){return this->endSphere->GetPosition();};
	int getCost();
	float getLength(){return this->length;};
	float getMaxLength(){return this->maxLength;};
	BatchSpriteVisibilityObject* getSharedVisibilityObject(){return this->sharedVisibilityObject;};
	void setRevealed(bool revealed){this->revealed = revealed;};
	bool isRevealed(){return this->revealed;};

	virtual void update(float dt);

private:
	NodeSphere* startSphere;
	NodeSphere* endSphere;

	int activeOrbsCount; //current orbs that are building the bridge
	int totalOrbsCount; //count of all orbs that were spent in that bridge

	float length; //current length of the bridge
	float targetLength; //length the bridge has to reach in a certain amount of time
	float maxLength; //max length of the bridge
	int currentSegmentBuilt;

	VSmartPtr<BatchSpriteVisibilityObject> sharedVisibilityObject; //visibility object shared between orbs traveling along this bridge
	bool revealed;
};

class PotentialBridge : public Bridge
{
public:
	virtual void update(float dt);
	virtual void faceCamera();
};

//class Bridge : public BatchSprite
//{
//public:
//	Bridge() : BatchSprite(), 
//		startSphere(NULL), 
//		endSphere(NULL),
//		length(0),
//		targetLength(0),
//		maxLength(0),
//		currentSegmentBuilt(0),
//		activeOrbsCount(0), 
//		totalOrbsCount(0),
//		rotationMatrix(hkvMat3()),
//		position(hkvVec3(0,0,0)){};
//	~Bridge() {};
//
//	void init(NodeSphere* startSphere, NodeSphere* endSphere, bool completed);
//	void init(NodeSphere* startSphere, NodeSphere* endSphere);
//	bool equals(Bridge* bridge);
//	bool equals(NodeSphere* startSphere, NodeSphere* endSphere);
//	hkvMat4 getTransformationMatrix();
//
//	void addOrb();
//	void onSegmentEnded(); //when a segment is finished
//	void increaseLength(float deltaLength);
//
//	virtual void destroySelf();
//
//	NodeSphere* getStartSphere(){return this->startSphere;};
//	NodeSphere* getEndSphere(){return this->endSphere;};
//	hkvVec3 getStartPoint(){return this->startSphere->GetPosition();};
//	hkvVec3 getEndPoint(){return this->endSphere->GetPosition();};
//	int getCost();
//	float getLength(){return this->length;};
//	float getMaxLength(){return this->maxLength;};
//
//	void faceCamera();
//	void update(float dt);
//
//private:
//	NodeSphere* startSphere;
//	NodeSphere* endSphere;
//
//	int activeOrbsCount; //current orbs that are building the bridge
//	int totalOrbsCount; //count of all orbs that were spent in that bridge
//
//	float length; //current length of the bridge
//	float targetLength; //length the bridge has to reach in a certain amount of time
//	float maxLength; //max length of the bridge
//	int currentSegmentBuilt;
//
//	hkvMat3 rotationMatrix;
//	hkvVec3 position;
//};
