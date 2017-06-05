#include "OrbsPluginPCH.h"
#include "Bridge.h"
#include "GameManager.h"

///////////////////////////////////////Bridge///////////////////////////////////////
Bridge::~Bridge()
{
	this->sharedVisibilityObject = NULL;
}

void Bridge::init(NodeSphere* startSphere, NodeSphere* endSphere, bool completed)
{
	this->startSphere = startSphere;
	this->endSphere = endSphere;
	hkvVec3 startPoint = startSphere->GetPosition();
	hkvVec3 endPoint = endSphere->GetPosition();

	this->position = 0.5f * (startPoint + endPoint);
	this->maxLength = (endPoint - startPoint).getLength();
	this->length = completed ? maxLength : 0.0f;

	this->size = hkvVec2(length, 2 * BRIDGE_DEFAULT_RADIUS);

	float yaw = atan2(endPoint.y - startPoint.y, endPoint.x - startPoint.x);

	this->rotationMatrix = hkvMat3(); //identity
	this->rotationMatrix.setRotationMatrixZ(GeometryUtils::radiansToDegrees(yaw));

	initVisibilityObject();
}

void Bridge::init(NodeSphere* startSphere, NodeSphere* endSphere)
{
	Bridge::init(startSphere, endSphere, true);
}

void Bridge::initVisibilityObject()
{
	//////init the visibility object for the bridge itself
	m_visibilityObject = new BatchSpriteVisibilityObject(VISTESTFLAGS_ACTIVE | VISTESTFLAGS_FRUSTUMTEST);
	m_visibilityObject->SetActivate(TRUE);
	m_visibilityObject->SetVisibleBitmask(0xffffff);
	hkvAlignedBBox visibilityObjectBoundingBox;
	visibilityObjectBoundingBox.setCenterAndSize(hkvVec3(0,0,0), hkvVec3(0.5f * this->maxLength, 0.5f * this->size.y , 0.5f * this->size.y));
	visibilityObjectBoundingBox.transformFromCenter(hkvMat4(this->rotationMatrix, hkvVec3(0,0,0)));
	m_visibilityObject->SetPosition(this->position);
	m_visibilityObject->SetLocalSpaceBoundingBox(visibilityObjectBoundingBox);
	m_visibilityObject->attachBatchSprite(this); 


	//////init the visibility object shared between traveling orbs along this bridge
	float startSphereOuterRadius = startSphere->getOuterRadius();
	float endSphereOuterRadius = endSphere->getOuterRadius();
	float maxOuterRadius = max(startSphereOuterRadius, endSphereOuterRadius);
	
	sharedVisibilityObject = new BatchSpriteVisibilityObject(VISTESTFLAGS_ACTIVE | VISTESTFLAGS_FRUSTUMTEST);
	sharedVisibilityObject->SetActivate(TRUE);
	sharedVisibilityObject->SetVisibleBitmask(0xffffff);
	hkvAlignedBBox bbox;
	//extend the length of the bounding box so there is no gap between 2 bridges bounding boxes and traveling orb remains visible when switching from one bridge to another
	bbox.setCenterAndSize(hkvVec3(0,0,0), hkvVec3(0.5f * (maxLength + 4 * maxOuterRadius), maxOuterRadius, maxOuterRadius)); 
	bbox.transformFromCenter(hkvMat4(this->rotationMatrix, hkvVec3(0,0,0)));
	sharedVisibilityObject->SetPosition(this->GetPosition());
	sharedVisibilityObject->SetLocalSpaceBoundingBox(bbox);
}

bool Bridge::equals(Bridge* bridge)
{
	return this->equals(bridge->getStartSphere(), bridge->getEndSphere());
}

bool Bridge::equals(NodeSphere* startSphere, NodeSphere* endSphere)
{
	return ((startSphere == this->startSphere) && (endSphere == this->endSphere)
			||
			(startSphere == this->endSphere) && (endSphere == this->startSphere));
}

void Bridge::addOrb()
{
	this->activeOrbsCount++;
	this->totalOrbsCount++;

	int maxOrbs = this->getCost();
	if (totalOrbsCount == maxOrbs) //last orb to be added
		this->targetLength = this->maxLength;
	else
		this->targetLength += this->maxLength / maxOrbs; //add one more segment
}

