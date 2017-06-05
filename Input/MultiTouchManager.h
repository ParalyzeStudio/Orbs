#pragma once

#define MAX_INPUT_TOUCHES 2
#define ON_MOVE_DELTA_THRESHOLD 5.0f
#define ON_MOVE_DELTA_EPSILON 0.001f

class MultiTouchCallback;

class MultiTouchManager
{
public:
	static MultiTouchManager& sharedInstance() {return instance;};

	void replaceTouches(const vector<hkvVec2> &touchesCoords);
	void registerCallback(MultiTouchCallback* callback);
	void unregisterCallback(MultiTouchCallback* callback);

	vector<MultiTouchCallback*> &getCallbacks(){return this->callbacks;};

private:
	static MultiTouchManager instance;

	vector<MultiTouchCallback*> callbacks;
	vector<hkvVec2> currentTouches;

	hkvVec2 onPressTouchLocation;
	bool onPressHandledByGUI;
	bool onPressHandledByGame;
	bool onMoveActive;
};

class MultiTouchCallback
{
public:
	virtual void onPinchChanged(hkvVec2 pinchCenter, float deltaPinch) = 0;
	virtual void onRotationChanged(float deltaRotationRad) = 0;
	virtual void onPositionChanged(hkvVec2 deltaPosition) = 0;
};