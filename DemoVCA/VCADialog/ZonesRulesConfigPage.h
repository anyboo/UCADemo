#pragma once
#include "..\resource.h"
#include "ZoneTreeCtrl.h"
#include "..\AlarmListCtrl\AlarmListCtrl.h"
#include "ConfigPage.h"

class CVCADialog;

// CZonesRulesConfigPage dialog

class CZonesRulesConfigPage : public CConfigPage
{
	DECLARE_DYNAMIC(CZonesRulesConfigPage)

public:
	CZonesRulesConfigPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CZonesRulesConfigPage();

// Dialog Data
	enum { IDD = IDD_ZONESRULESCONFIGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnApply();


	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnDestroy();
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
	
protected:
	CZoneTreeCtrl	m_TreeCtrl;
	AlarmListCtrl	m_AlarmListCtrl;
	CWnd		*m_pOldParent;
public:

};
