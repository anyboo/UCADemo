// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef POINTER_64
#define	POINTER_64	__ptr64
#endif


#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <atlbase.h>


// TODO: reference additional headers your program requires here
