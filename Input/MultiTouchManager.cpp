#include "OrbsPluginPCH.h"
#include "MultiTouchManager.h"
#include "GameManager.h"

/////////////////////////////MultiTouchManager/////////////////////////////
MultiTouchManager MultiTouchManager::instance;

void MultiTouchManager::replaceTouches(const vector<hkvVec2> &touches)
{
	int previousTouchCount = (int) currentTouches.size();
	int currentTouchCount = (int) touches.size();

	if (previousTouchCount == 3 && currentTouchCount == 3) //rotate the scene by moving 3 fingers on the screen
	{
		hkvVec2 oldBarycenter = (currentTouches[0] + currentTouches[1] + currentTouches[2]) / 3.0f;
		hkvVec2 newBarycenter = (touches[0] + touches[1] + touches[2]) / 3.0f;

		//old local coordinates
		hkvVec2 oldTouch0Local = currentTouches[0] - oldBarycenter;
		hkvVec2 oldTouch1Local = currentTouches[1] - oldBarycenter;
		hkvVec2 oldTouch2Local = currentTouches[2] - oldBarycenter;
		//new local coordinates
		hkvVec2 newTouch0Local = touches[0] - newBarycenter;
		hkvVec2 newTouch1Local = touches[1] - newBarycenter;
		hkvVec2 newTouch2Local = touches[2] - newBarycenter;

		float dAngleTouch0 = atan2(oldTouch0Local.y, oldTouch0Local.x) - atan2(newTouch0Local.y, newTouch0Local.x);
		float dAngleTouch1 = atan2(oldTouch1Local.y, oldTouch1Local.x) - atan2(newTouch1Local.y, newTouch1Local.x);
		float dAngleTouch2 = atan2(oldTouch2Local.y, oldTouch2Local.x) - atan2(newTouch2Local.y, newTouch2Local.x);

		float dAngle = dAngleTouch0 + dAngleTouch1 + dAngleTouch2;

		for (int callbackIndex = 0; callbackIndex != callbacks.size(); callbackIndex++)
		{
			MultiTouchCallback* callback = callbacks[callbackIndex];
			callback->onRotationChanged(-dAngle);
		}
	}
	else if (previousTouchCount == 2 && currentTouchCount == 2) //same number of touches (2) on 2 consecutive frames
	{
		hkvVec2 oldDiff = (currentTouches[1] - currentTouches[0]);
		hkvVec2 newDiff = (touches[1] - touches[0]);
		hkvVec2 pinchCenter = 0.5f * (touches[0] + touches[1]);

		//pinch
		float oldDistance = oldDiff.getLength();
		float newDistance = newDiff.getLength();

		//rotation
		float oldAngle = atan2(oldDiff.y, oldDiff.x);
		float newAngle = atan2(newDiff.y, newDiff.x);

		for (int callbackIndex = 0; callbackIndex != callbacks.size(); callbackIndex++)
		{
			MultiTouchCallback* callback = callbacks[callbackIndex];
			callback->onPinchChanged(pinchCenter, newDistance - oldDistance);
		}
	}
	else if (previousTouchCount == 0 && currentTouchCount == 1) //on press
	{
		this->onPressTouchLocation = touches[0];

		if (SceneManager::sharedInstance().isCurrentSceneLevel()) //level
		{
			GameDialogManager* gameDialogManager = GameDialogManager::sharedInstance();
			if (gameDialogManager->clickContainedInGUI(onPressTouchLocation)) //we clicked on a GUI element
			{
				gameDialogManager->processPointerEvent(onPressTouchLocation, POINTER_DOWN);
				onPressHandledByGUI = true;
				onPressHandledByGame = false;
			}
			else
			{
				onPressHandledByGUI = false;
				onPressHandledByGame = GameManager::GlobalManager().processPointerEvent(onPressTouchLocation, POINTER_DOWN);
			}
			onMoveActive = false;
		}
		else
		{
			MenuManager* menuManager = MenuManager::sharedInstance();
			if (menuManager->clickContainedInGUI(onPressTouchLocation)) //always return true
			{
				menuManager->processPointerEvent(onPressTouchLocation, POINTER_DOWN);
				onPressHandledByGUI = true;
			}
			else
				onPressHandledByGUI = false;
			onPressHandledByGame = false;
		}
	}
	else if (previousTouchCount == 1 && currentTouchCount == 1) //on move
	{
		if (this->onPressHandledByGUI)
		{
			if (GameDialogManager::sharedInstance()->isContextActive())
				GameDialogManager::sharedInstance()->processPointerEvent(touches[0], POINTER_MOVE);
			else if (MenuManager::sharedInstance()->isContextActive())
				MenuManager::sharedInstance()->processPointerEvent(touches[0], POINTER_MOVE);
		}
		else
		{
			if (!GameManager::GlobalManager().processPointerEvent(touches[0], POINTER_MOVE))
			{
				if (!this->onMoveActive)
				{
					float distanceFromOnPressLocation = (touches[0] - onPressTouchLocation).getLength();
					if (distanceFromOnPressLocation > ON_MOVE_DELTA_THRESHOLD) //start on move
					{
						onMoveActive = true;
					}
				}
				else
				{
					hkvVec2 deltaPosition = touches[0] - currentTouches[0];
					for (int callbackIndex = 0; callbackIndex != callbacks.size(); callbackIndex++)
					{
						MultiTouchCallback* callback = callbacks[callbackIndex];
						callback->onPositionChanged(deltaPosition);
					}
				}
			}
		}
	}
	else if (previousTouchCount == 1 && currentTouchCount == 0) //on release
	{
		if (this->onPressHandledByGUI)
		{
			if (GameDialogManager::sharedInstance()->isContextActive())
				GameDialogManager::sharedInstance()->processPointerEvent(currentTouches[0], POINTER_UP);
			else if (MenuManager::sharedInstance()->isContextActive())
			{
				MenuManager::sharedInstance()->processPointerEvent(currentTouches[0], POINTER_UP);
			}
		}
		else
		{
			if (this->onPressHandledByGame)
			{
				//if (GameManager::GlobalManager().processPointerEvent(currentTouches[0], POINTER_UP)) //something has been picked
				//	GameManager::GlobalManager().OnPickSphere();
				//else
				//	GameManager::GlobalManager().OnPickNothing(false);
				if (!GameManager::GlobalManager().processPointerEvent(currentTouches[0], POINTER_UP)) //something has been picked
					GameManager::GlobalManager().OnPickNothing(false);
			}
			else
			{
				if (!onMoveActive)
					GameManager::GlobalManager().OnPickNothing(true);
			}
		}
	}

	if (currentTouchCount == 0 || currentTouchCount >= 2)
	{
		this->onPressHandledByGame = false;
		if (this->onPressHandledByGUI)
		{
			if (GameDialogManager::sharedInstance()->isContextActive())
				GameDialogManager::sharedInstance()->processPointerEvent(currentTouches[0], POINTER_CANCEL);
			else if (MenuManager::sharedInstance()->isContextActive())
				MenuManager::sharedInstance()->processPointerEvent(currentTouches[0], POINTER_CANCEL);
			this->onPressHandledByGUI = false;
		}
	}

	//finally clear old touches and replace them with new ones
	currentTouches.clear(); //clear previous touches
	int touchesCapacity = (int) currentTouches.capacity();
	this->currentTouches.reserve(MAX_INPUT_TOUCHES);
	int newTouchesCapacity = (int) currentTouches.capacity();
	if (touches.size() > 0)
		this->currentTouches.insert(currentTouches.begin(), touches.begin(), touches.end());
}

void MultiTouchManager::registerCallback(MultiTouchCallback* callback)
{
	this->callbacks.push_back(callback);
}

void MultiTouchManager::unregisterCallback(MultiTouchCallback* callback)
{
	vector<MultiTouchCallback*>::iterator callbacksIt;
	for (callbacksIt = callbacks.begin(); callbacksIt != callbacks.end(); callbacksIt++)
	{
		if (*callbacksIt == callback)
		{
			callbacks.erase(callbacksIt);
			return;
		}
	}
}