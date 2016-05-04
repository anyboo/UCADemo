
// for MFC
#include "StdAfx.h"

#ifndef _AFXDLL
#include <windows.h>
#include <tchar.h>		// Make all functions UNICODE safe.
#pragma message("WIN32API")
#else
#pragma message("MFCAPI")
#endif

#ifndef _AFXDLL
#include "Misc.h"
#endif

static INT		nErrDbgLevel = 0;
static TCHAR	szTag[20];
static HANDLE	hDbgFile = NULL;
static HMODULE  ghModule = NULL;

static P_DBG	gpDBG = NULL;
static P_DBGn	gpDBGn = NULL;

void FreeErrDbgLevel()
{
	if( ghModule ) FreeLibrary( ghModule );
}

void ErrDbgLevel(INT n, LPSTR lpTag, LPSTR lpFilename, BOOL _fEnableDBWindow )		// n : 1 .. n
{
	nErrDbgLevel = n;
	if( (lpTag != NULL) && lstrlen(lpTag) )
	{
		lstrcpy( szTag, lpTag );
	}

	if( nErrDbgLevel == -1 )
	{
		if( hDbgFile ) CloseHandle( hDbgFile );
		hDbgFile = NULL;

		TCHAR	szTemp[55];
		
		lstrcpy( szTemp, "C:\\ErrDbg.txt" );
		if( lpFilename != NULL ) lstrcpy( szTemp, lpFilename );

		hDbgFile = CreateFile( szTemp,
							   GENERIC_READ | GENERIC_WRITE,
							   FILE_SHARE_READ | FILE_SHARE_WRITE,
							   NULL,
							   OPEN_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL );
		if( hDbgFile == INVALID_HANDLE_VALUE ) hDbgFile = NULL;
	}

	//////////////////////////////////////////////////////////
	if( _fEnableDBWindow )
	{
		ghModule = LoadLibrary( "DebugWindow.dll" );
		if( ghModule )
		{
			gpDBG  = (P_DBG)GetProcAddress( ghModule, "DBW_DBG" );
			gpDBGn = (P_DBGn)GetProcAddress( ghModule, "DBW_DBGn" );
			ErrDbg( 1, "디버거 초기화 성공 \r\n" );
		}
		else
		{
			gpDBG  = NULL;
			gpDBGn = NULL;
		}
	}
}

void ErrDbgFile( LPTSTR sz, LPTSTR szTag = NULL )
{
	if( hDbgFile == NULL ) return;

	DWORD		dwWritten;
/*
	SYSTEMTIME	sysTemp;
	TCHAR		szTemp[30];

	GetLocalTime(&sysTemp);
	wsprintf(szTemp, "[%d-%d-%d %d:%d:%d.%d] ", sysTemp.wYear
											  , sysTemp.wMonth
											  , sysTemp.wDay
											  , sysTemp.wHour
											  , sysTemp.wMinute
											  , sysTemp.wSecond
											  , sysTemp.wMilliseconds );
*/
	SetEndOfFile( hDbgFile );
/*
	WriteFile( hDbgFile, (LPCVOID)szTemp, lstrlen(szTemp), &dwWritten, NULL );
	if( szTag != NULL )
		WriteFile( hDbgFile, (LPCVOID)szTag, lstrlen(szTag), &dwWritten, NULL );
*/
	if( sz != NULL )
		WriteFile( hDbgFile, (LPCVOID)sz, lstrlen(sz), &dwWritten, NULL );
}

INT ErrDbg(INT nLevel, LPTSTR sz,...)
{
    if ( nErrDbgLevel == 0 ) return 0;

	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

	wsprintf( ach, "%s", szTag );

    va_list va;

    va_start(va, sz);
    wvsprintf( &ach[lstrlen(ach)], sz, va );
    va_end(va);

	if( nLevel == 99 )
	{
		ErrDbgFile( ach, szTag );
	}
	else
	if( nLevel >= nErrDbgLevel )
	{
		OutputDebugString(ach);
		if( gpDBG ) gpDBG(ach);
	}

	LFREE(ach);

    return FALSE;
}

INT ErrDbgX(INT nLevel, LPTSTR sz,...)
{
    if ( nErrDbgLevel == 0 ) return 0;

	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

    va_list va;

    va_start(va, sz);
    wvsprintf (ach, sz, va);
    va_end(va);

	if( nLevel >= nErrDbgLevel )
	{
		OutputDebugString(ach);
		if( gpDBG ) gpDBG(ach);
	}

	ErrDbgFile( ach );

	LFREE(ach);

    return FALSE;
}

INT ErrDbgX(LPTSTR sz,...)
{
	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

    va_list va;

    va_start(va, sz);
    wvsprintf (ach, sz, va);
    va_end(va);

	OutputDebugString(ach);
	if( gpDBG ) gpDBG(ach);

	LFREE(ach);

    return FALSE;
}

INT ErrDbgDB(LPTSTR sz,...)
{
	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

    va_list va;

    va_start(va, sz);
    wvsprintf (ach, sz, va);
    va_end(va);

	//OutputDebugString(ach);
	//OutputDebugString("\r\n");

	if( gpDBG ) gpDBG(ach);

	LFREE(ach);

    return FALSE;
}

