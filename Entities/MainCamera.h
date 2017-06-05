#pragma once

#define CAMERA_LINEAR_SPEED 800.0f //800 pixels/sec
#define CAMERA_ANGULAR_SPEED 120.0f //120deg/sec (for pc only)

#define CAMERA_MIN_ALTITUDE 1200
#define CAMERA_MAX_ALTITUDE 5000
#define CAMERA_MIN_PITCH 60
#define CAMERA_MAX_PITCH 90

#define CAMERA_BOUNDARY_TOLERANCE 400.0f //we can slide to more than 200 pixels beyond boundaries

#define CAMERA_AUTOMOVE_SPEED 1000.0f

class MainCamera : public VisBaseEntity_cl, public MultiTouchCallback
{
public:
	MainCamera(void);
	~MainCamera(void);

	virtual void InitFunction();
	virtual void ThinkFunction();
	virtual void Serialize( VArchive &ar );

	V_DECLARE_SERIAL_DLLEXP( MainCamera, DECLSPEC_DLLEXPORT ); 
	IMPLEMENT_OBJ_CLASS(MainCamera);

	void initCamera();

	void moveToPositionForLookAtAndAltitude(hkvVec3 lookAtPosition, float altitude, float yaw);
	void updatePositionOnAutoMove(float dt);

	void moveForward(float dt);
	void moveBackward(float dt);
	void moveLeft(float dt);
	void moveRight(float dt);

	void turnLeft(float dt);
	void turnRight(float dt);

	void IncAltitude(float deltaAltitude);
	void IncPitch(float deltaPitch);
	float getPitchForAltitude(float altitude);
	float getAltitudeForPitch(float pitch);

	void toggleCameraBlocked(){this->cameraBlocked = !this->cameraBlocked;};
	void blockCamera(){this->cameraBlocked = true;};
	void unblockCamera(){this->cameraBlocked = false;};
	
	void updateCameraBoundaries();

	float getZoomPercentage();

	virtual void onPinchChanged(hkvVec2 pinchCenter, float deltaPinch);
	virtual void onRotationChanged(float deltaRotationRad);
	virtual void onPositionChanged(hkvVec2 deltaPosition);

protected:
	bool cameraBlocked;

	//automatically move the camera to a certain position/orientation
	bool autoMoving;
	float autoMoveDuration;
	float autoMoveElapsedTime;
	hkvVec3 startPosition;
	hkvVec3 startOrientation;
	hkvVec3 finalPosition;
	hkvVec3 finalOrientation; //final orientation in Euler angles

	float cameraMinAltitude;
	float cameraMaxAltitude;
	VRectanglef cameraBoundaries;
};

