#pragma once
#include "ConfigPage.h"
#include "afxcmn.h"
#include "APPConfigure.h"

// CLicenseConfigPage dialog

class CLicenseConfigPage : public CConfigPage
{
	DECLARE_DYNAMIC(CLicenseConfigPage)

public:
	CLicenseConfigPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLicenseConfigPage();

// Dialog Data
	enum { IDD = IDD_LICENSECONFIGPAGE };
	void	UpdateData(BOOL bSave = FALSE);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnBnClickedBtnLicenseRegister();
	afx_msg void OnBnClickedBtnLicenseUnregister();
	afx_msg void OnBnClickedButtonAssign();
	afx_msg void OnBnClickedButtonRemove();


private:
	void	Update(BOOL bSave = FALSE);
	void	UpdateLicenseInfo();
	void	UpdateEngineLicenseInfo();

	int		GetFirstSelectItem(CListCtrl *pListCtrl);

	VCA5_APP_ENGINE_INFO* m_pEngineInfo;
	
};
