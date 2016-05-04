// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "ProgressDlg.h"
#include "Utils/MemDC.h"


// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CWnd)

CProgressDlg::CProgressDlg(CWnd* pParent )
	: CWnd()
{
	m_iIdx = 0;
	m_pParent = pParent;
}

CProgressDlg::~CProgressDlg()
{
	m_ImageList.DeleteImageList();
	m_Bmp.DeleteObject();
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CWnd::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

int CProgressDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	m_ImageList.Create( 64, 64, ILC_COLOR32 | ILC_MASK, 0, 0 );
	m_Bmp.LoadBitmap( IDB_PROGRESS );
	m_ImageList.Add( &m_Bmp, RGB(0,0,0) );

	SetWindowText(_T("Initializing..... PC VCA DEMO Program"));
	SetTimer( 1, 100, NULL );

	return 0;
}

void CProgressDlg::OnTimer( UINT_PTR nIDEvent )
{
	if( 1 == nIDEvent )
	{
		if( ++m_iIdx >= m_ImageList.GetImageCount() )
		{
			m_iIdx = 0;
		}

		RedrawWindow();
	}
}

// CProgressDlg message handlers
BOOL CProgressDlg::OnEraseBkgnd( CDC *pDC )
{
	return TRUE;
}


void CProgressDlg::OnPaint()
{
	CPaintDC dc( this );
	CMemDC dcMem( &dc );
	CRect	rc;
	GetClientRect( &rc );

	dcMem.FillSolidRect( &rc, RGB(0,0,0) );

	POINT pt;
	pt.x = (rc.Width() / 2) - 32;
	pt.y = (rc.Height() / 2) - 32;

	m_ImageList.Draw( &dcMem, m_iIdx, pt, ILD_NORMAL );

	int iStops = m_iIdx / (m_ImageList.GetImageCount()/ 5);

	CString sTxt = _T("Initializing");
	for( int i = 0; i < iStops; i++ )
	{
		sTxt += _T(".");
	}

	CSize sz = dcMem.GetTextExtent( _T("Initializing.....") );

	CRect rcTxt = rc;
	rcTxt.top = rcTxt.bottom - 30;
	rcTxt.left = (rc.Width() - sz.cx)/2;
	rcTxt.right = rcTxt.left + sz.cx;
	dcMem.SetTextColor( RGB(0xFF, 0xFF, 0xFF ) );
	dcMem.DrawText( sTxt, rcTxt, DT_SINGLELINE | DT_VCENTER );

	// Blits when it goes out of scope
	
}