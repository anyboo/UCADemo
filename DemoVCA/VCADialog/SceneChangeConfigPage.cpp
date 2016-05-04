// ./VCADialog/TamperConfigPage.cpp : implementation file
//

#include "stdafx.h"
#include "SceneChangeConfigPage.h"
#include "VCADialog.h"
#include "../Render/VCARender.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CSceneChangeConfigPage dialog

IMPLEMENT_DYNAMIC(CSceneChangeConfigPage, CConfigPage)

BEGIN_MESSAGE_MAP(CSceneChangeConfigPage, CConfigPage)
		ON_BN_CLICKED(IDC_RADIO_DISABLE, &CSceneChangeConfigPage::OnEnChangeMode)
		ON_BN_CLICKED(IDC_RADIO_AUTOMATIC, &CSceneChangeConfigPage::OnEnChangeMode)
		ON_BN_CLICKED(IDC_RADIO_MANUAL, &CSceneChangeConfigPage::OnEnChangeMode)
		ON_EN_CHANGE(IDC_TAMPER_TIME_EDIT, &CSceneChangeConfigPage::OnEnChangeTamperTimeEdit)
		ON_EN_CHANGE(IDC_TAMPER_AREA_EDIT, &CSceneChangeConfigPage::OnEnChangeTamperAreaEdit)
		ON_BN_CLICKED(IDC_CHECK_DN_COLOR, &CSceneChangeConfigPage::OnBnClickedCheckDnColor)
		ON_BN_CLICKED(IDC_CHECK_DN_RIM, &CSceneChangeConfigPage::OnBnClickedCheckDnRim)
		ON_WM_SIZE()
		ON_WM_DESTROY()
END_MESSAGE_MAP()


CSceneChangeConfigPage::CSceneChangeConfigPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CSceneChangeConfigPage::IDD, pParent)
{
	m_bPopulating	= FALSE;
	m_bAlive		= FALSE;
}

CSceneChangeConfigPage::~CSceneChangeConfigPage()
{
}

void CSceneChangeConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}

BOOL CSceneChangeConfigPage::OnInitDialog()
{
	m_bPopulating = TRUE;
	CConfigPage::OnInitDialog();

	m_pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_SCENECHANGE_INFO* pSceneChangeInfo = m_pDataMgr->GetSceneChangeInfo();

	CheckRadioButton(IDC_RADIO_DISABLE, IDC_RADIO_MANUAL, pSceneChangeInfo->ulMode+IDC_RADIO_DISABLE); 

	SetDlgItemInt(IDC_TAMPER_TIME_EDIT, pSceneChangeInfo->tTamper.ulAlarmTimeout);
	SetDlgItemInt(IDC_TAMPER_AREA_EDIT, pSceneChangeInfo->tTamper.ulAreaThreshold);
	CheckDlgButton(IDC_CHECK_DN_COLOR, pSceneChangeInfo->ulDNColorEnabled ?BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_DN_RIM, pSceneChangeInfo->ulDNRimEnabled ?BST_CHECKED : BST_UNCHECKED);

	VCA5_APP_ADVANCED_INFO* pAdvancedInfo =  m_pDataMgr->GetAdvancedInfo();
	BOOL bEnable = (pAdvancedInfo->TrackerParams.bMovObjEnabled || 
							pAdvancedInfo->TrackerParams.bAbObjEnabled || 
							pAdvancedInfo->TrackerParams.bAdvTraEnabled);

	GetDlgItem(IDC_RADIO_DISABLE)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_AUTOMATIC)->EnableWindow(bEnable);

	m_OldDisplay	= m_pDataMgr->GetDisplayFlags();
	m_pDataMgr->SetDisplayFlags(IVCARender::DISPLAY_SCENECANGE);

	m_pOldParent	= m_pVCADialog->SetParent( this );
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_TAMPER );
	m_pVCADialog->ShowWindow( SW_SHOW );

	SetDlgItemText(IDC_EDIT_SCENE_WARNING,
		_T("NOTE: VCA object tracking or abandoned/removed \r\nobject detection must be enabled for scene change\r\ndetection to work"));

	
	OnEnChangeMode();

	m_bPopulating	= FALSE;
	m_bAlive		= TRUE;

	return TRUE;
}


void CSceneChangeConfigPage::OnSize(UINT nType, int cx, int cy)
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


void CSceneChangeConfigPage::SyncToDataMgr()
{
	if( m_bPopulating )
	{
		return;
	}

	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_SCENECHANGE_INFO SceneChangeInfo;


	if(IsDlgButtonChecked(IDC_RADIO_DISABLE)) SceneChangeInfo.ulMode = VCA5_SCENECHANGE_MODE_DISABLED;
	else if(IsDlgButtonChecked(IDC_RADIO_AUTOMATIC)) SceneChangeInfo.ulMode = VCA5_SCENECHANGE_MODE_AUTOMATIC;
	else SceneChangeInfo.ulMode = VCA5_SCENECHANGE_MODE_MANUAL;
	
	SceneChangeInfo.tTamper.ulEnabled	= (SceneChangeInfo.ulMode == VCA5_SCENECHANGE_MODE_MANUAL);
	SceneChangeInfo.tTamper.ulAlarmTimeout		= GetDlgItemInt(IDC_TAMPER_TIME_EDIT);
	SceneChangeInfo.tTamper.ulAreaThreshold		= GetDlgItemInt(IDC_TAMPER_AREA_EDIT);
	SceneChangeInfo.tTamper.ulFiringThreshold	= 0;
	SceneChangeInfo.tTamper.ulLowLightEnabled	= FALSE;
	SceneChangeInfo.ulDNColorEnabled	= (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_DN_COLOR))?TRUE:FALSE;
	SceneChangeInfo.ulDNRimEnabled	= (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_DN_RIM))?TRUE:FALSE;

	pDataMgr->SetSceneChangeInfo(&SceneChangeInfo);
	pDataMgr->FireEvent( IVCAConfigureObserver::VCA_SCENECHANGE_UPDATE, NULL );
}

// CSceneChangeConfigPage message handlers
void CSceneChangeConfigPage::OnEnChangeMode()
{
	BOOL bEnable = ((BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_MANUAL)) && (GetDlgItem(IDC_RADIO_MANUAL)->IsWindowEnabled()));
	GetDlgItem(IDC_TAMPER_TIME_EDIT)->EnableWindow(bEnable);
	GetDlgItem(IDC_TAMPER_AREA_EDIT)->EnableWindow(bEnable);
	
	SyncToDataMgr();
}


void CSceneChangeConfigPage::OnEnChangeTamperTimeEdit()
{
	SyncToDataMgr();
}

void CSceneChangeConfigPage::OnEnChangeTamperAreaEdit()
{
	SyncToDataMgr();
}

void CSceneChangeConfigPage::OnBnClickedCheckDnColor()
{
	SyncToDataMgr();
}

void CSceneChangeConfigPage::OnBnClickedCheckDnRim()
{
	SyncToDataMgr();
}

void CSceneChangeConfigPage::OnDestroy()
{
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_PREVIEW );
	m_pVCADialog->SetParent( m_pOldParent );

	CDialog::OnDestroy();

	m_pVCADialog->ShowWindow( SW_HIDE );
	m_pDataMgr->SetDisplayFlags(m_OldDisplay);

	m_bAlive = FALSE;
	// TODO: Add your message handler code here
}
