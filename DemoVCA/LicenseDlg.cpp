// LicenseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "LicenseDlg.h"
#include "VCA5CoreLib.h"
#include "wm_user.h"


BOOL WriteLicense(TCHAR *szFIleName, char *szLincens, DWORD nLen)
{
	FILE	*pFile = NULL;
	errno_t err = _tfopen_s(&pFile, szFIleName, _T("w+b"));
	if(err) {
		return FALSE;
	}
		
	DWORD writesize;

	writesize = fwrite(szLincens, 1, nLen, pFile);
	if(writesize != nLen) return FALSE;
	
	fclose(pFile);
	return TRUE;
}

// CLicenseDlg dialog

IMPLEMENT_DYNAMIC(CLicenseDlg, CDialog)

CLicenseDlg::CLicenseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLicenseDlg::IDD, pParent)
{
	CLicenseDlg(NULL, NULL);
	
}

CLicenseDlg::CLicenseDlg(CLicenseMgr *pLicenseMgr, CWnd* pParent)
	: CDialog(CLicenseDlg::IDD, pParent)
{
	memset(&m_LicenseInfo, 0, sizeof(m_LicenseInfo));
	m_pLicenseMgr		= pLicenseMgr;
	m_LicenseInputType	= LICENSE_INPUT_DIRECT;
	m_bSameFileAsFail	= TRUE;
	memset(m_LicenseFileName, 0 ,sizeof(m_LicenseFileName));
}


CLicenseDlg::~CLicenseDlg()
{
}

void CLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLicenseDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CLicenseDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_LICENSE_FILE, &CLicenseDlg::OnBnClickedButtonLicenseFile)
	ON_BN_CLICKED(IDC_RADIO_DIRECT, &CLicenseDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CLicenseDlg::OnBnClickedRadio)
END_MESSAGE_MAP()


const TCHAR *szLicenseInfo1 = _T("This product needs to be licensed before it can be used. Please contact your supplier to obtain an activation key. \n\r \
You will need to send them the code below.");
const TCHAR *szLicenseInfo2 = _T("They will send you an activation code. Please enter it in the space below or select license file");


