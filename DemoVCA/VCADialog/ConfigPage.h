#pragma once

#include "../resource.h"


// CConfigPage dialog
class CVCADialog;

class CConfigPage : public CDialog
{
	DECLARE_DYNAMIC(CConfigPage)

public:
	CConfigPage(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigPage();

	void SetVCADialog( CVCADialog *pVCADialog ) { m_pVCADialog = pVCADialog; }

// Dialog Data
	enum { IDD = IDD_CONFIGPAGE };

protected:


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	CVCADialog	*m_pVCADialog;
};
