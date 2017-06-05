#pragma once

#define GENERIC_NUM_INPUTS 6
#define CAMERA_NUM_INPUTS 8
#define INPUT_NUM_ALTERNATIVE 1

enum GENERIC_CONTROLS
{
	POINTER_DOWN = 0,
	POINTER_UP,
	POINTER_MOVE,
	POINTER_CANCEL,
	POINTER_LOCATION_X,
	POINTER_LOCATION_Y
};

enum CAMERA_CONTROL {
	CAMERA_MOVE_FORWARD = 6,
	CAMERA_MOVE_BACKWARD,
	CAMERA_MOVE_LEFT,
	CAMERA_MOVE_RIGHT,
	CAMERA_TURN_LEFT,
	CAMERA_TURN_RIGHT,
	CAMERA_ZOOM_IN,
	CAMERA_ZOOM_OUT
};