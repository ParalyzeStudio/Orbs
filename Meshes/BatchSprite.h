#pragma once

class BatchSpriteVisibilityObject;

class BatchSprite : public VRefCounter, public IVisCallbackHandler_cl
{
public:
	BatchSprite();
	~BatchSprite();  

	virtual void init(hkvVec3 position, hkvVec2 size);
	virtual void update(float dt);
	virtual void destroySelf();
	virtual void faceCamera();

	virtual void SetPosition(hkvVec3 position){this->position = position;};
	hkvMat3 GetRotationMatrix(){return this->rotationMatrix;};
	hkvVec3 GetPosition(){return this->position;};
	void SetSize(hkvVec2 size){this->size = size;};
	hkvVec2 GetSize(){return this->size;};
	float GetRotationAngle(){return this->rotationAngle;};
	void SetTintColor(VColorRef color){this->tintColor = color;};
	VColorRef GetTintColor(){return this->tintColor;};
	hkvMat4 getTransformationMatrix();

	//visibility
	virtual void initVisibilityObject();
	void setVisibilityObject(BatchSpriteVisibilityObject* visibilityObject);
	void passVisibilityTest(){this->visibilityTestValue = true;};
	void clearVisibilityTestValue(){this->visibilityTestValue = false;};
	bool hasPassedVisibilityTest(){
		#ifdef MAKE_ALL_SCENE_ELEMENTS_VISIBLE 
		return true; 
		#endif 
	return this->visibilityTestValue;};

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

protected:
	hkvMat3 rotationMatrix;
	hkvVec3 position;
	hkvMat4 transformationMatrix;
	hkvVec2 size;
	float rotationAngle;
	VColorRef tintColor;

	//visibility
	//VSmartPtr<VisVisibilityObject_cl> m_visibilityObject;
	VSmartPtr<BatchSpriteVisibilityObject> m_visibilityObject;
	bool visibilityTestValue;
};

class BatchSpriteVisibilityObject : public VisVisibilityObjectAABox_cl
{
public:
	BatchSpriteVisibilityObject(int iFlags = VISTESTFLAGS_PERFORM_ALL_TESTS) : VisVisibilityObjectAABox_cl(iFlags) {};

	V_DECLARE_DYNCREATE(BatchSpriteVisibilityObject);

	void attachBatchSprite(BatchSprite* attachedBatchSprite){this->attachedBatchSprites.push_back(attachedBatchSprite);};
	void detachBatchSprite(BatchSprite* batchSprite);
	vector<BatchSprite*> &getAttachedBatchSprites(){return this->attachedBatchSprites;};

private:
	vector<BatchSprite*> attachedBatchSprites;
};