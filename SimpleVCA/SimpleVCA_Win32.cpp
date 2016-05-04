// SimpleVCA_Win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "SimpleVCA_Win32.h"
#include "GDIViewer.h"
#include "VideoSourceUI.h"
#include "SimpleVCA.h"

CGDIViewer		g_GDIViewer;	

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, LPTSTR ,int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STARTVCA5_WIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, lpCmdLine, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STARTVCA5_WIN32));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STARTVCA5_WIN32));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_STARTVCA5_WIN32);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;

	TCHAR *szLIcensePath	= _T("license.lic");
	TCHAR *szDvrDllPath		= NULL;
	
	//Parse argc, argv
	UINT i, argc = 0;
	TCHAR* argv[10];
	for(i = 0 ; i < _tcslen(lpCmdLine) ; i++){
		if(lpCmdLine[i] == _T(' ')){
			argv[argc] = lpCmdLine+i+1;
			argc++;
		}
	}
	
	for(i = 0 ; i < argc; i++){
		TCHAR *szParam = _tcslwr(argv[i]);
		if(_tcsstr(szParam, _T(".lic"))){
			szLIcensePath	= argv[i];
		}else if(_tcsstr(szParam, _T(".dll"))){
			szDvrDllPath	= argv[i];
			szLIcensePath	= NULL;
		}
	}

	hInst = hInstance; // Store instance handle in our global variable
 
	if(!VCASetup(szDvrDllPath, szLIcensePath)){
		MessageBox(NULL, _T("Can not Setup VCA5 Instance, is the license valid?"), _T("ERROR"), MB_OK);
		return FALSE;
	}

	hWnd = CreateWindow( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
	   CW_USEDEFAULT, 0, 320, 240, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
	  return FALSE;
	}

	if(!g_GDIViewer.Setup( hWnd, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT,CGDIViewer::COLORTYPE_YUY2)){
		MessageBox(NULL, _T("Can not Setup GDI Viewer"), _T("ERROR"), MB_OK);
		return FALSE;
	}



	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


void	CloseVideoSource(HWND hWnd)
{
	KillTimer(hWnd, 1);
	DoClose();
}

BOOL	OpenVideoSource(HWND hWnd, int wmId)
{
	CloseVideoSource(hWnd);

	BOOL bResult = FALSE;
	if(ID_FILE_OPEN_FILE == wmId)						bResult = DoFileOpen(hInst, hWnd);
	else if(ID_FILE_OPEN_STREAM == wmId)				bResult = DoStreamOpen(hInst, hWnd);
	else if(ID_FILE_OPEN_DIRECTSHOW_CAPTURE == wmId)	bResult = DoDShowOpen(hInst, hWnd);
	else if(ID_FILE_OPEN_CAPTURE_CARD == wmId)			bResult = DoCAP5Open(hInst, hWnd);
	
	if(!bResult || (NULL == g_pVideoSource)) return FALSE;

	VCA5_ENGINE_PARAMS EngineParam;
	memset(&EngineParam, 0, sizeof(VCA5_ENGINE_PARAMS));

	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD)&(EngineParam.ulImageSize), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD)&(EngineParam.ulColorFormat), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_VIDEOFORMAT, (DWORD)&(EngineParam.ulVideoFormat), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD)&(EngineParam.ulFrameRate100), 0);
	
	EngineParam.ulFrameRate100 *= 100;
	
	//Need V1.2 Library for license setting.
	EngineParam.ulLicenseCnt	= 1;
	EngineParam.ucLicenseId[0]	= 0;

	
	if(!VCAOpen(hWnd, &EngineParam)){
		DoClose();
		return FALSE;
	}

	VCASetConf();
	//VCASetConfFile();

	g_pVideoSource->Start();
	SetTimer(hWnd, 1, 33, NULL);	
	return bResult;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_OPEN_FILE:
		case ID_FILE_OPEN_STREAM:
		case ID_FILE_OPEN_DIRECTSHOW_CAPTURE:
		case ID_FILE_OPEN_CAPTURE_CARD:
			if(!OpenVideoSource(hWnd, wmId)){
				MessageBox( NULL, _T("Can not open video source"), _T("ERROR"), MB_OK );
			}
			break;
		
		case ID_FILE_STOP:
			CloseVideoSource(hWnd);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_TIMER:
		if( 1 == wParam ){
			VCAPlay(hWnd);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		g_GDIViewer.Update(hdc);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		CloseVideoSource(hWnd);
		VCAEndup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



