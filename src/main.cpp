#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <time.h>
#define _USE_MATH_DEFINES
#pragma comment(lib, "opengl32.lib")

int width = 1280;
int height = 720;
float koef;
float gravityBall = 0.002;
float gravity = 0.004;

void DrawCircle(int cnt)
{
	float x, y;
	float da = M_PI*2.0/cnt;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0, 0);
	for (int i = 0; i <= cnt; i++)
	{
		x = sin(da * i);
		y = cos(da * i);
		glVertex2f(x, y);
	}

	glEnd();
}

void Quad(float x, float y, float dx, float dy)
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	glVertex2f(x + dx, y);
	glVertex2f(x + dx, y+dy);
	glVertex2f(x, y+dy);
	glEnd();


}

typedef struct {
	float x, y;
	float dx, dy;
	float r;
} Tball;

Tball ball;
Tball player[2];
float netHeight = -0.2;

BOOL IsCross(float x1, float y1, float r, float x2, float y2)
{
	return pow(x1 - x2, 2) + pow(y1 - y2, 2) < r * r;
}

void Mirror(Tball* obj, float x, float y, float speed)
{
	float objVec = atan2(obj->dx, obj->dy);
	float crossVec = atan2(obj->x - x, obj->y - y);

	float resVec = speed ==0?M_PI - objVec + crossVec * 2:crossVec;
	speed = speed == 0 ? sqrt(pow(obj->dx, 2) + pow(obj->dy, 2)) :speed;

	obj->dx = sin(resVec) * speed;
	obj->dy = cos(resVec) * speed;

}

void TBall_Init(Tball* obj, float x1, float y1, float dx1, float dy1, float r1) 
{
	obj->x = x1;
	obj->y = y1;
	obj->dx = dx1;
	obj->dy = dy1;
	obj->r = r1;
}


void Tball_Show(Tball obj)
{
	glPushMatrix();
		glTranslatef(obj.x, obj.y, 0);
		glScalef(obj.r, obj.r, 1);
		DrawCircle(20);
	glPopMatrix();
}

void Player_Move(Tball *obj, char left, char right, char jump, float wl1, float wl2)
{
	static float speed = 0.05;

	if (GetKeyState(left) < 0) obj->x -= speed;
	else if (GetKeyState(right)<0) obj->x += speed;

	if (obj->x - obj->r < wl1) obj->x = wl1 + obj->r;
	if (obj->x + obj->r > wl2) obj->x = wl2 - obj->r;

	if ((GetKeyState(jump) < 0) && (obj->y < -0.99 + obj->r))
		obj->dy = speed * 1.4;
	obj->y += obj->dy;
	obj->dy -= gravity;

	if (obj->y - obj->r < -1)
	{
		obj->y = -1 + obj->r;
		obj->dy = 0;
	}

	if (IsCross(obj->x, obj->y, obj->r + ball.r, ball.x, ball.y))
	{
		Mirror(&ball, obj->x, obj->y, 0.07);
		ball.dy += 0.01;
	}
}

void Reflect(float* da, float* a, BOOL cond, float wall)
{
	if (!cond) return;
	*da *= -0.85;
	*a = wall;

}

void TBall_Move(Tball* obj)
{
	obj->x += obj->dx;
	obj->y += obj->dy;
	
	Reflect(&obj->dy, &obj->y, (obj->y < obj->r - 1), obj->r - 1);
	Reflect(&obj->dy, &obj->y, (obj->y > 1-obj->r),1-obj->r);
	obj->dy -= gravityBall;

	Reflect(&obj->dx, &obj->x, (obj->x < obj->r - koef), obj->r - koef);
	Reflect(&obj->dx, &obj->x, (obj->x > koef-obj->r), koef-obj->r);
	
	if (obj->y < netHeight)
	{
		if(obj->x>0)
			Reflect(&obj->dx, &obj->x, (obj->x < +obj->r), +obj->r);
		else
			Reflect(&obj->dx, &obj->x, (obj->x > -obj->r), -obj->r);
	} 
	else 
	{
		if (IsCross(obj->x, obj->y, obj->r, 0, netHeight))
			Mirror(obj, 0, netHeight,0);
	}
}

void Game_Init()
{
	TBall_Init(&ball,0.1,0.05,0.0,0.0,0.2);
	TBall_Init(player+0, -1.0, 0, 0.0, 0.0, 0.2);
	TBall_Init(player+1, 1, 0.05, 0.0, 0.0, 0.2);
}

void Game_Show()
{
	glColor3f(0.83, 0.81, 0.67);
	Quad(-koef, -1, koef * 2, 1);
	glColor3f(0.21, 0.67, 0.88);
	Quad(-koef, 0, koef * 2, 1);
	glColor3f(0.66, 0.85, 1);
	Quad(-koef, 0.2, koef * 2, 1);

	glColor3f(0.23, 0.29, 0.79);
	Tball_Show(ball);

	glColor3f(0.8, 0, 0);
	Tball_Show(player[0]);

	glColor3f(0, 0.5, 0);
	Tball_Show(player[1]);


	glColor3f(0,0,0);
	glLineWidth(8);
	glBegin(GL_LINES);
	glVertex2f(0, netHeight);
	glVertex2f(0, -1);
	glEnd();
}
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		"GLSample",
		"OpenGL Sample",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);
	koef = (float)width / height;
	glScalef(1 / koef, 1, 1);
	Game_Init();

	/* program main loop */
	while (!bQuit)
	{
		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			/* OpenGL animation code goes here */

			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			
			TBall_Move(&ball);
			Player_Move(player + 0, 'A', 'D', 'W', -koef, 0);
			Player_Move(player + 1, VK_LEFT, VK_RIGHT, VK_UP, 0, koef);

			Game_Show();

			SwapBuffers(hDC);

			Sleep(1);
		}
	}

	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);

	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
	break;


	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}

