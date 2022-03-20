#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <time.h>
#include <iostream>
#define _USE_MATH_DEFINES
#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

float k;
int width, height;

typedef struct {
	char name[20];
	float vert[8];
	BOOL hover;
} TButton ;

TButton btn[] = {
	"start", {0,0,100,0,100,30,0,30}, FALSE,
	"stop", {0,40,100,40,100,70,0,70}, FALSE ,
	"quit", {0,80,100,80,100,110,0,110}, FALSE
};

int btnCnt = sizeof(btn) / sizeof(btn[0]);

void TButton_Show(TButton btn)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	if (btn.hover) glColor3f(1, 0, 0);
	else glColor3f(0, 1, 0);
	glVertexPointer(2, GL_FLOAT, 0, btn.vert);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);

}

BOOL PointInButton(int x, int y, TButton btn)
{
	return((x > btn.vert[0]) && (x < btn.vert[4])
		&& (y > btn.vert[1]) && (y < btn.vert[5]));
}

void ShowMenu()
{
	glPushMatrix();
		glLoadIdentity();
		glOrtho(0,width,height,0,-1,1);
		for (int i = 0; i < btnCnt; i++)
		{
			TButton_Show(btn[i]);
		}
	glPopMatrix();
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
		500,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

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
			
			glPushMatrix();
			glPopMatrix();

			ShowMenu();

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
	case WM_LBUTTONDOWN:
		for (int i = 0; i < btnCnt; i++)
		{
			if (PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]))
			{
				std::cout << btn[i].name << std::endl;
				if (strcmp(btn[i].name, "quit") == 0)
					PostQuitMessage(0);
			}
		}
		break;
	case WM_MOUSEMOVE:
		for (int i = 0; i < btnCnt; i++)
		{
			btn[i].hover = PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]);
		}
		break;
	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		glViewport(0, 0, width, height);
		glLoadIdentity();
		k = width / (float)height;
		glOrtho(-1*k, k, -1, 1, -1, 1);
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

