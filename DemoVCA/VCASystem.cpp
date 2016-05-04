// VCASystem.cpp : implementation file
//

#include "stdafx.h"

#include <initguid.h>
#include "VCA5CoreLib.h"

#include "DemoVCA.h"
#include "VCASystem.h"
#include "./VideoSource/VCAVideoSource.h"
#include "./VCAEngine/VCAEngine.h"
#include "LicenseDlg.h"
#include "wm_user.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static BOOL		Uda5CreateInstance(HMODULE hLib, REFIID riid, void **ppInterface)
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

/* Start of Unix time stamp is 1970/01/01 00:00 (UTF),
* and, windows time stamp of this is 116444736000000000
*/
void ConvertLoaclFileTimeToVCA5Time(FILETIME* pFileTime, VCA5_TIME_STRUCT *pVCA5Time)
{
	ULONGLONG WinTime;
	ULONGLONG UnixTime;//Unix

	memcpy(&WinTime, pFileTime, sizeof(ULONGLONG));
	ULONGLONG unix_def_stamp = 11644473600;
	UnixTime = (WinTime/ 10000000) - unix_def_stamp;
	memcpy(pVCA5Time, &UnixTime, sizeof(ULONGLONG));
}


/////////////////////////////////////////////////////////////////////////////
CVCASystem::CVCASystem()
{
	m_bSetup	= FALSE;
	m_pVCA5API	= NULL;
	m_OpenEngCnt	= 0;
	m_hVCA5Lib	= NULL;
	ZeroMemory(m_pEngines, sizeof(m_pEngines));
}

CVCASystem::~CVCASystem()
{
	Endup();
}

BOOL	CVCASystem::Setup()
{
	if(m_bSetup){
		TRACE("CVCASystem Setup before \n");
		return FALSE;
	}
	
	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
	TCHAR	*szVCA5DllPath = CAPPConfigure::Instance()->GetVCA5DllPath();
	HMODULE hLib=NULL;

	//1. Load VCA5Dll
	if (szVCA5DllPath) {
		hLib = LoadLibrary(szVCA5DllPath);
		CHECK_FUNC_AND_RETURN_FALSE(hLib != NULL, _T("Fail to load VCA5 library"));
	}

	if (!Uda5CreateInstance(hLib, IID_IVCA5, (void **)&m_pVCA5API)) {
		CHECK_FUNC_AND_RETURN_FALSE(0, _T("Fail to Create VCA5 library"));	
	}


	BOOL (FAR WINAPI*_VCA5Init)(ULONG ulVersion);
	FARPROC test_proc=GetProcAddress(hLib,"VCA5Init");
	if (test_proc) {
		*(FARPROC*)&_VCA5Init=test_proc;
		(*_VCA5Init)(VCA5_VERSION);
	}else{
		TRACE("This program support VCA5Library V1.2 \n");
		goto EXIT;
	}
		
	pLicenseMgr->SetVCALib(m_pVCA5API);
	//2. Activate License
	if(!pLicenseMgr->Activate(CAPPConfigure::Instance()->GetCAP5DllPath())){
		TRACE("Can not Setup VCA5 instance\n");
		goto EXIT;
	}


	m_bSetup	= TRUE;
EXIT:
	if(!m_bSetup){
		if(m_pVCA5API){
			m_pVCA5API->Release();
			m_pVCA5API = NULL;
		}
	}
	return m_bSetup;
}



void	CVCASystem::Endup()
{
	if (m_bSetup)
	{
		for (ULONG i=0; i<m_OpenEngCnt; ++i)
		{
			Stop(i);
			Close(i);
		}
		
		m_pVCA5API->Release();
		m_pVCA5API	= NULL;

		if(m_hVCA5Lib){
			FreeLibrary(m_hVCA5Lib);
			m_hVCA5Lib = NULL;
		}
		m_bSetup	= FALSE;
		m_OpenEngCnt	= 0;
	}
}


BOOL	CVCASystem::Run( int iEngId )
{
	if (!m_bSetup) {
		TRACE("VCASystem is not Setup before \n");
		return FALSE;
	}


	if(m_pEngines[iEngId]){
		m_pEngines[iEngId]->Run();
	}


	
	return TRUE;
}


