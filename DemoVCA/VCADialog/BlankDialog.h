#pragma once
#include "../resource.h"


// CBlankDialog dialog

class CBlankDialog : public CDialog
{
	DECLARE_DYNAMIC(CBlankDialog)

public:
	CBlankDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBlankDialog();

// Dialog Data
	enum { IDD = IDD_BLANK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
};
