#ifndef __VIDEO_SOURCE_USER_INTERFACE_H__
#define __VIDEO_SOURCE_USER_INTERFACE_H__

#include "VideoSource/VCAVideoSource.h"

BOOL	DoFileOpen(HINSTANCE hInst, HWND hWnd);
BOOL	DoStreamOpen(HINSTANCE hInst, HWND hWnd);
BOOL	DoDShowOpen(HINSTANCE hInst, HWND hWnd);
BOOL	DoCAP5Open(HINSTANCE hInst, HWND hWnd);
void	DoClose();


extern IVCAVideoSource	*g_pVideoSource;

#endif