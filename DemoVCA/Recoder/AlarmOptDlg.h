#pragma once
#include "afxwin.h"

class CAlaramOptDlg : public CDialog
{
	DECLARE_DYNAMIC(CAlaramOptDlg)

public:
	CAlaramOptDlg(CWnd* pParent = NULL);
	virtual ~CAlaramOptDlg();

	enum { IDD = IDD_RECORDING_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	DWORD	m_dwPeriod;
	BOOL	m_bEnable;
	CString m_Path;
	afx_msg void OnBnClickedEnable();
	afx_msg void OnBnClickedOk();
};
