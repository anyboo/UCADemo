// ./VCADialog/ConfigPage.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigPage.h"
#include "wm_user.h"
#include "VCADialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CConfigPage dialog

IMPLEMENT_DYNAMIC(CConfigPage, CDialog)

CConfigPage::CConfigPage(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
}

CConfigPage::~CConfigPage()
{
}

void CConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConfigPage, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CConfigPage message handlers
HBRUSH CConfigPage::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	switch( nCtlColor )
	{
	case CTLCOLOR_EDIT:
	case CTLCOLOR_DLG:
	case CTLCOLOR_LISTBOX:
	case CTLCOLOR_STATIC:
		return GetSysColorBrush( COLOR_WINDOW );
	}

	return CDialog::OnCtlColor( pDC, pWnd, nCtlColor );
}

BOOL CConfigPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	if(IDOK == wmId || IDCANCEL  == wmId){
		return TRUE;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}
