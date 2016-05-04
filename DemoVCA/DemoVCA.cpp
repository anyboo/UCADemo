// DemoVCA.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "DemoVCADlg.h"
#include "CustomCmdLineInfo.h"
#include "ConfigAppDlg.h"
#include "enums.h"

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


void	ElevateAdmin(HWND hWnd)
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
            sei.hwnd	= hWnd;
            sei.nShow	= SW_SHOW;
			sei.fMask	= SEE_MASK_NO_CONSOLE ;

			if(ShellExecuteExW(&sei)){
				//Kill parent console
				ExitProcess(-1);
			}
		}
	}
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemoVCA

BEGIN_MESSAGE_MAP(CDemoVCA, CWinApp)
	//{{AFX_MSG_MAP(CDemoVCA)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoVCA construction

CDemoVCA::CDemoVCA()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDemoVCA object

CDemoVCA theApp;

/////////////////////////////////////////////////////////////////////////////
// CDemoVCA initialization

BOOL CDemoVCA::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	//Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//need to UAC 
#ifndef _DEBUG
	ElevateAdmin(NULL);
#endif


	CCustomCmdLineInfo info;
	ParseCommandLine( info );

	CDialog *pDlg;

	if( info.GetMode() == eAppModeDemo )
	{
		pDlg = new CDemoVCADlg();
		if(!((CDemoVCADlg *)pDlg)->InitInstance())
		{
			delete pDlg;
			return FALSE;
		}
	}
	else
	{
		// Must be AppModeConfig
		pDlg = new CConfigAppDlg( &info );
	}

	
	m_pMainWnd = pDlg;
	int nResponse = pDlg->DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	delete pDlg;
	CAPPConfigure::Instance()->DestroySelf();
	CVCAConfigure::Instance()->DestroySelf();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
