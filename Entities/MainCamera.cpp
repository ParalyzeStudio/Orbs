#include "OrbsPluginPCH.h"
#include "GameManager.h"

V_IMPLEMENT_SERIAL( MainCamera, VisBaseEntity_cl, 0, &g_myComponentModule);

MainCamera::MainCamera() :
	cameraBlocked(false), 
	autoMoving(false),
	startPosition(hkvVec3(0,0,0)),
	startOrientation(hkvVec3(0,0,0)),
	finalPosition(hkvVec3(0,0,0)),
	finalOrientation(hkvVec3(0,0,0)),
	autoMoveDuration(0),
	autoMoveElapsedTime(0),
	cameraMinAltitude(0),
	cameraMaxAltitude(0),
	cameraBoundaries(VRectanglef(hkvVec2(0,0), hkvVec2(0,0)))

{

}

MainCamera::~MainCamera()
{

}

void MainCamera::InitFunction()
{
	
}

void MainCamera::ThinkFunction()
{
	float dt = Vision::GetTimer()->GetTimeDifference();
	this->updatePositionOnAutoMove(dt);
}

void MainCamera::Serialize( VArchive &ar )
{
	//Store base properties like current position and rotation
	VisBaseEntity_cl::Serialize(ar);

	if (ar.IsLoading())
	{
		//Load data
		char iVersion;
		ar >> iVersion;    VASSERT(iVersion==0);

	} else
	{
		//Save data
		ar << (char)0; // version for entity loading
	}

	initCamera();
}

void MainCamera::initCamera()
{
	this->SetUseEulerAngles(true);
	float currentAltitude = this->GetPosition().z;
	float altitudePercentage = (currentAltitude - CAMERA_MIN_ALTITUDE) / (CAMERA_MAX_ALTITUDE - CAMERA_MIN_ALTITUDE);
	float currentPitch = altitudePercentage * (CAMERA_MAX_PITCH - CAMERA_MIN_PITCH) + CAMERA_MIN_PITCH; 
	this->SetOrientation(this->GetOrientation().x, currentPitch, this->GetOrientation().z);
	this->cameraBlocked = true;
	this->cameraMinAltitude = CAMERA_MIN_ALTITUDE;
	this->cameraMaxAltitude = CAMERA_MAX_ALTITUDE;

	//standard initialization
	this->autoMoving = false;
	this->startPosition = hkvVec3(0,0,0);
	this->startOrientation = hkvVec3(0,0,0);
	this->finalPosition = hkvVec3(0,0,0);
	this->finalOrientation = hkvVec3(0,0,0);
	this->autoMoveDuration = 0;
	this->autoMoveElapsedTime = 0;
	this->cameraBoundaries = VRectanglef(hkvVec2(0,0), hkvVec2(0,0));
}

void MainCamera::moveToPositionForLookAtAndAltitude(hkvVec3 lookAtPosition, float altitude, float yaw)
{
	autoMoving = true;

	this->startPosition = this->GetPosition();
	this->startOrientation = this->GetOrientation();

	float pitch = getPitchForAltitude(altitude);

	float deltaYaw = yaw - this->GetOrientation().x;
	float deltaPitch = pitch - this->GetOrientation().y;

	finalOrientation = this->GetOrientation();
	finalOrientation.x += deltaYaw;
	finalOrientation.y += deltaPitch;

	hkvMat3 rotationMatrix = hkvMat3(hkvNoInitialization);
	rotationMatrix.setFromEulerAngles(finalOrientation.z, finalOrientation.y, finalOrientation.x);
	hkvVec3 lookAtDirection = rotationMatrix.transformDirection(hkvVec3(1, 0, 0));
	float kParameter = (altitude - lookAtPosition.z) / lookAtDirection.z;
	finalPosition.x = lookAtPosition.x + kParameter * lookAtDirection.x;
	finalPosition.y = lookAtPosition.y + kParameter * lookAtDirection.y;
	finalPosition.z = altitude;

	this->autoMoveElapsedTime = 0;
	this->autoMoveDuration = (finalPosition - startPosition).getLength() / CAMERA_AUTOMOVE_SPEED;
}

