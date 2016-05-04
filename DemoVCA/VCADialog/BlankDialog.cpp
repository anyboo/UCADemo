// VCADialog\BlankDialog.cpp : implementation file
//

#include "stdafx.h"

#include "BlankDialog.h"


// CBlankDialog dialog

IMPLEMENT_DYNAMIC(CBlankDialog, CDialog)

CBlankDialog::CBlankDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBlankDialog::IDD, pParent)
{

}

CBlankDialog::~CBlankDialog()
{
}

void CBlankDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBlankDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


BOOL CBlankDialog::OnEraseBkgnd( CDC *pDC )
{
	return FALSE;
}

// CBlankDialog message handlers

void CBlankDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rc;
	GetClientRect( &rc );

	dc.FillSolidRect(rc,  RGB(0,0,0) );

	CString s = _T("VCA ENGINE NOT ATTACHED");
	dc.SetTextColor( RGB( 255, 255, 255) );

	dc.DrawText( s, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
}

