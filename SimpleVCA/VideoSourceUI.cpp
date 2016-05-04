#include "stdafx.h"
#include "resource.h"
#include "VideoSourceUI.h"

#define MAX_REG_ENTRIES 10
IVCAVideoSource	*g_pVideoSource;

INT_PTR CALLBACK OpenStreamDlg( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
//--------------------------------------------------------------------------
BOOL	DoFileOpen(HINSTANCE hInst, HWND hWnd)
{
	TCHAR nameBuf[MAX_PATH];
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof( OPENFILENAME ) );
	memset( nameBuf, 0, _countof(nameBuf));
	
	ofn.lStructSize		= sizeof( ofn );
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= _T("Media File\0*.avi;*.vob;*.mod;*.mov;*.wmv;*.asf;*.mpg;*.mp4\0Raw File\0*.raw\0");
	ofn.nFilterIndex	= 0;
	ofn.lpstrFile		= nameBuf;
	ofn.nMaxFile		= 256;

	if( FALSE != GetOpenFileName( &ofn ) )
	{
		IVCAVideoSource::eSOURCETYPE type;
		TCHAR *szTemp = _tcslwr(nameBuf);
		if(_tcsstr(nameBuf, _T(".raw"))){
			type = IVCAVideoSource::RAWFILESOURCE;
		}else{
			type = IVCAVideoSource::COMPRESSEDFILESOURCE;
		}

		g_pVideoSource = CreateVCAVideoSource(type, nameBuf, 0);
		if(!g_pVideoSource){
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

TCHAR	g_szStreamerExe[4096];
TCHAR	g_szStreamerArgs[4096];
BOOL	DoStreamOpen(HINSTANCE hInst, HWND hWnd)
{
	if( IDOK != DialogBox(hInst, MAKEINTRESOURCE(IDD_OPENSTREAM_DLG), hWnd, OpenStreamDlg) ) return FALSE;

	g_pVideoSource = CreateVCAVideoSource(IVCAVideoSource::STREAMSOURCE, g_szStreamerExe, (DWORD)g_szStreamerArgs);
	if(!g_pVideoSource){
		return (INT_PTR)FALSE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
// Message handler for open stream box
INT_PTR CALLBACK OpenStreamDlg( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_INITDIALOG:
		{
			// Initialize the edit boxes to the values used last time
			HWND hExeEdit = GetDlgItem( hDlg, IDC_EXE_EDIT );
			HWND hArgsEdit = GetDlgItem( hDlg, IDC_ARGS_EDIT );

			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\UDP Technology\\SimpleVCA.exe"),
											REG_OPTION_NON_VOLATILE, KEY_READ, &hKey );

			if( hKey )
			{
				TCHAR tcBuf[4096];
				DWORD dwType, dwLen;

				dwLen = 4096;

				if( ERROR_SUCCESS == RegQueryValueEx( hKey, _T("exe"), NULL, &dwType, (BYTE *)tcBuf, &dwLen ))
				{
					SetWindowText( hExeEdit, tcBuf );
				}

				dwLen = 4096;
				if( ERROR_SUCCESS == RegQueryValueEx( hKey, _T("args"), NULL, &dwType, (BYTE *)tcBuf, &dwLen ) )
				{
					SetWindowText( hArgsEdit, tcBuf );
				}

			}
			else
			{
				// Defaults:
				SetWindowText( hExeEdit, _T("C:\\Program Files\\VideoLAN\\VLC\\vlc.exe") );
				SetWindowText( hArgsEdit, _T("--sout=#transcode{vcodec=YUY2}:duplicate{dst=display,dst=std{access=file,mux=avi,dst=-}} --repeat") );

			}
			return (INT_PTR) TRUE;
		}


	case WM_COMMAND:
		if( LOWORD(wParam) == IDOK )
		{
			HWND hExeEdit = GetDlgItem( hDlg, IDC_EXE_EDIT );
			HWND hArgsEdit = GetDlgItem( hDlg, IDC_ARGS_EDIT );

			// Store the Exe and corresponding args in the registry
			TCHAR tcBuf[4096];
			GetWindowText( hExeEdit, tcBuf, 4096 );

			if( _tcslen( tcBuf ) )
			{
				DWORD dwDisposition;
				HKEY hKey;
				LONG lResult = RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\UDP Technology\\SimpleVCA.exe"), 
												NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition );

				if( hKey )
				{
					// Set the value of the exe key
					RegSetValueEx( hKey, _T("exe"), NULL, REG_SZ, (BYTE *)tcBuf, (DWORD)_tcslen( tcBuf ) + 1 );

					// Set the value of the args key
					GetWindowText( hArgsEdit, tcBuf, 4096 );
					RegSetValueEx( hKey, _T("args"), NULL, REG_SZ, (BYTE *)tcBuf, (DWORD)_tcslen( tcBuf ) + 1 );

					RegCloseKey( hKey );
				}
			}

			GetWindowText( hExeEdit, g_szStreamerExe, 4096 );
			GetWindowText( hArgsEdit, g_szStreamerArgs, 4096 );
			
			EndDialog( hDlg, LOWORD(wParam) );
			return (INT_PTR)TRUE;
		}
		else
		if( LOWORD(wParam) == IDCANCEL )
		{
			EndDialog( hDlg, LOWORD(wParam) );
			return (INT_PTR)FALSE;
		}
		break;
	}

	return (INT_PTR) FALSE;
}



BOOL DoDShowOpen(HINSTANCE hInst, HWND hWnd)
{

	return TRUE;
}


BOOL DoCAP5Open(HINSTANCE hInst, HWND hWnd)
{


	return TRUE;
}


//-----------------------------------------------------------------------------------
void DoClose()
{
	if(g_pVideoSource){
		g_pVideoSource->Close();
		DestroyVCAVideoSource(g_pVideoSource);
		g_pVideoSource = NULL;
	}
}