void MainCamera::updatePositionOnAutoMove(float dt)
{
	if (!autoMoving)
		return;

	this->autoMoveElapsedTime += dt;

	if (autoMoveElapsedTime > autoMoveDuration)
	{
		this->SetPosition(finalPosition);
		this->SetOrientation(finalOrientation);
		this->autoMoving = false;
	}
	else
	{
		//set new orientation
		float deltaYaw = (finalOrientation.x - startOrientation.x) * dt / autoMoveDuration;
		float deltaPitch = (finalOrientation.y - startOrientation.y) * dt / autoMoveDuration;

		hkvVec3 orientation = this->GetOrientation();
		orientation.x += deltaYaw;
		orientation.y += deltaPitch;
		this->SetOrientation(orientation);

		//set new position
		hkvVec3 deltaPosition = (finalPosition - startPosition) * dt / autoMoveDuration;
		this->SetPosition(this->GetPosition() + deltaPosition);
	}

	ScreenMaskPanel* selectionHaloPanel = GameDialogManager::sharedInstance()->getHUD()->getSelectionHaloPanel();
	if (selectionHaloPanel)
	{
		SelectionHalo* selectionHalo = (SelectionHalo*) selectionHaloPanel->findElementById(ID_SELECTION_HALO);
		selectionHalo->updateSize();
	}

	GameDialogManager::sharedInstance()->getHUD()->updateSelectionHaloPanelOpacity();
}

void MainCamera::moveForward(float dt)
{
	if (autoMoving)
		return;

	hkvVec3 cameraOrientation = this->GetOrientation();
	float eulerX = cameraOrientation.x;
	hkvVec3 forwardDirector = hkvVec3(cos(GeometryUtils::degreesToRadians(eulerX)), sin(GeometryUtils::degreesToRadians(eulerX)), 0);
	forwardDirector.operator*=(CAMERA_LINEAR_SPEED * dt); // multiply the normalized director vector by the camera linear speed
	
	hkvVec3 cameraPosition = this->GetPosition();
	cameraPosition.operator+=(forwardDirector);
	this->SetPosition(cameraPosition);
}

void MainCamera::moveBackward(float dt)
{
	if (autoMoving)
		return;

	hkvVec3 cameraOrientation = this->GetOrientation();
	float eulerX = cameraOrientation.x;
	hkvVec3 forwardDirector = hkvVec3(cos(GeometryUtils::degreesToRadians(eulerX)), sin(GeometryUtils::degreesToRadians(eulerX)), 0);
	forwardDirector.operator*=(CAMERA_LINEAR_SPEED * dt); // multiply the normalized director vector by the camera linear speed

	hkvVec3 cameraPosition = this->GetPosition();
	cameraPosition.operator-=(forwardDirector);
	this->SetPosition(cameraPosition);
}

void MainCamera::moveLeft(float dt)
{
	if (autoMoving)
		return;

	hkvVec3 cameraOrientation = this->GetOrientation();
	float eulerX = cameraOrientation.x;
	hkvVec3 forwardDirector = hkvVec3(sin(GeometryUtils::degreesToRadians(eulerX)), -cos(GeometryUtils::degreesToRadians(eulerX)), 0);
	forwardDirector.operator*=(CAMERA_LINEAR_SPEED * dt); // multiply the normalized director vector by the camera linear speed

	hkvVec3 cameraPosition = this->GetPosition();
	cameraPosition.operator-=(forwardDirector);
	this->SetPosition(cameraPosition);
}

void MainCamera::moveRight(float dt)
{
	if (autoMoving)
		return;

	hkvVec3 cameraOrientation = this->GetOrientation();
	float eulerX = cameraOrientation.x;
	hkvVec3 forwardDirector = hkvVec3(sin(GeometryUtils::degreesToRadians(eulerX)), -cos(GeometryUtils::degreesToRadians(eulerX)), 0);
	forwardDirector.operator*=(CAMERA_LINEAR_SPEED * dt); // multiply the normalized director vector by the camera linear speed

	hkvVec3 cameraPosition = this->GetPosition();
	cameraPosition.operator+=(forwardDirector);
	this->SetPosition(cameraPosition);
}

void MainCamera::turnLeft(float dt)
{
	if (autoMoving)
		return;

	float angle = CAMERA_ANGULAR_SPEED * dt;
	hkvVec3 cameraOrientation = this->GetOrientation();
	this->SetOrientation(cameraOrientation.x + angle,
						 cameraOrientation.y,
						 cameraOrientation.z);
}

