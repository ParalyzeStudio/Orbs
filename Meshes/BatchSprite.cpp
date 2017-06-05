#include "OrbsPluginPCH.h"
#include "BatchSprite.h"
#include "GameManager.h"

///////////////////////////BatchSprite///////////////////////////
BatchSprite::BatchSprite() : VRefCounter(),
	rotationMatrix(hkvMat3()),
	position(hkvVec3(hkvNoInitialization)),
	transformationMatrix(hkvMat4()),
	size(hkvVec2(0,0)),
	rotationAngle(0),
	tintColor(V_RGBA_WHITE),
	m_visibilityObject(NULL),
	visibilityTestValue(false)
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

BatchSprite::~BatchSprite()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
	m_visibilityObject = NULL;
}

void BatchSprite::init(hkvVec3 position, hkvVec2 size)
{
	this->position = position;
	this->size = size;
	faceCamera();
	initVisibilityObject();
}

void BatchSprite::update(float dt)
{
	this->faceCamera();
}

void BatchSprite::destroySelf()
{
	this->Release();
}

void BatchSprite::faceCamera()
{
	hkvMat3 cameraRotationMatrix = GameManager::GlobalManager().getMainCamera()->GetRotationMatrix();
	this->rotationMatrix = cameraRotationMatrix.multiply(hkvMat3(0,1,0, -1,0,0, 0,0,1)); //-pi/2 rotation
	if (rotationAngle != 0)
	{
		hkvMat3 yRotationMatrix;
		yRotationMatrix.setIdentity();
		yRotationMatrix.setRotationMatrixY(rotationAngle);
		this->rotationMatrix = this->rotationMatrix.multiply(yRotationMatrix);
	}
}

hkvMat4 BatchSprite::getTransformationMatrix()
{
	return hkvMat4(this->rotationMatrix, this->position);
}

void BatchSprite::initVisibilityObject()
{
	m_visibilityObject = new BatchSpriteVisibilityObject(VISTESTFLAGS_ACTIVE | VISTESTFLAGS_FRUSTUMTEST);
	m_visibilityObject->SetActivate(TRUE);
	m_visibilityObject->SetVisibleBitmask(0xffffff);
	hkvAlignedBBox visibilityObjectBoundingBox;
	visibilityObjectBoundingBox.setCenterAndSize(hkvVec3(0,0,0), hkvVec3(0.5f * this->size.x, 0.5f * this->size.x ,0.5f * this->size.x));
	visibilityObjectBoundingBox.transformFromCenter(hkvMat4(this->rotationMatrix, hkvVec3(0,0,0)));
	m_visibilityObject->SetPosition(this->position);
	m_visibilityObject->SetLocalSpaceBoundingBox(visibilityObjectBoundingBox);
	m_visibilityObject->attachBatchSprite(this);
}

void BatchSprite::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
		this->update(dt);
	}
}


///////////////////////////BatchSpriteVisibilityObject///////////////////////////
V_IMPLEMENT_DYNCREATE(BatchSpriteVisibilityObject, VisVisibilityObjectAABox_cl, Vision::GetEngineModule());

void BatchSpriteVisibilityObject::detachBatchSprite(BatchSprite* batchSprite)
{
	vector<BatchSprite*>::iterator attachedBatchSpritesIt;
	for (attachedBatchSpritesIt = attachedBatchSprites.begin(); attachedBatchSpritesIt != attachedBatchSprites.end(); attachedBatchSpritesIt++)
	{
		if (*attachedBatchSpritesIt == batchSprite)
		{
			attachedBatchSprites.erase(attachedBatchSpritesIt);
			return;
		}
	}
}