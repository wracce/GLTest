#include "_OpenGL/camera.h"

SCamera* camera;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

typedef struct {
	float x, y, z;
} TCell;

typedef struct {
	float r, g, b;
} TColor;

#define mapW 100
#define mapH 100
TCell map[mapW][mapH];
TColor mapCol[mapW][mapH];

GLuint mapInd[mapW - 1][mapH - 1][6];
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);

BOOL IsCordInMap(float x, float y)
{
	return (x >= 0)&& (x < mapW)&& (y >= 0)&& (y < mapH);
}

void Map_Create_Hill(int posX, int posY, int rad, int height)
{
	for (int i = posX-rad; i <= posX+rad ; i++)
		for (int j = posY-rad; j <= posY+rad; j++)
			if (IsCordInMap(i,j))
			{
				float len = sqrt(pow(posX - 1, 2) + pow(posY - j, 2));
				if (len < rad)
				{
					len = len / rad * M_PI_2;
					map[i][j].z += cos(len) * height;
				}
			}
}

float Map_GetHeight(float x, float y)
{
	if (!IsCordInMap(x, y)) return 0;
	int cX = (int)x;
	int cY = (int)y;
	float h1 = ((1 - (x - cX)) * map[cX][cY].z + (x - cX) * map[cX + 1][cY].z);
	float h2 = ((1 - (x - cX)) * map[cX][cY + 1].z + (x - cX) * map[cX + 1][cY + 1].z);
	return (1 - (y - cY)) * h1 + (y - cY) * h2;
}


void Map_Init()
{
	for (int i = 0; i < mapW; i++)
		for (int j = 0; j < mapH; j++)
		{
			float dc = (rand() % 20) * 0.01;
			mapCol[i][j].r = 0.31 + dc;
			mapCol[i][j].g = 0.5 + dc;
			mapCol[i][j].b = 0.13 + dc;

			map[i][j].x = i;
			map[i][j].y = j;
			map[i][j].z = (rand() % 10) * 0.05;

		}

	for (int i = 0; i < mapW-1; i++)
	{
		int pos = i * mapH;
		for (int j = 0; j < mapH-1; j++)
		{
			mapInd[i][j][0] = pos;
			mapInd[i][j][1] = pos+1;
			mapInd[i][j][2] = pos+1+mapH;

			mapInd[i][j][3] = pos + 1 + mapH;
			mapInd[i][j][4] = pos + mapH;
			mapInd[i][j][5] = pos;
			pos++;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		Map_Create_Hill(rand() % mapW, rand() % mapH, rand() % 50, rand() % 10);
	}

	for (int i = 0; i < mapW-1; i++)
		for (int j = 0; j < mapH - 1; j++)
		{
			float dc = (map[i + 1][j + 1].z - map[i][j].z) * 0.2;
			mapCol[i][j].r += dc;
			mapCol[i][j].g += dc;
			mapCol[i][j].b += dc;
		}
}


void Map_Show()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, map);
	glColorPointer(3, GL_FLOAT, 0, mapCol);
	glDrawElements(GL_TRIANGLES, mapIndCnt, GL_UNSIGNED_INT, mapInd);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void WndResize(int x, int y) {
	glViewport(0, 0, x, y);
	float k = x / (float)y;
	float sz = 0.1;
	glLoadIdentity();
	glFrustum(-k * sz, k * sz, -sz, sz, sz * 2, 100);
}

void Player_Move()
{
	camera->Camera_MoveDirection(GetKeyState('W') < 0? 1: (GetKeyState('S') < 0 ? -1 : 0), GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0),0.1);
	camera->Camera_AutoMoveByMouse(400, 400, 0.2);
	camera->z = Map_GetHeight(camera->x, camera->y) + 1.7;
}

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
		600,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	RECT rct;
	GetClientRect(hwnd, &rct);
	WndResize(rct.right, rct.bottom);
	camera = new SCamera(0,0,1.7,0,0);
	Map_Init();
	glEnable(GL_DEPTH_TEST);

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

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPushMatrix();
			if (GetForegroundWindow() == hwnd)
				Player_Move();
			camera->Camera_Apply();
			Map_Show();

			glEnd();

			glPopMatrix();

			SwapBuffers(hDC);

			theta += 1.0f;
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

	case WM_DESTROY:
		return 0;
	case WM_SIZE:
		WndResize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SETCURSOR:
		ShowCursor(false);
		break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
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
