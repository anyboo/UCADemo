// VCADialog\ConfigTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigTabCtrl.h"
#include "ConfigPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CConfigTabCtrl

IMPLEMENT_DYNAMIC(CConfigTabCtrl, CTabCtrl)

CConfigTabCtrl::CConfigTabCtrl()
{
	m_pActiveDlg = NULL;
}

CConfigTabCtrl::~CConfigTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CConfigTabCtrl, CTabCtrl)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT( TCN_SELCHANGE, &CConfigTabCtrl::OnSelChange)
END_MESSAGE_MAP()



// CConfigTabCtrl message handlers

void CConfigTabCtrl::Activate( BOOL bActivate )
{
	if( bActivate )
	{
		CreateContents( GetCurSel() );
	}
	else
	{
		// Closing, tidy up..
		if( m_pActiveDlg )
		{
			m_pActiveDlg->DestroyWindow();
			m_pActiveDlg = NULL;
		}
	}
}

void CConfigTabCtrl::AddTab( LPCTSTR lpszName, unsigned int uiResourceId, CConfigPage *pDlg)
{
	TABENTRY te;
	te.pDlg	= pDlg;
	te.uiResourceId	= uiResourceId;

	unsigned int uId = InsertItem( GetItemCount(), lpszName );

	m_Tabs.insert( std::pair< unsigned int, TABENTRY>( uId, te ) );
}

BOOL CConfigTabCtrl::DeleteAllItems()
{
	CTabCtrl::DeleteAllItems();

	m_Tabs.clear();

	return TRUE;
}

void CConfigTabCtrl::OnSelChange( NMHDR *pNotifyStruct, LRESULT *pResult )
{
	CreateContents( GetCurSel() );

	*pResult = 0;
}

void CConfigTabCtrl::OnSize( UINT nType, int cx, int cy )
{
	SizeActiveDlg();
}

void CConfigTabCtrl::SizeActiveDlg()
{
	if( m_pActiveDlg )
	{
		// Use first tab for sizing
		std::map< unsigned int, TABENTRY >::iterator it = m_Tabs.begin();

		CRect rcItem, rcClient;
		GetItemRect( it->first, &rcItem );
		GetClientRect( &rcClient );

		CRect rcPage;
		rcPage.left = rcClient.left + 1;
		rcPage.right = rcClient.right - 5;
		rcPage.bottom = rcClient.bottom - 5;
		rcPage.top = rcItem.bottom + 5;
		m_pActiveDlg->MoveWindow( rcPage );

		RedrawWindow();
	}
}

void CConfigTabCtrl::CreateContents(unsigned int uId)
{
	if( !GetSafeHwnd() )
	{
		return;
	}

	// Destroy active dialog
	if( m_pActiveDlg )
	{
		m_pActiveDlg->DestroyWindow();
		m_pActiveDlg = NULL;
	}

	std::map< unsigned int, TABENTRY >::iterator it = m_Tabs.find( uId );

	if( it != m_Tabs.end() )
	{
		if( it->second.pDlg )
		{
			m_pActiveDlg = it->second.pDlg;
			m_pActiveDlg->Create( it->second.uiResourceId, this );
			m_pActiveDlg->ShowWindow( SW_SHOW );

			SizeActiveDlg();
		}
	}
}
