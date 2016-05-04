#ifndef __SIMPLE_VCA_H__
#define __SIMPLE_VCA_H__

#include "VCA5CoreLib.h"

BOOL	VCASetup(TCHAR	*szDrvDll, TCHAR *szLicenseFile);
void	VCAEndup();

BOOL	VCAOpen(HWND hWnd, VCA5_ENGINE_PARAMS* pEngineParam);
void	VCAClose();

BOOL	VCASetConf();
BOOL	VCASetConfFile();
BOOL	VCAPlay(HWND hWnd);



#endif