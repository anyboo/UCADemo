#pragma once
#include "ConfigPage.h"

// CTamperConfigPage dialog

class CVCADataMgr;
class CTamperConfigPage : public CConfigPage
{
	DECLARE_DYNAMIC(CTamperConfigPage)

public:
	CTamperConfigPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTamperConfigPage();

// Dialog Data
	enum { IDD = IDD_TAMPERCONFIGPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void SyncToDataMgr();

	DECLARE_MESSAGE_MAP()

protected:
	BOOL	m_bPopulating;

protected:
	afx_msg void OnEnChangeTamperTimeEdit();
	afx_msg void OnEnChangeTamperAreaEdit();
	afx_msg void OnBnClickedCheckSuppressLight();
	afx_msg void OnChangeTamperEnableCheck();
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	CVCADataMgr		*m_pDataMgr;
	CWnd			*m_pOldParent;
	BOOL			m_bAlive;
	DWORD			m_OldDisplay;
public:
	afx_msg void OnDestroy();
};
