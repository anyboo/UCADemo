#include "stdafx.h"
#include "LicenseMgr.h"
#include "LicenseDlg.h"
#include "AppConfigure.h"

BOOL	ReadLicense(TCHAR *szFIleName, char *szLincens, DWORD *nLen)
{
	FILE	*pFile = NULL;
	
	errno_t err = _tfopen_s(&pFile, szFIleName, _T("r+b"));
	if(err) {
		return FALSE;
	}

	DWORD filesize, readsize;
	

	fseek(pFile , 0, SEEK_END );
	filesize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET );
	
	if(filesize > *nLen) return FALSE;
	readsize = fread(szLincens, 1, filesize, pFile);
	if(readsize != filesize) return FALSE;

	*nLen = filesize;
	fclose(pFile);
	return TRUE;
}


CLicenseMgr::CLicenseMgr(void)
{
	m_LicenseCnt = 0;
	memset(m_LicenseInfo, 0, sizeof(m_LicenseInfo));
}

CLicenseMgr::~CLicenseMgr(void)
{
}

void	CLicenseMgr::SetLicenseInfo(DWORD count, VCA_APP_LICENSE_INFO *pLicenseInfo)
{
	if(count > VCA5_MAX_NUM_LICENSE){
		count = VCA5_MAX_NUM_LICENSE;
	}

	memcpy(m_LicenseInfo, pLicenseInfo, count*sizeof(VCA_APP_LICENSE_INFO));
	m_LicenseCnt = count;
}


VCA_APP_LICENSE_INFO *CLicenseMgr::GetLicenseInfoById(DWORD id)
{
	DWORD i;
	for(i = 0; i < m_LicenseCnt ; i++){
		if(m_LicenseInfo[i].VCA5LicenseInfo.nLicenseID == id) return &m_LicenseInfo[i];
	}
	return NULL;
}


