#include <windows.h>
#include "pacman.c"
#include "resource.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <wingdi.h>
#include <unistd.h>
#include <ctype.h>

#define IDT_TIMER1 101
#define REFRESH_RATE 50
const char g_szClassName[] = "myWindowClass";

HDC hdcMem;
HBITMAP g_hbmPACMAN = NULL,g_hbmPACMAN_MASK = NULL;
HBITMAP g_hbmFRUIT = NULL,g_hbmFRUIT_MASK = NULL;
HBITMAP g_hbmWALL = NULL;
const int drawcyc[6] = {0,1,0,2,3,2};
void drawpc(HDC hdc, RECT* prc,HBITMAP *bmp,HBITMAP *msk,int x,int y,int sx,int sy){

	HBITMAP hbmOld;
	RECT fr = {x,y,x + PACMANW,y + PACMANH};
	if (msk != NULL){
	 	hbmOld = (HBITMAP)SelectObject(hdcMem, *msk);
		FillRect(hdc,&fr, (HBRUSH)GetStockObject(WHITE_BRUSH));
		BitBlt(hdc, x, y, PACMANW, PACMANH, hdcMem, sx, sy, SRCAND);
	}
	if (msk == NULL) hbmOld = SelectObject(hdcMem, *bmp);
	else SelectObject(hdcMem,*bmp);
	if (msk != NULL)
		BitBlt(hdc, x, y, PACMANW, PACMANH, hdcMem, sx, sy, SRCPAINT);
	else 
		BitBlt(hdc, x, y, PACMANW, PACMANH, hdcMem, sx, sy, SRCCOPY);

	

	SelectObject(hdcMem, hbmOld);
}
void draw(HDC hdc, RECT* prc){
	FillRect(hdc, prc, (HBRUSH)GetStockObject(WHITE_BRUSH));
	hdcMem = CreateCompatibleDC(hdc);
	for (int i = 0;i < n;i++){
		for (int j = 0;j < m;j++){
			if (game.state[i][j] == 1) {
				drawpc(hdc,prc,&g_hbmFRUIT,&g_hbmFRUIT_MASK,j * PACMANW ,i * PACMANH,0,0);
				//printf("drawing fruit\n");
			}
			else if (game.state[i][j] == -1) drawpc(hdc,prc,&g_hbmWALL,NULL,j * PACMANW,i * PACMANH,0,0);
		}
	}
	drawpc(hdc,prc,&g_hbmPACMAN,&g_hbmPACMAN_MASK,game.pX,game.pY,game.coms[game.curcom] * PACMANW,drawcyc[game.curCycle %6] * PACMANH );
	DeleteDC(hdcMem);
}
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;

	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	SelectObject(hdcMem, hbmColour);
	SelectObject(hdcMem2, hbmMask);

	SetBkColor(hdcMem, crTransparent);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			// loading pacman
			g_hbmPACMAN = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_PACMAN));
			if (g_hbmPACMAN == NULL)
				printf("Couldn't Load PACMAN\n");
			g_hbmPACMAN_MASK = CreateBitmapMask(g_hbmPACMAN, RGB(0, 0, 0));
			if (g_hbmPACMAN_MASK == NULL)
				printf("Couldn't Create MASK of PACMAN\n");
			// loading pacman

			// loading wall
			g_hbmWALL = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_WALL));
			if (g_hbmWALL == NULL)
				printf("Couldn't Load WALL\n");
			// loading wall

			// loading fruit
			g_hbmFRUIT = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_FRUIT));
			if (g_hbmFRUIT == NULL)
				printf("Couldn't Load FRUIT\n");
			g_hbmFRUIT_MASK = CreateBitmapMask(g_hbmFRUIT, RGB(0, 0, 0));
			if (g_hbmFRUIT_MASK == NULL)
				printf("Couldn't Create MASK of FRUIT\n");
			// loading fruit

		}break;
		case WM_TIMER:
		{
			update();
			RECT rcClient;
			HDC hdc = GetDC(hwnd);
			GetClientRect(hwnd, &rcClient);
			draw(hdc,&rcClient);
			ReleaseDC(hwnd,hdc);

		}break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_PAINT:
		{
            RECT rcClient;
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rcClient);
			draw(hdc,&rcClient);
			EndPaint(hwnd, &ps);
			
		}break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	preproc();
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"A Bitmap Program",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, (m ) *PACMANW + 20  ,(n) * PACMANH +40,
		NULL, NULL, hInstance, NULL);
	SetTimer(hwnd,
		IDT_TIMER1,
		REFRESH_RATE,
		(TIMERPROC) NULL);
	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}