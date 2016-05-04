/*
	HISTROY :
	2005.0218	1. �������� �ҽ� �ڵ尡 �־� �ѹ濡 ����.
				2. MFC Stdafx.h�� ���� ����

*/

#define GALLOC(dwBytes)		GlobalAlloc(GPTR, dwBytes)
#define GFREE(lptr)			GlobalFree(lptr)
#define LALLOC(dwBytes)		LocalAlloc(LPTR, dwBytes)
#define LFREE(lptr)			LocalFree(lptr)

#define	WIDTHBYTES_(c)	((c+31)/32*4)	// c = width * bpp

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(c) if(c){c->Release();c=NULL;}else{}
#endif

// "DebugWindow.DLL" �� �����Ѵ�.
typedef VOID(WINAPI *P_DBG)( LPTSTR _lpsz );
typedef VOID(WINAPI *P_DBGn)( LPTSTR _lpsz );

// n : 1 .. n
// _fEnableDBWindow : "DebugWindow.DLL" �� �����Ѵ�.
void ErrDbgLevel( int		n, 
				  LPSTR		lpTag, 
				  LPSTR	    lpFilename, 
				  BOOL		_fEnableDBWindow ); 
void FreeErrDbgLevel();

INT  ErrMsg (LPSTR sz,...);
int	 ErrDbg (int nLevel, LPSTR sz,...);
int  ErrDbgX (LPSTR sz,...);
int  ErrDbgX (int nLevel, LPSTR sz,...);

// "DebugWindow.DLL" �� �����Ѵ�.
INT  ErrDbgDB(LPSTR sz,...);
INT  ErrDbgDBn(LPSTR sz,...);

int  DisplayError(TCHAR * ErrorName);
int	 seungsu(int base, int jea);
int	 HexString2Dec(LPCSTR szHex);
int  DecString2Dec(LPCSTR szDec);
BOOL IsHexString(LPCSTR szText);
void double2string(double f, char *buffer, WORD nreal);	// interger.3
void double2hex( double f, INT nBinaryCount, DWORD* pdwHex);
void Num2BinaryString(BYTE cbByte, char* buffer);
void Num2BinaryString(WORD wWord, char* buffer);
void Num2BinaryString(DWORD dwDWord, char* buffer);
void Num2ComaString( INT n, char* buffer );

HRESULT MyUcToAnsi( LPWSTR pwszUc, LPSTR pszAnsi, int cch );
HRESULT MyAnsiToUc( LPSTR pszAnsi, LPWSTR pwszUc, int cch );

#pragma comment(lib, "Version.lib")
BOOL MiscGetFixedFileInfo( LPTSTR _lpFile, PVOID _ptVS_FIXEDFILEINFO );
BOOL MiscGetFileVersionString( LPTSTR _lpFile, LPTSTR _lpFileVersion );

//<><><><><><><><><><><><><><><><><><><><><><><><><>
// �������� Modalless ���̾�α׵��� Ű���� ������ ����  
#define	MAX_DIALOGBOXCOUNT		20
extern  HWND ghWndDialogBox[];

BOOL IsAllEmpty();
INT	 gethwndindex( HWND _hwnd );
INT	 getemptyindex();