BOOL	CLicenseMgr::Activate(TCHAR *szDrvDllPath)
{
	char*	szLicense[VCA5_MAX_NUM_LICENSE], *szNewLicense, *szUSN[VCA5_MAX_NUM_LICENSE];
	TCHAR	szErrorStr[256];
	char	szTempLicense[1024];
	DWORD	i, LicenseCnt, uLicenseLength = 1024;
	BOOL	bLicenseOK = FALSE;
	BOOL	bActivated = FALSE;
	memset(szLicense, 0, VCA5_MAX_NUM_LICENSE*sizeof(char*));


	//1. Read License Key
	USES_CONVERSION;
	for(i = 0, LicenseCnt = 0 ; i < m_LicenseCnt ; i++){
		VCA_APP_LICENSE_INFO *pLicenseInfo = &m_LicenseInfo[i];
		TCHAR *szLicensePath = pLicenseInfo->szLicensePath;
		if(szLicensePath[0]){ //VCA survelliance or VCAOpen 
			uLicenseLength = 1024;
			if(ReadLicense(szLicensePath, szTempLicense, &uLicenseLength)){ 
				szNewLicense = new char[uLicenseLength+10];
				szLicense[LicenseCnt] = szNewLicense;
				memcpy(szLicense[LicenseCnt], szTempLicense, uLicenseLength);
				szNewLicense[uLicenseLength] = 0;
				szUSN[LicenseCnt] = T2A(pLicenseInfo->szUSN);
				pLicenseInfo->Status = LICENSE_STATUS_READ;
			}else{
				if(1 == m_LicenseCnt){ //check first run in PC, ignore file open error.
					TRACE(_T("License [%d] Path[%s] can not open"), i, szLicensePath);
				}else{
					_stprintf_s(szErrorStr, _countof(szErrorStr), _T("License [%d] Path[%s] can not open"), i, szLicensePath);
					AfxMessageBox(szErrorStr);
				}
				continue;
			}
		}else{ //VCA Presence 
			szLicense[LicenseCnt] = NULL;
			szUSN[LicenseCnt] = T2A(pLicenseInfo->szUSN);
			//Auto VCA Presence
			if(szUSN[LicenseCnt][0] == NULL){
				//Get USN by first board's USN
				BYTE*	pTempUSN = CAPPConfigure::Instance()->GetUSN(0);
				if(pTempUSN){
					pTempUSN[9] = 0;
					szUSN[LicenseCnt] = (char *)pTempUSN;
				}else{
					_stprintf_s(szErrorStr, _countof(szErrorStr), 
						_T("VCApresence license cannot be activated. If you're using a UDP card, please check it's installed correctly.\n\r If you wish to proceed and activate the system with a software license, Refer to HOW TO RUN.txt\n\r please click OK."));
					AfxMessageBox(szErrorStr);
					continue;
				}
			}

			pLicenseInfo->Status = LICENSE_STATUS_READ;
		}
		LicenseCnt++;
	}


	while(1){
		if(0 == LicenseCnt){//Failed read license 
			memset(&m_LicenseInfo, 0, sizeof(m_LicenseInfo));
			m_LicenseCnt	= 0;

			CLicenseDlg dlg(CAPPConfigure::Instance()->GetLicenseMgr(), NULL);
			dlg.SetLicenseFileName(DEFAULT_LICENSE_PATH);
			int nResponse = dlg.DoModal();
			if (nResponse != IDOK){
				_stprintf_s(szErrorStr, _countof(szErrorStr), 
					_T("Activation Failed. This product needs to be activated and execution cannot continue until the activation is successful. Please try again."));
				AfxMessageBox(szErrorStr, MB_SYSTEMMODAL);
				bActivated = FALSE;
			}else{
				bActivated = TRUE;
			}
			//when license check
			goto EXIT;
		}else{
			bLicenseOK  = TRUE;
		}
		
		
		//Activate license
		VCA5_LICENSE_INFO	LicenseInfo;
		for(i = 0  ; i < LicenseCnt ; i++){
			if(LICENSE_STATUS_NONE == m_LicenseInfo[i].Status){
				continue;
			}else if(LICENSE_STATUS_ACTIVATE == m_LicenseInfo[i].Status){
				bActivated = TRUE;
				continue;
			}
			
			LicenseInfo.szUSN		= szUSN[i];
			LicenseInfo.szDrvDllPath= T2A(szDrvDllPath);
			LicenseInfo.szLicense	= szLicense[i];
		
			if(FALSE == ActivateLicense(&LicenseInfo, NULL)){
				continue;
			}else{
				m_LicenseInfo[i].Status	= LICENSE_STATUS_ACTIVATE;
				m_LicenseInfo[i].VCA5LicenseInfo= LicenseInfo;
				m_LicenseInfo[i].UsedCount		= 0;
				if(LicenseInfo.szLicense)
					strcpy_s(m_LicenseInfo[i].szLicense, MAX_LICENSE_SIZE, LicenseInfo.szLicense);
				
				bActivated = TRUE;
			}
		}

		if(bActivated){
			goto EXIT;
		}else{
			LicenseCnt = 0;
		}
	}

EXIT:
	for(LicenseCnt = 0 ; LicenseCnt < VCA5_MAX_NUM_LICENSE ;LicenseCnt++){
		if(szLicense[LicenseCnt]) delete [] szLicense[LicenseCnt];
	}

	return bActivated;
}


BOOL	CLicenseMgr::CheckLicense(VCA5_LICENSE_INFO *pLicenseInfo)
{
	return ActivateLicense(pLicenseInfo, NULL);
}


