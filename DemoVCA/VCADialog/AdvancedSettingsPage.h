#pragma once
#include "../resource.h"
#include "ConfigPage.h"

// CAdvancedSettingsPage dialog

class CVCADataMgr;
class CVCADialog;
class CAdvancedSettingsPage : public CConfigPage
{
	DECLARE_DYNAMIC(CAdvancedSettingsPage)

public:
	CAdvancedSettingsPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAdvancedSettingsPage();



// Dialog Data
	enum { IDD = IDD_ADVANCED_SETTING_DIALOG };

protected:
	void SyncToDataMgr();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()

private:

	BOOL		m_bPopulating;

public:
	afx_msg void OnEnChangeValue();
};