void	CVCASystem::Stop( int iEngId )
{
	if (!m_bSetup) {
		TRACE("VCASystem is not Setup before \n");
	}
		
	if(m_pEngines[iEngId]){
		m_pEngines[iEngId]->Stop();
	}
}


BOOL	CVCASystem::Open( int iEngId )
{
	BOOL bRet = TRUE;
	USES_CONVERSION;

	if( !m_bSetup ){
		TRACE( "VCAsystem not setup!\n" );
	}

	m_pEngines[iEngId] = new CVCAEngine(iEngId, this);
	bRet = m_pEngines[iEngId]->Setup( iEngId );

	if(bRet){
		m_OpenEngCnt	+= 1;
	}
	return bRet;
}


BOOL	CVCASystem::Close( int iEngId )
{
	if( !m_bSetup ){
		return FALSE;
	}

	if(m_pEngines[iEngId]){
		m_pEngines[iEngId]->Stop();
		m_pEngines[iEngId]->Endup();

		m_OpenEngCnt	-= 1;
	}

	SAFE_DELETE(m_pEngines[iEngId]);

	return TRUE;
}

/*
ULONG		CVCASystem::GetFunction(int iEngId)
{

	ULONG	ulFunction = 0;
	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
	
	for(ULONG i = 0 ; i < m_EngineParams[iEngId].ulLicenseCnt ; i++){
		VCA_APP_LICENSE_INFO *pInfo = pLicenseMgr->GetLicenseInfo(m_EngineParams[iEngId].ucLicenseId[i]);
		if(pInfo) ulFunction |= pInfo->VCA5LicenseInfo.ulFunction;
	}

<<<<<<< .mine
	return ulFunction;
=======
	if (Uda5CreateInstance(hLib, IID_IVCA5, (void **)&m_pVCA5API)) {
		
		VCA5_LICENSE_INFO	LicenseInfo[CMN5_SYSTEM_MAX_BOARD];
		for(i = 0  ; i < LicenseCnt ; i++){
			LicenseInfo[i].szUSN		= szUSN[i];
			LicenseInfo[i].szDrvDllPath	= T2A(szDrvDllPath);
			LicenseInfo[i].szLicense	= szLicense[i];
		}
		
//		CHECK_FUNC_AND_RETURN_FALSE(m_pVCA5API->VCA5Activate(LicenseCnt, &LicenseInfo[0]), _T("Fail to activate VCA5"));
		BOOL bActivated = FALSE;
		if( FALSE == m_pVCA5API->VCA5Activate(LicenseCnt, &LicenseInfo[0] )  )
		{
			CString sErr = _T("Unknown error");
			VCA5_ERROR_CODE_ITEM item;
			if( m_pVCA5API->VCA5GetLastErrorCode( &item ) )
			{
				switch( VCA5_EXTRACT_ERRCODE2( item.ErrorCode ) )
				{
					case VCA5_ERRCMN_F_LICENSE_EXPIRED:
					{
						CString s;
						s.Format( _T("At least one license has expired: %s"), A2T( item.AuxMsg ) );
						AfxMessageBox( s, MB_OK | MB_ICONEXCLAMATION );
					}
					// Intentional durchfall...
					case VCA5_ERRCMN_F_ALREADY_ACTIVATED:
					{
						// Already activated
						bActivated = TRUE;
					}
					break;

					default:
					{
						bActivated = FALSE;
						sErr = CString( A2T( item.AuxMsg ) );
					}
					break;
				}
			}

			if( !bActivated )
			{
				CString sMsg;
				sMsg.Format( _T("Unable to activate VCA5: %s"), sErr );
				AfxMessageBox( sMsg );
				return FALSE;
			}
		}
		else
		{
			bActivated = TRUE;
		}



		CHECK_FUNC_AND_RETURN_FALSE(m_pVCA5API->VCA5GetSystemInfo(&m_SystemInfo), _T("Failed to get VCA5 information"));
		for(i = 0  ; i < m_SystemInfo.ulNumOfLicense ; i++)m_OpenEngCnt+=m_SystemInfo.ulNumOfEngine[i];

		CHECK_FUNC_AND_RETURN_FALSE((m_OpenEngCnt !=0), _T("There are no available VCA engines to use!\nThe application will now exit."));

		m_hVCA5Lib = hLib;
		return TRUE;
	} 

	return FALSE;
>>>>>>> .r410
}
*/