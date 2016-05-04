// HWGUIDGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <initguid.h>
#include <Cap5BoardLibEx.h>
#include <VCA5CoreLib.h>
#include <conio.h>

#define	DEFAULT_CAP5_DLL_PATH		"ECPSV.DLL"
#define	DEFAULT_HWGUID_FILE_PATH	"HWGUID.txt"
#define VCA5_DLL_PATH				"VCA5Lib.dll"
		
void DbgMsg(LPCSTR szFmt, ...)
{
	#define DEBUG_STRING_LENGTH		255
    char szOutBuff [DEBUG_STRING_LENGTH] ;

    va_list  args ;
    va_start (args , szFmt) ;
    _vsnprintf_s( szOutBuff , _countof(szOutBuff), DEBUG_STRING_LENGTH-1, szFmt , args ) ;
    va_end ( args ) ;
	szOutBuff[DEBUG_STRING_LENGTH-1] = NULL;
	fprintf(stderr, szOutBuff);
}

#define	_V(x) \
	do { \
		if (x==0) { \
			printf("%s return 0\n", #x); \
			return FALSE; \
		} \
	} while (0)


IVCA5*		g_VCAApi;
static BOOL Uda5CreateInstance(HMODULE hLib,REFIID riid,void ** ppInterface)
{
	if (hLib) {

		BOOL rs;
		IUnknown* pUnknown;
		BOOL (FAR WINAPI*_CreateInstance)(IUnknown ** ppInterface);
		FARPROC test_proc=GetProcAddress(hLib,"VCA5CreateInstance");
		if (test_proc) {
			*(FARPROC*)&_CreateInstance=test_proc;
			rs=(*_CreateInstance)(&pUnknown);
			if (rs) {
				HRESULT hr;
				hr=pUnknown->QueryInterface(riid,ppInterface);
				pUnknown->Release();
				if (SUCCEEDED(hr))
					return TRUE;
			}
		}
	}
	
	return FALSE;
}

void	SaveHWGUID(char* szFileName, char* szUSN, char* szHWGUID)
{
	FILE *fp = 0;
	fopen_s( &fp, szFileName, "w+t");
	fprintf(fp, "USN:[%s], HWGUID [%s]", szUSN, szHWGUID);
	fclose(fp);
}


typedef BOOL (WINAPI* LPFCAP5QUERYINFO)(ULONG , ULONG uIn, void *);
typedef BOOL (WINAPI* LPCAP5GETSYSTEMINFO)(CMN5_SYSTEM_INFO *);


BOOL GetModulePath(char *path, int buffsize)
{
	// Get the filename of this executable
	if (GetModuleFileName(NULL, path, buffsize) == 0)
	return FALSE;

	for(size_t i= strlen(path) -1 ;i >= 0; --i){
		if( path[i] == '\\' ){
			path[i] = NULL;
		    return TRUE;
		}
	}
	return FALSE;
}



// Enable Visual Style
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

BOOL IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID, 
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 0, 0, 0, 0, 0, 
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}


void	ElevateAdmin()
{
	//Chck OS version
	OSVERSIONINFO VersionInfo;
	memset(&VersionInfo,0,sizeof(OSVERSIONINFO));
	VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&VersionInfo);
	if(VersionInfo.dwMajorVersion < 6){ // VISTA 
		return;
	}

	if(!IsRunAsAdmin()){
		wchar_t szPath[MAX_PATH];
		if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))){
            // Launch itself as administrator.
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd	= NULL;
            sei.nShow	= SW_SHOW;
			sei.fMask	= SEE_MASK_NO_CONSOLE ;

			if(ShellExecuteExW(&sei)){
				//Kill parent console
				ExitProcess(-1);
			}
		}
	}
}


int main(int argc, char* argv[])
{
	
	char	*szVCADll		= VCA5_DLL_PATH;
	char	*szDrvDll		= DEFAULT_CAP5_DLL_PATH;
	char	*szHWGUIDFile	= DEFAULT_HWGUID_FILE_PATH;
	char	szHWGUIDPath[1024];
	
	if(argc == 2){
		szDrvDll = 	argv[1];	
	}else if(argc == 3){
		szDrvDll		= 	argv[1];	
		szHWGUIDFile	= 	argv[2];	
	}

	GetModulePath(szHWGUIDPath, 1024);
	strcat(szHWGUIDPath, "\\");
	strcat(szHWGUIDPath, szHWGUIDFile);

	//Check UAC and elevate 
	ElevateAdmin();


	//Get USN Number
	char	szUSN[CMN5_SYSTEM_MAX_BOARD+1][16];
	memset(szUSN, 0, (CMN5_SYSTEM_MAX_BOARD+1) * 16);
	LPFCAP5QUERYINFO	pCap5QueryInfo;
	LPCAP5GETSYSTEMINFO	pCap5GetSystemInfo;
	CMN5_SYSTEM_INFO Cmn5SystemInfo;

	printf( "Please choose which GUID to generate:\n\n" );
	ULONG board = 0;

	//If installed UDP board then Show 
	HMODULE hCAP5Lib=NULL;
	hCAP5Lib=LoadLibrary(szDrvDll);
	if(hCAP5Lib){
		pCap5GetSystemInfo	= (LPCAP5GETSYSTEMINFO)GetProcAddress(hCAP5Lib, "Cap5GetSystemInfo");
		pCap5QueryInfo		= (LPFCAP5QUERYINFO)GetProcAddress(hCAP5Lib, "Cap5QueryInfo");

		if(pCap5GetSystemInfo(&Cmn5SystemInfo)){
			for(board = 0 ; board < Cmn5SystemInfo.uNumOfBoard ; board++){
				pCap5QueryInfo( CAP5_QR_GET_USN | (board<<16), 16, szUSN[board] );
					printf("%d. Board %d: %s\n", board, board, szUSN[board] );
			}
		}
	}

	printf( "%d. Generate from PC hardware (OPEN license)\n", board );

	printf( "\n\nPlease enter your choice:" );
	ULONG answer = _getche() - '0';
	printf("\n\n");

	_V( answer <= board );
	

	int cmd = (answer == board) ? VCA5_QR_GETHWGUIDOPEN : VCA5_QR_GETHWGUIDUSN;

	HMODULE hLib=NULL;
	char	szHWGUID[1024];
	memset(szHWGUID, 0, 1024);

	//_V((hLib=LoadLibrary(szVCADll)));
	hLib=LoadLibrary(szVCADll);
	if(!hLib){
		printf("Fail load [%s] VCA5Library error = 0x%X", szVCADll, GetLastError());
		return FALSE;
	}

	if (Uda5CreateInstance(hLib,IID_IVCA5,(void**)&g_VCAApi)) {
		VCA5_HWGUID_INFO guidInfo;
		guidInfo.szUSN			= szUSN[answer];	
		guidInfo.szDrvDllPath	= szDrvDll;
		guidInfo.szGUID			= szHWGUID;

		if(g_VCAApi->VCA5QueryInfo(cmd, sizeof(VCA5_HWGUID_INFO),(void*)&guidInfo)){
			
			DbgMsg("Success USN[%s] HWGUID [%s] \n", szUSN[answer], guidInfo.szGUID);
			SaveHWGUID(szHWGUIDPath, szUSN[answer], guidInfo.szGUID );
		}else{
			DbgMsg("Failed to get HWGUID (You might need to run this as admin user)\n");
		}
	}
	else
	{
		DbgMsg("Failed to get HWGUID (You might need to run this as admin user)\n");
	}

	printf("\nPress any key to continue...\n");
	_getch();

	return 0;
}