void Bridge::onSegmentEnded()
{
	this->activeOrbsCount--;
	this->currentSegmentBuilt++;

	if (currentSegmentBuilt == getCost()) //last segment
	{
		this->length = maxLength;
		increaseLength(0);

		startSphere->addNeighbouringSphere(endSphere);
		endSphere->addNeighbouringSphere(startSphere);
	}
}

void Bridge::increaseLength(float deltaLength)
{
	this->length += deltaLength;
	this->size = hkvVec2(length, 2 * BRIDGE_DEFAULT_RADIUS);
	hkvVec3 bridgeDirector = this->getEndPoint() - this->getStartPoint();
	bridgeDirector.normalize();
	this->position = this->getStartPoint() + 0.5f * length * bridgeDirector;
}

int Bridge::getCost()
{
	return GeometryUtils::round(maxLength * BRIDGE_COST_FACTOR);
}

void Bridge::destroySelf()
{
	this->Release();
}

void Bridge::update(float dt)
{
	//hkvMat3 deltaRotationMatrix;
	//deltaRotationMatrix.setRotationMatrixX(50 * dt);
	//this->rotationMatrix = this->rotationMatrix.multiply(deltaRotationMatrix);

	if (activeOrbsCount > 0)
	{
		float constructionSpeed = activeOrbsCount * BRIDGE_CONSTRUCTION_SPEED;
		increaseLength(constructionSpeed * dt);

		float segmentLength = maxLength / getCost();
		if (length >= ((currentSegmentBuilt + 1) * segmentLength))
		{
			this->onSegmentEnded();
		}
	}
}

/////////////////////////PotentialBridge/////////////////////////
void PotentialBridge::faceCamera()
{	
	MainCamera* mainCamera = GameManager::GlobalManager().getMainCamera();
	float cameraYaw = GeometryUtils::degreesToRadians(mainCamera->GetOrientation().x);
	float cameraPitch = mainCamera->GetOrientation().y;

	float dx = (getEndPoint() - getStartPoint()).x;
	float dy = (getEndPoint() - getStartPoint()).y;
	float angle = atan2(dy, dx);
	
	this->rotationMatrix = hkvMat3(); //identity
	this->rotationMatrix.setRotationMatrixZ(GeometryUtils::radiansToDegrees(angle));
	
	float rollAmplitude = 90 - cameraPitch;
	float modifiedCameraYaw = (float) (cameraYaw - M_PI_2);
	float cameraRelativeAngle = cameraYaw - angle;
	if (cameraRelativeAngle < -M_PI)
		cameraRelativeAngle += 2 * M_PI;
	else if (cameraRelativeAngle > M_PI)
		cameraRelativeAngle -= 2 * M_PI;

	float distanceToPi2;
	float rollAngle;
	if (cameraRelativeAngle >= 0 && cameraRelativeAngle <= M_PI) //[0 ; pi]
	{
		distanceToPi2 = (float) abs(cameraRelativeAngle - M_PI_2); //distance to Pi/2
		rollAngle = GeometryUtils::degreesToRadians(-(cameraPitch + rollAmplitude * (float) (distanceToPi2 / M_PI_2)));
	}
	else if (cameraRelativeAngle >= -M_PI && cameraRelativeAngle <= 0) //[-pi ; 0]
	{
		distanceToPi2 = (float) abs(cameraRelativeAngle + M_PI_2); //distance to -Pi/2
		rollAngle = GeometryUtils::degreesToRadians(-(180 - (cameraPitch + rollAmplitude * (float) (distanceToPi2 / M_PI_2))));
	}

	float cosRollAngle = cos(rollAngle);
	float sinRollAngle = sin(rollAngle);
	hkvMat3 rotX = hkvMat3(1,0,0, 0,cosRollAngle,-sinRollAngle, 0,sinRollAngle,cosRollAngle);
	this->rotationMatrix = rotationMatrix.multiply(rotX);
}

void PotentialBridge::update(float dt)
{
	BatchSprite::update(dt);
}