INT ErrDbgDBn(LPTSTR sz,...)
{
	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

    va_list va;

    va_start(va, sz);
    wvsprintf (ach, sz, va);
    va_end(va);

	if( gpDBGn ) gpDBGn(ach);

	LFREE(ach);

    return FALSE;
}

INT ErrMsg(LPTSTR sz,...)
{
	PTCHAR	ach;
	ach = (PTCHAR)LALLOC( 2048 );

    va_list va;

    va_start(va, sz);
    wvsprintf (ach, sz, va);
    va_end(va);

    MessageBox(NULL, ach, "_MISC_", MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);

	LFREE(ach);

    return FALSE;
}

INT seungsu(INT base, INT jea)
{
	INT i = 0, nret;
	
	if(jea == 0) 
		nret = 1;
	else
	{
		nret = base;
		for(i=1; i < jea; i++)
			nret = nret * base;
	}
	return nret;
}

INT HexString2Dec(LPCSTR szHex)
{
	INT 	i, len;
	INT     nret = 0;

	szHex = CharUpper((LPSTR)szHex);
	len = lstrlen(szHex);

	for(i=0; i < len; i++)
	{
		if(szHex[i] >= '0' && szHex[i] <= '9')
		{
			nret += (szHex[i] - '0') * seungsu(16, (len-(i+1)));
		}
		else
		if(szHex[i] >= 'A' && szHex[i] <= 'F')
		{
			nret += ((szHex[i] - 'A') + 10) * seungsu(16, (len-(i+1)));
		}
	}

	return nret;
}

INT DecString2Dec(LPCTSTR szDec)
{
	INT 	i, len;
	INT     nret = 0;

	len = lstrlen(szDec);
	for(i=0; i < len; i++)
	{
		if(szDec[i] >= '0' && szDec[i] <= '9')
		{
			nret += (szDec[i] - '0') * seungsu(10, (len-(i+1)));
		}
	}

	return nret;
}

BOOL IsHexString(LPCTSTR szText)
{
	PTCHAR	pszHex;

	pszHex = CharUpper((LPSTR)szText);

	if( pszHex[0] == '0' && pszHex[1] == 'X' ) return TRUE;
	
	return FALSE;
}

void double2string(double f, LPTSTR	buffer, WORD nreal)	// interger.3
{
	INT		k,r[10],pt;
	double	real;
	char	cbtemp[2];

	pt = 0;
	k = (INT)f;
	real = f - (double)k;

	wsprintf(buffer, "%d.",k);
	for(pt=0; pt<nreal; pt++)
	{
		real = real * (double)10;
		r[pt] = (INT)real;
		real = real - (double)r[pt];
		wsprintf(cbtemp, "%d",r[pt]);
		lstrcat(buffer, cbtemp);
	}
}

void double2hex( double f, INT nBinaryCount, DWORD* pdwHex)	// interger.nreal
{
	*pdwHex = 0;

	if( f > 1.0 ) f = f - (double)(INT)f;

	for(INT i=0; i < nBinaryCount; i++)
	{
		f = f * 2.0;
		if( f >= 1.0 )
		{
			f = f - 1.0;
			*pdwHex |= 0x00000001;
		}
		*pdwHex <<= 1;
	}
}

void Num2ComaString( INT n, char* buffer )
{
	char	sz[15];
	INT		i, j, k;
	INT     nComa;

	wsprintf( sz, "%d", n );

	// 콤마가 몇개나 들어가야하나 ?
	i = lstrlen(sz);
	nComa = ((i-1) / 3);
	if( nComa == 0 )
	{
		wsprintf( buffer, "%d", n );
		return;
	}

	// 목표버퍼는 콤마갯수 만큼 더 필요하다.
	j = i + nComa;
	buffer[j] = 0;
	
	// 콤마 넣을자리 찾기
	k = 0;
	for( i=lstrlen(sz)-1; i >= 0; i-- )
	{
		k++;
		j--;
		buffer[j] = sz[i];
		
		if( j == 0 ) break;	// 마지막 자리가 3,6,9라도 콤마는 넣지 않는다.
		if( (k % 3) == 0 )
		{
			j--;
			buffer[j] = ',';
		}
	}
}

void Num2BinaryString(BYTE cbByte, char* buffer)
{
	INT		i;

	for(i=0; i<sizeof(BYTE)*8; i++)
	{
		*(buffer+i) = '0';
		if( (cbByte << i) & 0x80 ) *(buffer+i) = '1';
	}
	*(buffer+i) = 0;	// Null Terminate
}

void Num2BinaryString(WORD wWord, char* buffer)
{
	INT		i;

	for(i=0; i<sizeof(WORD)*8; i++)
	{
		*(buffer+i) = '0';
		if( (wWord << i) & 0x8000 ) *(buffer+i) = '1';
	}
	*(buffer+i) = 0;	// Null Terminate
}

