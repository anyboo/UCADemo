#pragma once
#include "ConfigPage.h"

// CSceneChangeConfigPage dialog

class CVCADataMgr;
class CSceneChangeConfigPage : public CConfigPage
{
	DECLARE_DYNAMIC(CSceneChangeConfigPage)

public:
	CSceneChangeConfigPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSceneChangeConfigPage();

// Dialog Data
	enum { IDD = IDD_SCENECHANGECONFIGPAGE };

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
	afx_msg void OnEnChangeMode();
	afx_msg void OnBnClickedCheckDnColor();
	afx_msg void OnBnClickedCheckDnRim();
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	CVCADataMgr		*m_pDataMgr;
	CWnd			*m_pOldParent;
	BOOL			m_bAlive;
	DWORD			m_OldDisplay;
public:
	afx_msg void OnDestroy();
};
