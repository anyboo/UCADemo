#pragma once
#include "VCA5CoreLib.h"

#define MAX_USN_SIZE			16
#define	VCA_MAX_SOURCE_PATH		512
#define DEFAULT_LICENSE_PATH	_T("license.lic")
#define MAX_LICENSE_SIZE		1024

typedef enum
{
	LICENSE_STATUS_NONE		= 0,
	LICENSE_STATUS_READ		= 1,
	LICENSE_STATUS_ACTIVATE = 2,
	LICENSE_STATUS_REMOVED	= 3
}VCA_LICENSE_STATUS;


typedef struct 
{
	VCA_LICENSE_STATUS	Status;
	VCA5_LICENSE_INFO	VCA5LicenseInfo;
	DWORD				UsedCount;
	DWORD				TmpUsedCount;
	TCHAR				szLicensePath[VCA_MAX_SOURCE_PATH];
	TCHAR				szUSN[MAX_USN_SIZE];
	char				szLicense[MAX_LICENSE_SIZE];
}VCA_APP_LICENSE_INFO;


class CLicenseMgr
{
public:
	CLicenseMgr(void);
	~CLicenseMgr(void);

	void	SetVCALib(IVCA5 *pVCA5API){m_pVCA5API = pVCA5API;}
	void	SetLicenseInfo(DWORD count, VCA_APP_LICENSE_INFO *pLicenseInfo);

	DWORD	GetLicenseCnt(){return m_LicenseCnt;}
	VCA_APP_LICENSE_INFO* GetLicenseInfo(){ return 	m_LicenseInfo;}

	VCA_APP_LICENSE_INFO *GetLicenseInfo(DWORD index){
		return (m_LicenseCnt > index)?&m_LicenseInfo[index]:NULL;}
	
	VCA_APP_LICENSE_INFO *GetLicenseInfoById(DWORD id);

	BOOL	Activate(TCHAR *szDrvDllPath);

	BOOL	CheckLicense(VCA5_LICENSE_INFO *pLicenseInfo);

	BOOL	AddLicense(TCHAR *szPath, BOOL bSameFileAsFail);
	BOOL	RemoveLicense(DWORD LicenseID);

	IVCA5*	GetIVCA5(){return m_pVCA5API;}

private:

	IVCA5*					m_pVCA5API;
	DWORD					m_LicenseCnt;
	VCA_APP_LICENSE_INFO	m_LicenseInfo[VCA5_MAX_NUM_LICENSE];//For temporary 
	
	BOOL	ActivateLicense(VCA5_LICENSE_INFO* pLicenseInfo, BOOL *pbActivated);
};
