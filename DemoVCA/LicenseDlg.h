#pragma once

#include "APPConfigure.h"

// CLicenseDlg dialog
class CLicenseDlg : public CDialog
{
	DECLARE_DYNAMIC(CLicenseDlg)
public:
	enum { IDD = IDD_DIALOG_LICENSE };
	typedef enum{LICENSE_INPUT_DIRECT, LICENSE_INPUT_FILE}eLicenseInputType;
	
	CLicenseDlg(CWnd* pParent);   // standard constructor
	CLicenseDlg(CLicenseMgr *pLicenseMgr, CWnd* pParent);   // standard constructor
	virtual ~CLicenseDlg();

	void	SetLicenseFileName(TCHAR *szFileName){_tcscpy(m_LicenseFileName, szFileName);}
	void	SetLicenseMgr(CLicenseMgr *pLicenseMgr){m_pLicenseMgr = pLicenseMgr;}
	VCA_APP_LICENSE_INFO	*GetLicenseInfo(){return &m_LicenseInfo;}
	eLicenseInputType		GetLicenseInputType(){return m_LicenseInputType;}

	void	SetSameFileAsFail(BOOL bTrue){m_bSameFileAsFail = bTrue;}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonLicenseFile();
	afx_msg void OnBnClickedRadio();

private:
	CLicenseMgr*			m_pLicenseMgr;
	VCA_APP_LICENSE_INFO	m_LicenseInfo;
	eLicenseInputType		m_LicenseInputType;
	BOOL					m_bSameFileAsFail;
	TCHAR					m_LicenseFileName[MAX_PATH];
};