void MainCamera::turnRight(float dt)
{	
	if (autoMoving)
		return;

	float angle = CAMERA_ANGULAR_SPEED * dt;
	hkvVec3 cameraOrientation = this->GetOrientation();
	this->SetOrientation(cameraOrientation.x - angle,
						 cameraOrientation.y,
						 cameraOrientation.z);
}

void MainCamera::IncAltitude(float deltaAltitude)
{
	float currentAltitude = this->GetPosition().z;
	currentAltitude += deltaAltitude;
	if (currentAltitude < CAMERA_MIN_ALTITUDE)
		currentAltitude = CAMERA_MIN_ALTITUDE;
	else if (currentAltitude > CAMERA_MAX_ALTITUDE)
		currentAltitude = CAMERA_MAX_ALTITUDE;

	hkvVec3 cameraPosition = this->GetPosition();
	this->SetPosition(cameraPosition.x, cameraPosition.y, currentAltitude);
}

void MainCamera::IncPitch(float deltaPitch)
{
	float currentPitch = this->GetOrientation().y;
	currentPitch += deltaPitch;
	if (currentPitch < CAMERA_MIN_PITCH)
		currentPitch = CAMERA_MIN_PITCH;
	else if (currentPitch > CAMERA_MAX_PITCH)
		currentPitch = CAMERA_MAX_PITCH;

	hkvVec3 cameraOrientation = this->GetOrientation();
	this->SetOrientation(cameraOrientation.x, currentPitch, cameraOrientation.z);
}

void MainCamera::updateCameraBoundaries()
{
	int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();

	vector<NodeSphere*> &allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	for (int sphereIndex = 0; sphereIndex != allNodeSpheres.size(); sphereIndex++)
	{
		NodeSphere* sphere = allNodeSpheres[sphereIndex];

		int sphereVisibilityStatus = sphere->getVisibilityStatus();
		if (sphereVisibilityStatus == VISIBILITY_STATUS_UNDISCOVERED_FAR_AWAY)
			continue;
		else if (currentLevelNumber == 1)
		{
			int spheresCountInsideCameraBoundaries = TutorialsHandler::sharedInstance().getSphereCountInsideCameraBoundaries();
			if ((sphereIndex + 1) > spheresCountInsideCameraBoundaries) //dont reveal yet all tutorial spheres
				continue;
		}

		float sphereAreaMinX = sphere->GetPosition().x - 200.0f;
		float sphereAreaMaxX = sphere->GetPosition().x + 200.0f;
		float sphereAreaMinY = sphere->GetPosition().y - 200.0f;
		float sphereAreaMaxY = sphere->GetPosition().y + 200.0f;

		if (sphereAreaMinX < cameraBoundaries.m_vMin.x)
			cameraBoundaries.m_vMin.x = sphereAreaMinX;
		if (sphereAreaMaxX > cameraBoundaries.m_vMax.x)
			cameraBoundaries.m_vMax.x = sphereAreaMaxX;
		if (sphereAreaMinY < cameraBoundaries.m_vMin.y)
			cameraBoundaries.m_vMin.y = sphereAreaMinY;
		if (sphereAreaMaxY > cameraBoundaries.m_vMax.y)
			cameraBoundaries.m_vMax.y = sphereAreaMaxY;
	}
}

float MainCamera::getZoomPercentage()
{
	return (this->GetPosition().z - CAMERA_MIN_ALTITUDE) / (CAMERA_MAX_ALTITUDE - CAMERA_MIN_ALTITUDE);
}

void MainCamera::onPinchChanged(hkvVec2 pinchCenter, float deltaPinch)
{
	if (!this->cameraBlocked && !autoMoving)
	{
		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float pinchVariationPercentage = deltaPinch / screenSize.x;

		float amplitudeFactor = 3.0f;

		float deltaAltitude = -pinchVariationPercentage * (CAMERA_MAX_ALTITUDE - CAMERA_MIN_ALTITUDE);
		float deltaPitch = -pinchVariationPercentage * (CAMERA_MAX_PITCH - CAMERA_MIN_PITCH);

		this->IncAltitude(amplitudeFactor * deltaAltitude);
		this->IncPitch(amplitudeFactor * deltaPitch);

		ScreenMaskPanel* selectionHaloPanel = GameDialogManager::sharedInstance()->getHUD()->getSelectionHaloPanel();
		if (selectionHaloPanel)
		{
			SelectionHalo* selectionHalo = (SelectionHalo*) selectionHaloPanel->findElementById(ID_SELECTION_HALO);
			selectionHalo->updateSize();
		}
	}
}

