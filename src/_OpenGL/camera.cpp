#include "camera.h"

SCamera::SCamera(float x, float y, float z, float Xrot, float Zrot)
{
		this->x = x;
		this->y = y;
		this->z = z;
		this->Xrot = Xrot;
		this->Zrot = Zrot;
}

void SCamera::Camera_Apply() {
	glRotatef(-Xrot, 1, 0, 0);
	glRotatef(-Zrot, 0, 0, 1);
	glTranslatef(-x, -y, -z);
}

void SCamera::Camera_Rotation(float xAngle, float zAngle) {
	Zrot += zAngle;
	if (Zrot < 0) Zrot += 360;
	if (Zrot > 360) Zrot -= 360;
	Xrot += xAngle;
	if (Xrot < 0) Xrot = 0;
	if (Xrot > 180) Xrot = 180;
}

void SCamera::Camera_AutoMoveByMouse(int centerX, int centerY, float speed)
{
	POINT cur;
	POINT base = { centerX,centerY };
	GetCursorPos(&cur);
	Camera_Rotation((base.y - cur.y) * speed, (base.x - cur.x) * speed);
	SetCursorPos(base.x, base.y);
}

void SCamera::Camera_MoveDirection(int forwardMove, int rightMove, float speed)
{
	float ugol = -Zrot / 180 * M_PI;

	if (forwardMove > 0)
		ugol += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);
	if (forwardMove < 0)
		ugol += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));
	if (forwardMove == 0)
	{
		ugol += rightMove > 0 ? M_PI_2 : -M_PI_2;
		if (rightMove == 0) speed = 0;
	}
	if (speed != 0)
	{
		x += sin(ugol) * speed;
		y += cos(ugol) * speed;
	}

}
