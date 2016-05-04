#pragma once

#include "ProgressDlg.h"



// CStartupWnd

class CStartupWnd : public CWinThread
{
	DECLARE_DYNCREATE(CStartupWnd)

protected:
	CStartupWnd();           // protected constructor used by dynamic creation
	virtual ~CStartupWnd();

public:
	void Start();
	void Stop();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()

private:
	CProgressDlg	m_dlg;
};