float MainCamera::getPitchForAltitude(float altitude)
{
	float altitudePercentage = (altitude - CAMERA_MIN_ALTITUDE) / (CAMERA_MAX_ALTITUDE - CAMERA_MIN_ALTITUDE);
	
	return altitudePercentage * (CAMERA_MAX_PITCH - CAMERA_MIN_PITCH) + CAMERA_MIN_PITCH;
}

float MainCamera::getAltitudeForPitch(float pitch)
{
	float pitchPercentage = (pitch - CAMERA_MIN_PITCH) / (CAMERA_MAX_PITCH - CAMERA_MIN_PITCH);
	
	return pitchPercentage * (CAMERA_MAX_ALTITUDE - CAMERA_MIN_ALTITUDE) + CAMERA_MIN_ALTITUDE;
}

void MainCamera::onRotationChanged(float deltaRotationRad)
{
	if (!cameraBlocked && !autoMoving)
	{
		float yaw = this->GetOrientation().x;
		yaw -= GeometryUtils::radiansToDegrees(deltaRotationRad);
		this->SetOrientation(yaw, this->GetOrientation().y, this->GetOrientation().z);
	}
}

void MainCamera::onPositionChanged(hkvVec2 deltaPosition)
{
	if (!this->cameraBlocked && !autoMoving)
	{
		float yawRad = GeometryUtils::degreesToRadians(this->GetOrientation().x);
		hkvVec2 deltaPositionNorm = deltaPosition;
		float deltaPositionLength = deltaPosition.getLength();
		deltaPositionNorm.normalize();
		float deltaPositionPolarAngle = atan2(deltaPosition.y, deltaPosition.x);
		float directorPolarAngle = (float) (yawRad - M_PI_2 - deltaPositionPolarAngle);
		hkvVec3 director = -deltaPositionLength * hkvVec3(cos(directorPolarAngle), sin(directorPolarAngle), 0);

		float amplitudeFactor = 7.0f;
		hkvVec2 slideDelta = amplitudeFactor * hkvVec2(director.x, director.y);
		hkvVec2 tmp2DPosition = hkvVec2(this->GetPosition().x, this->GetPosition().y) + slideDelta;
		//hkvVec3 newPosition = this->GetPosition() + amplitudeFactor * director;

		//We are out of camera boundaries, slow down the slide speed
		hkvVec2 tmpPositionToBoundariesCenter = tmp2DPosition - cameraBoundaries.GetCenter();
		float distanceToCenterX = abs(tmpPositionToBoundariesCenter.x);
		float distanceToCenterY = abs(tmpPositionToBoundariesCenter.y);

		float distanceToBoundaries = 0;
		float distanceToXBoundary = distanceToCenterX - 0.5f * cameraBoundaries.GetSizeX();
		float distanceToYBoundary = distanceToCenterY - 0.5f * cameraBoundaries.GetSizeY();
		if (distanceToXBoundary > 0 || distanceToYBoundary > 0)
		{
			if (distanceToXBoundary > 0 && distanceToYBoundary > 0)
			{
				distanceToBoundaries = sqrt(pow(distanceToXBoundary, 2) + pow(distanceToYBoundary, 2));
			}
			else if (distanceToXBoundary > 0)
			{
				distanceToBoundaries = distanceToXBoundary;
			}
			else if (distanceToYBoundary > 0)
			{
				distanceToBoundaries = distanceToYBoundary;
			}
		}

		float slowFactor = distanceToBoundaries / CAMERA_BOUNDARY_TOLERANCE;
		if (slowFactor > 1)
			slowFactor = 1;

		tmp2DPosition -= slowFactor * slideDelta; //recalcultate the 2D position taking into account the new slow factor
		hkvVec3 newPosition = hkvVec3(tmp2DPosition.x, tmp2DPosition.y, this->GetPosition().z);
		
		this->SetPosition(newPosition);
	}
}