// CLicenseDlg message handlers
BOOL CLicenseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(&this->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	SetDlgItemText(IDC_STATIC_LICENSE_INFO_1, szLicenseInfo1);
	SetDlgItemText(IDC_STATIC_LICENSE_INFO_2, szLicenseInfo2);
	

	char	szHWGUID[1024];
	memset(szHWGUID, 0, 1024);

	VCA5_HWGUID_INFO		guidInfo;
	guidInfo.szUSN			= NULL;	
	guidInfo.szDrvDllPath	= NULL;
	guidInfo.szGUID			= szHWGUID;

	int cmd = VCA5_QR_GETHWGUIDOPEN;
	IVCA5*	pVCA5API = m_pLicenseMgr->GetIVCA5();

	if(pVCA5API->VCA5QueryInfo(cmd, sizeof(VCA5_HWGUID_INFO),(void*)&guidInfo)){
		SetDlgItemTextA(m_hWnd, IDC_EDIT_HWGUID ,guidInfo.szGUID);
	}else{
		SetDlgItemText(IDC_EDIT_HWGUID ,_T("Failed to get HWGUID (You might need to run this as admin user)"));
	}

//	GetDlgItem(IDOK)->EnableWindow(FALSE);
	CheckDlgButton(IDC_RADIO_DIRECT, BST_CHECKED);
	OnBnClickedRadio();

	// FRIG, eugh!
	AfxGetMainWnd()->SendMessage( WM_SHOW_WAIT_WND, 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CLicenseDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	TCHAR	strTemp[1024];
		
	if(IsDlgButtonChecked(IDC_RADIO_DIRECT)){
		char	szLicense[1024];
		memset(szLicense, 0, 1024);
		
		GetDlgItemTextA(m_hWnd, IDC_EDIT_LICENSE_DIRECT, szLicense, 1024);

		if(szLicense[0] == 0){
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			AfxMessageBox(_T("Empty license. Activation failed.\n"), MB_ICONINFORMATION | MB_OK );
			return;

		}

		if(m_LicenseFileName[0] == NULL){
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			DWORD NameIndex = m_pLicenseMgr->GetLicenseCnt();

			while(1){
				_stprintf_s(m_LicenseFileName, _T("license_%02d.lic"), NameIndex++);
				hFind = FindFirstFile(m_LicenseFileName, &FindFileData);
				if(hFind == INVALID_HANDLE_VALUE) break;
				FindClose(hFind);
			}
		}

		WriteLicense(m_LicenseFileName, szLicense, strlen(szLicense));
	}else{
		GetDlgItemText(IDC_EDIT_LICENSE_FILE, m_LicenseFileName, MAX_PATH);
	}


	BeginWaitCursor();
	BOOL bOk = m_pLicenseMgr->AddLicense(m_LicenseFileName, m_bSameFileAsFail);
	EndWaitCursor();

	if(!bOk){
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		AfxMessageBox(_T("Failed to activate license. Please check the license code.\n"), MB_ICONINFORMATION | MB_OK );
		SetDlgItemText(IDC_EDIT_LICENSE_FILE, NULL);

		SetDlgItemText(IDC_EDIT_LICENSE_FILE, NULL);
		SetDlgItemText(IDC_EDIT_LICENSE_DIRECT, NULL);
	}else{
		USES_CONVERSION;	

		DWORD LatestLicenseIndex = m_pLicenseMgr->GetLicenseCnt() -1;
		VCA_APP_LICENSE_INFO* pLicenseInfo = m_pLicenseMgr->GetLicenseInfo(LatestLicenseIndex);

		_stprintf_s(strTemp, _T("License details \n%s"), A2T(pLicenseInfo->VCA5LicenseInfo.szLicenseDesc) );
		AfxMessageBox( strTemp, MB_ICONINFORMATION | MB_OK );
		OnOK();
	}
}


void CLicenseDlg::OnClose()
{
	// FRIG, eugh!
	AfxGetMainWnd()->SendMessage( WM_SHOW_WAIT_WND, 1 );
}


void CLicenseDlg::OnBnClickedButtonLicenseFile()
{
	TCHAR szFileter[] = _T("License File | *.lic\0All file\0*.*\0");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_READONLY|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR, szFileter);

    if (IDOK != dlg.DoModal()) return;

	SetDlgItemText(IDC_EDIT_LICENSE_FILE, dlg.GetPathName());

/*
	TCHAR szCurPath[MAX_PATH];
	TCHAR szCurPath2[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, szCurPath);
	CString strPath = dlg.GetPathName();
	CString strFileName = dlg.GetFileName();

	int FilePos = strPath.GetLength() - strFileName.GetLength();
	_tcsncpy(szCurPath2, strPath,FilePos);
	szCurPath2[FilePos-1] = 0;
	
	if(_tcscmp(szCurPath2, szCurPath) == 0){
		strFileName = dlg.GetFileName();
	}else{
		strFileName = strPath;
	}
*/
}

void CLicenseDlg::OnBnClickedRadio()
{
	if(IsDlgButtonChecked(IDC_RADIO_DIRECT)){
		GetDlgItem(IDC_EDIT_LICENSE_DIRECT)->EnableWindow(TRUE);

		GetDlgItem(IDC_EDIT_LICENSE_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_LICENSE_FILE)->EnableWindow(FALSE);
		m_LicenseInputType = LICENSE_INPUT_DIRECT;
	}else{
		GetDlgItem(IDC_EDIT_LICENSE_DIRECT)->EnableWindow(FALSE);

		GetDlgItem(IDC_EDIT_LICENSE_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_LICENSE_FILE)->EnableWindow(TRUE);
		m_LicenseInputType = LICENSE_INPUT_FILE;
	}
}
