// ./VCADialog/TamperConfigPage.cpp : implementation file
//

#include "stdafx.h"
#include "TamperConfigPage.h"
#include "VCADialog.h"
#include "../Render/VCARender.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CTamperConfigPage dialog

IMPLEMENT_DYNAMIC(CTamperConfigPage, CConfigPage)

CTamperConfigPage::CTamperConfigPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CTamperConfigPage::IDD, pParent)
{
	m_bPopulating	= FALSE;
	m_bAlive		= FALSE;
}

CTamperConfigPage::~CTamperConfigPage()
{
}

void CTamperConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}

BOOL CTamperConfigPage::OnInitDialog()
{
	m_bPopulating = TRUE;
	CConfigPage::OnInitDialog();

	m_pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_TAMPER_INFO *pTamperInfo = m_pDataMgr->GetTamperInfo();
	CheckDlgButton(IDC_CHECK_TAMPER, pTamperInfo->ulEnabled ?BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(IDC_TAMPER_TIME_EDIT, pTamperInfo->ulAlarmTimeout);
	SetDlgItemInt(IDC_TAMPER_AREA_EDIT, pTamperInfo->ulAreaThreshold);
	CheckDlgButton(IDC_CHECK_SUPPRESS_LIGHT, pTamperInfo->ulLowLightEnabled ?BST_CHECKED : BST_UNCHECKED);

	m_OldDisplay	= m_pDataMgr->GetDisplayFlags();
	m_pDataMgr->SetDisplayFlags(IVCARender::DISPLAY_TAMPER_ALARM);
	m_pOldParent	= m_pVCADialog->SetParent( this );
	
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_TAMPER );
	m_pVCADialog->ShowWindow( SW_SHOW );

	VCA5_APP_ADVANCED_INFO* pAdvancedInfo =  m_pDataMgr->GetAdvancedInfo();
	BOOL bEnableTamper = (pAdvancedInfo->TrackerParams.bMovObjEnabled || 
							pAdvancedInfo->TrackerParams.bAbObjEnabled || 
							pAdvancedInfo->TrackerParams.bAdvTraEnabled);
	GetDlgItem(IDC_CHECK_TAMPER)->EnableWindow(bEnableTamper);

	OnChangeTamperEnableCheck();

	m_bPopulating	= FALSE;
	m_bAlive		= TRUE;

	return TRUE;
}


void CTamperConfigPage::OnSize(UINT nType, int cx, int cy)
{
	if( !m_bAlive )
	{
		return;
	}

	// Put calib ctl on RHS
	CRect rcClient;
	GetClientRect( &rcClient );

	if( m_pVCADialog )
	{
		CRect rc( 300, 20, rcClient.Width()-20 , rcClient.bottom - 100);
		m_pVCADialog->MoveWindow( rc );
	}
}


BEGIN_MESSAGE_MAP(CTamperConfigPage, CConfigPage)
		ON_BN_CLICKED(IDC_CHECK_TAMPER, &CTamperConfigPage::OnChangeTamperEnableCheck)
		ON_EN_CHANGE(IDC_TAMPER_TIME_EDIT, &CTamperConfigPage::OnEnChangeTamperTimeEdit)
		ON_EN_CHANGE(IDC_TAMPER_AREA_EDIT, &CTamperConfigPage::OnEnChangeTamperAreaEdit)
		ON_BN_CLICKED(IDC_CHECK_SUPPRESS_LIGHT, &CTamperConfigPage::OnBnClickedCheckSuppressLight)
		ON_WM_SIZE()
		ON_WM_DESTROY()
END_MESSAGE_MAP()


void CTamperConfigPage::SyncToDataMgr()
{
	if( m_bPopulating )
	{
		return;
	}

	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_TAMPER_INFO TamperInfo;

	TamperInfo.ulEnabled		= (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TAMPER))?TRUE:FALSE;
	TamperInfo.ulAlarmTimeout	= GetDlgItemInt(IDC_TAMPER_TIME_EDIT);
	TamperInfo.ulAreaThreshold	= GetDlgItemInt(IDC_TAMPER_AREA_EDIT);
	TamperInfo.ulFiringThreshold= 0;
	TamperInfo.ulLowLightEnabled= (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SUPPRESS_LIGHT))?TRUE:FALSE;

	pDataMgr->SetTamperInfo(&TamperInfo);

	pDataMgr->FireEvent( IVCAConfigureObserver::VCA_TAMPER_UPDATE, NULL );
}

// CTamperConfigPage message handlers
void CTamperConfigPage::OnChangeTamperEnableCheck()
{
	BOOL bEnable = ((BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TAMPER)) && (GetDlgItem(IDC_CHECK_TAMPER)->IsWindowEnabled()));
	GetDlgItem(IDC_TAMPER_TIME_EDIT)->EnableWindow(bEnable);
	GetDlgItem(IDC_TAMPER_AREA_EDIT)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_SUPPRESS_LIGHT)->EnableWindow(bEnable);

	SyncToDataMgr();

}

void CTamperConfigPage::OnEnChangeTamperTimeEdit()
{
	SyncToDataMgr();
}

void CTamperConfigPage::OnEnChangeTamperAreaEdit()
{
	SyncToDataMgr();
}

void CTamperConfigPage::OnBnClickedCheckSuppressLight()
{
	SyncToDataMgr();
}

void CTamperConfigPage::OnDestroy()
{
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_PREVIEW );
	m_pVCADialog->SetParent( m_pOldParent );

	CDialog::OnDestroy();

	m_pVCADialog->ShowWindow( SW_HIDE );
	m_pDataMgr->SetDisplayFlags(m_OldDisplay);

	m_bAlive = FALSE;
	// TODO: Add your message handler code here
}
