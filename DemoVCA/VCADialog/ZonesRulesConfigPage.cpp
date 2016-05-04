// ./VCADialog/ZonesRulesConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ZonesRulesConfigPage.h"
#include "../VCAEngine/VCAEngine.h"
#include "VCADialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// CZonesRulesConfigPage dialog

IMPLEMENT_DYNAMIC(CZonesRulesConfigPage, CConfigPage)

CZonesRulesConfigPage::CZonesRulesConfigPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CZonesRulesConfigPage::IDD, pParent)
{
	m_pVCADialog	= NULL;
	m_pOldParent	= NULL;
}

CZonesRulesConfigPage::~CZonesRulesConfigPage()
{
}

void CZonesRulesConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CZonesRulesConfigPage, CConfigPage)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CZonesRulesConfigPage message handlers

BOOL CZonesRulesConfigPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT( m_pVCADialog );

	unsigned int uiFunction = 0xFFFFFFFF;

	CEngine *pEngine = m_pVCADialog->GetEngine();
	m_TreeCtrl.Setup(this, m_pVCADialog->GetVCADataMgr());
	m_TreeCtrl.ShowWindow( SW_SHOW );

	m_TreeCtrl.DeleteAllItems();
	m_TreeCtrl.RedrawAllZoneListTree();

	m_pOldParent = m_pVCADialog->SetParent( this );

	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_CONFIGURE );
	m_pVCADialog->ShowWindow( SW_SHOW );

	CRect rc (0,0,100,100 );

	m_AlarmListCtrl.Create(WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT, rc, this, IDC_ALARMLIST );
	m_AlarmListCtrl.SetExtendedStyle( m_AlarmListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_AlarmListCtrl.Initialize();

	int iEngId = -1;
	if( pEngine )
	{
		iEngId = pEngine->GetEngId();
	}

	m_AlarmListCtrl.SetVCADataMgr( iEngId, m_pVCADialog->GetVCADataMgr() );

	if( pEngine )
	{
		pEngine->RegisterVCAEventSink( &m_AlarmListCtrl );
	}

	return TRUE;
}


void CZonesRulesConfigPage::OnSize( UINT nType, int cx, int cy )
{
	// Right hand 3rd to zone/tree ctrl
	CRect rcClient;
	GetClientRect( &rcClient );

	int VCADialogWidth,TreeCtrlWidth	= (rcClient.Width() *3)/ 8;
	TreeCtrlWidth	= (TreeCtrlWidth < CZoneTreeCtrl::MAX_TREECTRL_WIDTH)?TreeCtrlWidth:CZoneTreeCtrl::MAX_TREECTRL_WIDTH;
	VCADialogWidth	= rcClient.Width()-TreeCtrlWidth;


	if( m_TreeCtrl.GetSafeHwnd() ){
		m_TreeCtrl.MoveWindow( VCADialogWidth, 0, TreeCtrlWidth, (rcClient.Height()*3) / 4 );
	}

	if( m_pVCADialog ){
		m_pVCADialog->MoveWindow( 0, 0, VCADialogWidth, (rcClient.Height()*3) / 4 );
	}

	if( m_AlarmListCtrl.GetSafeHwnd() ){
		m_AlarmListCtrl.MoveWindow( 0, (rcClient.Height() * 3)/4, rcClient.Width(), rcClient.Height() / 4 );
	}
}

void CZonesRulesConfigPage::OnDestroy()
{
	// Put VCA Dialog parent back to what it was
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_PREVIEW );
	m_pVCADialog->SetParent( m_pOldParent );

	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	m_TreeCtrl.Destroy();

	CEngine *pEngine = m_pVCADialog->GetEngine();

	if( pEngine )
	{
		pEngine->UnregisterVCAEventSink( &m_AlarmListCtrl );
	}

	m_AlarmListCtrl.DestroyWindow();

	m_pVCADialog->ShowWindow( SW_HIDE );
}

void CZonesRulesConfigPage::OnApply()
{
	m_pVCADialog->ApplyZoneSetting();
}

BOOL CZonesRulesConfigPage::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
	NMHDR *pNMHDR = (NMHDR *)lParam;
	if(pNMHDR->idFrom == IDC_TREELIST)
	{
		switch(pNMHDR->code)
		{
		case TVN_SELCHANGED:
			m_TreeCtrl.OnSelChanged(pNMHDR, pResult);
			break;
		case TVN_CBSTATECHANGED:
			m_TreeCtrl.OnCbStateChanged(pNMHDR, pResult);
			break;
		case NM_CLICK:
			m_TreeCtrl.OnItemlClick(pNMHDR, pResult, m_hWnd);
			break;
		case NM_DBLCLK:
			m_TreeCtrl.OnItemDblClick(pNMHDR, pResult, m_hWnd);
			break;
		case TVN_BEGINLABELEDIT:
			m_TreeCtrl.OnBeginLabelEdit(pNMHDR, pResult);
			break;
		case TVN_ENDLABELEDIT:
			m_TreeCtrl.OnEndLabelEdit(pNMHDR, pResult);
			break;
		case TVN_ITEMEXPANDING:
			m_TreeCtrl.OnItemExpanding(pNMHDR, pResult);
			break;
		}
	}

	return CConfigPage::OnNotify(wParam, lParam, pResult);
}