BOOL	CLicenseMgr::AddLicense(TCHAR *szLicensePath, BOOL bSameFileAsFail)
{
	char	szTempLicense[1024];
	DWORD	i, uLicenseLength = 1024;
	memset(szTempLicense, 0, sizeof(szTempLicense));

	CString s;

	//check same license file.
		for(i = 0 ; i < VCA5_MAX_NUM_LICENSE ; i++){
			if(0 == _tcscmp(szLicensePath, m_LicenseInfo[i].szLicensePath)){
			if(bSameFileAsFail){
				s.Format( _T("AddLicense Fail : using same license file [%s] \n"), szLicensePath);
				AfxMessageBox( s, MB_OK | MB_ICONEXCLAMATION );
				return FALSE;
			}else{
				m_LicenseInfo[i].Status = LICENSE_STATUS_ACTIVATE; 
				return TRUE;
			}
		}
	}

	if(!ReadLicense(szLicensePath, szTempLicense, &uLicenseLength)){
		s.Format(_T("AddLicense Fail : Can not read license file [%s] \n"), szLicensePath);
		AfxMessageBox( s, MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}
	
	//Activate license
	VCA5_LICENSE_INFO	LicenseInfo;
	BOOL	bActivated, bOK;

	
	LicenseInfo.szUSN		= NULL;
	LicenseInfo.szDrvDllPath= NULL;
	LicenseInfo.szLicense	= szTempLicense;
	bActivated = FALSE;

	bOK = ActivateLicense(&LicenseInfo, &bActivated);
	if(bActivated || bOK){
		if(bActivated){
			//find license in removed items.
			for(i = 0 ; i < VCA5_MAX_NUM_LICENSE ; i++){
				if(m_LicenseInfo[i].Status == LICENSE_STATUS_REMOVED){
					if(0 == memcmp(szTempLicense, m_LicenseInfo[i].szLicense , 100)){
						m_LicenseInfo[i].Status = LICENSE_STATUS_ACTIVATE;
					}
				}
			}
		}else{
			m_LicenseInfo[m_LicenseCnt].Status			= LICENSE_STATUS_ACTIVATE;
			m_LicenseInfo[m_LicenseCnt].VCA5LicenseInfo	= LicenseInfo;
			m_LicenseInfo[m_LicenseCnt].UsedCount		= 0;

			_tcscpy(m_LicenseInfo[m_LicenseCnt].szLicensePath, szLicensePath);
			memset(m_LicenseInfo[m_LicenseCnt].szUSN, 0, sizeof(TCHAR)*MAX_USN_SIZE);
			
			m_LicenseCnt++; 
		}
		return TRUE;
	}

	return FALSE;
}


BOOL	CLicenseMgr::RemoveLicense(DWORD LicenseID)
{
	//1. Find License
	DWORD i,j;
	for(i = 0, j = 0 ; i < m_LicenseCnt ; i++){
		if(m_LicenseInfo[i].VCA5LicenseInfo.nLicenseID == LicenseID){
			m_LicenseInfo[i].Status = LICENSE_STATUS_REMOVED;	
		}
	}
	return TRUE;
}


BOOL	CLicenseMgr::ActivateLicense(VCA5_LICENSE_INFO* pLicenseInfo, BOOL* pbActivated)
{
	if(!m_pVCA5API) return FALSE;

	if(FALSE == m_pVCA5API->VCA5Activate(1, pLicenseInfo)){
		USES_CONVERSION;
		CString sErr = _T("Unknown error");
		VCA5_ERROR_CODE_ITEM item;
		if( m_pVCA5API->VCA5GetLastErrorCode( &item ) ){
			switch( VCA5_EXTRACT_ERRCODE2( item.ErrorCode ) ){

				case VCA5_ERRCMN_F_LICENSE_EXPIRED:{
					CString s;
					s.Format( _T("license has expired: %s"), A2T( item.AuxMsg ) );
					AfxMessageBox( s, MB_OK | MB_ICONEXCLAMATION );
					break;
				}

				// Intentional durchfall...
				case VCA5_ERRCMN_F_ALREADY_ACTIVATED:{
					// Already activated
					if(pbActivated)*pbActivated = TRUE;
					break;
				}
				
				default:{
					CString sErr = CString( A2T( item.AuxMsg ) );
					CString sMsg;
					sMsg.Format( _T("Unable to activate VCA5: %s"), sErr );
					AfxMessageBox( sMsg );
					break;
				}
			}
		}
		return FALSE;
	}
	return TRUE;
}