void Num2BinaryString(DWORD dwDWord, char* buffer)
{
	INT		i;

	for(i=0; i<sizeof(DWORD)*8; i++)
	{
		*(buffer+i) = '0';
		if( (dwDWord << i) & 0x80000000 ) *(buffer+i) = '1';
	}
	*(buffer+i) = 0;	// Null Terminate
}

INT DisplayError(TCHAR * ErrorName)
{
    DWORD	Err = GetLastError();
    LPVOID	lpMessageBuffer = NULL;
    TCHAR	szText[255];

    if ( FormatMessage( 
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL, 
         Err,  
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR) &lpMessageBuffer,  
         0,  
         NULL ) )
		wsprintf(szText, " %s FAILURE: %s (%d) ", ErrorName, (TCHAR *)lpMessageBuffer, Err);
    else 
        wsprintf(szText, " %s FAILURE: (%d) ", ErrorName, Err);
    
	MessageBox(NULL, szText, "ERROR", MB_OK);

    if (lpMessageBuffer) LocalFree( lpMessageBuffer ); // Free system buffer 

    SetLastError(Err);

    return FALSE;
}

HRESULT MyUcToAnsi( LPWSTR pwszUc, LPTSTR pszAnsi, INT cch )
{
	HRESULT hr = E_POINTER;
	INT cSize;
	INT cOut;

	if (NULL != pszAnsi && NULL != pwszUc)
	{
		if (0 == cch)
			cSize = WideCharToMultiByte(CP_ACP,0,pwszUc,-1,NULL,0,NULL,NULL);
		else
			cSize = cch;

		if (0 != cSize)
		{
			cOut = WideCharToMultiByte(CP_ACP,0,pwszUc,-1,pszAnsi,cSize,NULL,NULL);
			if (0 != cOut) hr = NOERROR;
		}
		else
			hr = E_FAIL;
	}

	return hr;
}

HRESULT MyAnsiToUc( LPSTR pszAnsi, LPWSTR pwszUc, INT cch)
{
	HRESULT hr = E_POINTER;
	INT		cSize;
	INT		cOut;

	if (NULL != pszAnsi && NULL == pwszUc)
	{
		if (0 == cch)
		{
			cSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszAnsi, -1, NULL, 0);
			return (HRESULT)cSize;
		}
	}

	if (NULL != pszAnsi && NULL != pwszUc)
	{
		if (0 == cch)
		{
			cSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszAnsi, -1, NULL, 0);
		}
		else
		{
			cSize = cch;
		}

		if (0 != cSize)
		{
			cOut = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszAnsi, -1, pwszUc, cSize);
			if (0 != cOut) hr = NOERROR;
		}
		else
		{
			hr = E_FAIL;
		}
	}

	if( hr == NOERROR ) hr = (HRESULT)cSize;
	else hr = -1;

	return hr;
}

BOOL MiscGetFixedFileInfo( LPTSTR _lpFile, PVOID _ptVS_FIXEDFILEINFO )
{
	DWORD dwVerInfoSize = GetFileVersionInfoSize( _lpFile, 0 );
	if( 0 == dwVerInfoSize ) return FALSE;

	PBYTE pVerInfo = new BYTE[dwVerInfoSize];
	BOOL fret = GetFileVersionInfo( _lpFile, 0, dwVerInfoSize, pVerInfo );

	LPVOID	lpRetValue = NULL;
	UINT	uiRetValue = 0;
	VerQueryValue( pVerInfo, "\\", &lpRetValue, &uiRetValue );
	CopyMemory( _ptVS_FIXEDFILEINFO, lpRetValue, sizeof(VS_FIXEDFILEINFO) );
	delete[] pVerInfo;

	return fret;
}

BOOL MiscGetFileVersionString( LPTSTR _lpFile, LPTSTR _lpFileVersion )
{
	VS_FIXEDFILEINFO	tVerInfo = {0,};
	BOOL fret = MiscGetFixedFileInfo( _lpFile, &tVerInfo );

	wsprintf( _lpFileVersion, "%d.%d.%d.%d", HIWORD(tVerInfo.dwFileVersionMS)
										   , LOWORD(tVerInfo.dwFileVersionMS)
										   , HIWORD(tVerInfo.dwFileVersionLS)
										   , LOWORD(tVerInfo.dwFileVersionLS) );
	return fret;
}

//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
/// 모달리스 다이얼로그박스의 키보드 지원을 위해...
HWND ghWndDialogBox[MAX_DIALOGBOXCOUNT];

BOOL IsAllEmpty()
{
	for(INT i=0; i<MAX_DIALOGBOXCOUNT; i++)
	{
		if( ghWndDialogBox[i] != NULL ) return FALSE;
	}

	return TRUE;
}

INT	gethwndindex( HWND _hwnd )
{
	for(INT i=0; i<MAX_DIALOGBOXCOUNT; i++)
	{
		if( ghWndDialogBox[i] == _hwnd ) return i;
	}
	return -1;
}

INT	getemptyindex()
{
	for(INT i=0; i<MAX_DIALOGBOXCOUNT; i++)
	{
		if( ghWndDialogBox[i] == NULL ) return i;
	}
	return -1;
}

