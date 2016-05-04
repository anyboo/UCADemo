// StartupWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "StartupWnd.h"

#include "ProgressDlg.h"


// CStartupWnd

IMPLEMENT_DYNCREATE(CStartupWnd, CWinThread)

CStartupWnd::CStartupWnd()
{
}

CStartupWnd::~CStartupWnd()
{
}

BOOL CStartupWnd::InitInstance()
{
	// TODO:  perform and per-thread initialization here

	int w = GetSystemMetrics( SM_CXSCREEN );
	int h = GetSystemMetrics( SM_CYSCREEN );
	CRect rcScreen( (w/2) - 100, (h/2) - 100, (w/2)+100, (h/2)+100);

	LPCTSTR lpszClass = AfxRegisterWndClass( NULL );
	m_dlg.CreateEx( NULL,  lpszClass, _T("Name"), WS_POPUP | WS_VISIBLE, rcScreen, 0, 0);
	return TRUE;
}

int CStartupWnd::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

void CStartupWnd::Start()
{
//	AfxBeginThread( RUNTIME_CLASS( CStartupWnd ) );
}

void CStartupWnd::Stop()
{
	PostQuitMessage( 0 );
}

BEGIN_MESSAGE_MAP(CStartupWnd, CWinThread)
END_MESSAGE_MAP()


// CStartupWnd message handlers
