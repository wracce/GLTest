#pragma once
#define _USE_MATH_DEFINES
#include <windows.h>
#include <gl/gl.h>
#include <math.h>

#pragma comment(lib, "opengl32.lib")
class SCamera {
public:
	float x, y, z;
	float Xrot, Zrot;
	SCamera(float x, float y, float z, float Xrot, float Zrot);
	void Camera_Apply();
	void Camera_Rotation(float xAngle, float zAngle);
	void Camera_AutoMoveByMouse(int centerX, int centerY, float speed);
	void Camera_MoveDirection(int forwardMove, int rightMove, float speed);
};