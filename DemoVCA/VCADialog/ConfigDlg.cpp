// VCADialog\ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigDlg.h"
#include "wm_user.h"
#include "VCADialog.h"
#include "../VCAEngine/VCAEngine.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CConfigDlg dialog

IMPLEMENT_DYNAMIC(CConfigDlg, CDialog)

CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{
	m_bAlive = FALSE;
	m_uiActualTabMask	= CONFIG_ALL;
	m_uiAllowedTabMask	= CONFIG_ALL;
	m_uiDirtyMask = 0;

	m_pVCADialog	= NULL;
}

CConfigDlg::~CConfigDlg()
{
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIG_TAB, m_TabCtrl);
}

BOOL CConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bAlive = TRUE;

	UpdateTabs();
	return FALSE;
}

BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CONFIG_APPLY, &CConfigDlg::OnBnClickedConfigApply)
	ON_BN_CLICKED(IDC_CONFIG_EXIT, &CConfigDlg::OnBnClickedConfigExit)
	ON_MESSAGE( WM_CONFIG_DIRTY, &CConfigDlg::OnConfigDirty )
END_MESSAGE_MAP()


// CConfigDlg message handlers

void CConfigDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	m_TabCtrl.Activate( bShow );



	if( bShow )
	{
		m_uiDirtyMask = 0;

		// Assume no changes have been made
		GetDlgItem( IDC_CONFIG_APPLY )->EnableWindow( FALSE );

		RedrawWindow( );
	}
}

void CConfigDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if( m_bAlive )
	{
		m_TabCtrl.MoveWindow( 10, 10, cx , cy - 50 );

		CRect rc, rc2;
		GetDlgItem( IDC_CONFIG_EXIT )->GetClientRect( &rc );
		GetDlgItem( IDC_CONFIG_EXIT )->MoveWindow( cx - rc.Width() - 5, cy - rc.Height() - 5, rc.Width(), rc.Height() );

		GetDlgItem( IDC_CONFIG_EXIT )->GetWindowRect( &rc );
		ScreenToClient( &rc );
		GetDlgItem( IDC_CONFIG_APPLY )->GetClientRect( &rc2 );
		GetDlgItem( IDC_CONFIG_APPLY )->MoveWindow( rc.left - 5 - rc2.Width(), cy - rc2.Height() - 5, rc2.Width(), rc2.Height() );
	}

//	Invalidate();
}

void CConfigDlg::OnBnClickedConfigApply()
{
	// TODO: Add your control notification handler code here

	// Apply the changes
//	m_TabCtrl.Apply();

	BOOL bSaveConf = FALSE;
	if( m_uiDirtyMask & (IVCAConfigureObserver::VCA_ZONE_ANY | IVCAConfigureObserver::VCA_RULE_ANY | IVCAConfigureObserver::VCA_COUNTER_ANY ) )
	{
		m_pVCADialog->ApplyZoneSetting();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_CALIB_UPDATE )
	{
		m_pVCADialog->ApplyCalibInfoSetting();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_OBJCLS_UPDATE )
	{
		m_pVCADialog->ApplyClassObjsSetting();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_ADVANCED_UPDATE )
	{
		m_pVCADialog->ApplyAdvancedSetting();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_TAMPER_UPDATE )
	{
		m_pVCADialog->ApplyTamperInfo();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_SCENECHANGE_UPDATE )
	{
		m_pVCADialog->ApplySceneChangeInfo();
		bSaveConf = TRUE;
	}
	if( m_uiDirtyMask & IVCAConfigureObserver::VCA_ENGINE_UPDATE )
	{
		m_pVCADialog->ApplyEngineSetting();
		// Also send a message to my parent to tell them to reconfigure the video source for this engine
		GetParent()->SendMessage( WM_VIDEO_SRC_CHANGED, (WPARAM) m_pVCADialog->GetEngId() );

		//UpdateData will save DemoVCA.XML (license include in DemoXML)
		m_LicensePage.UpdateData(TRUE);
	}

	m_uiDirtyMask = 0;

	GetDlgItem( IDC_CONFIG_APPLY )->EnableWindow( FALSE );

	UpdateTabs();

	if( GetParent() )
	{
		GetParent()->PostMessage( WM_CONFIG_UPDATED, (WPARAM) m_pVCADialog->GetEngId() );
	}

	//Save current Setting
	if(bSaveConf){
		int iEngId = m_pVCADialog->GetEngId();
		CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
		VCA5_APP_ENGINE_INFO *pAppInfo = pAppCfg->GetAPPEngineInfo( iEngId );
		CVCAConfigure::Instance()->SaveEngine(iEngId, pAppInfo->szConfPath);
	}
}

void CConfigDlg::OnBnClickedConfigExit()
{
	// TODO: Add your control notification handler code here
	if( GetParent() )
	{
		GetParent()->SendMessage( WM_CONFIG_MODE, (WPARAM) m_pVCADialog->GetEngId() , 0 );
	}
}

LRESULT CConfigDlg::OnConfigDirty( WPARAM wParam, LPARAM lParam )
{
	GetDlgItem( IDC_CONFIG_APPLY )->EnableWindow( TRUE );

	return 0;
}

void CConfigDlg::FireOnEvent( DWORD uiEvent, DWORD uiContext )
{
	unsigned int uiMask = IVCAConfigureObserver::VCA_ZONE_ANY |
					IVCAConfigureObserver::VCA_RULE_ANY |
					IVCAConfigureObserver::VCA_COUNTER_ANY |
					IVCAConfigureObserver::VCA_OBJCLS_UPDATE |
					IVCAConfigureObserver::VCA_CALIB_UPDATE |
					IVCAConfigureObserver::VCA_ADVANCED_UPDATE |
					IVCAConfigureObserver::VCA_TAMPER_UPDATE |
					IVCAConfigureObserver::VCA_SCENECHANGE_UPDATE |
					IVCAConfigureObserver::VCA_ENGINE_UPDATE;

	if( uiEvent & uiMask )
	{
		// Something changed
		GetDlgItem( IDC_CONFIG_APPLY )->EnableWindow( TRUE );
		m_uiDirtyMask |= uiEvent;
	}
}

void CConfigDlg::SetTabMask(unsigned int uiTabMask)
{
	m_uiAllowedTabMask = uiTabMask;
	UpdateTabs();
}

void CConfigDlg::SetVCADialog( CVCADialog *pVCADialog )
{
	// Unregister updates from old one
	if( m_pVCADialog )
	{
		m_pVCADialog->GetVCADataMgr()->UnregisterObserver( this );
	}
	m_pVCADialog = pVCADialog;

	// Register new one
	if( m_pVCADialog )
	{
		m_pVCADialog->GetVCADataMgr()->RegisterObserver( this );
	}

	// Apply to all children
	m_AdvancedPage.SetVCADialog( pVCADialog );
	m_ClassificationPage.SetVCADialog( pVCADialog );
	m_ZonesRulesPage.SetVCADialog( pVCADialog );
	m_CalibPage.SetVCADialog( pVCADialog );
	m_TamperPage.SetVCADialog( pVCADialog );
	m_SceneChangePage.SetVCADialog( pVCADialog );
	m_VideoPage.SetVCADialog( pVCADialog );
	m_LicensePage.SetVCADialog( pVCADialog );
	m_LicensePage.UpdateData();

	UpdateTabs();
}

void CConfigDlg::UpdateTabs()
{
	unsigned int uiTabMask;
	unsigned int uiNewMask = 0;


	// Remove unsupported tabs based on engine support
	uiNewMask = CONFIG_VIDEOSRC;

	if( m_pVCADialog )
	{
		CEngine *pEngine = m_pVCADialog->GetEngine();

		if( pEngine )
		{
			unsigned int uiFunction = m_pVCADialog->GetEngine()->GetFunction();

			uiNewMask |= CONFIG_ZONESRULES | CONFIG_TAMPER | CONFIG_SCENECHANGE | CONFIG_LICENSE | CONFIG_ADVANCED;
			
			if( uiFunction & VCA5_FEATURE_CALIBRATION )
			{
				uiNewMask |= CONFIG_CALIBRATION | CONFIG_CLASSIFICATION;
			}
		}
	}


	// Get the intersection of both
	uiTabMask = m_uiAllowedTabMask & uiNewMask;

	if( uiTabMask == m_uiActualTabMask )
	{
		// No change, return
		return;
	}

	m_TabCtrl.DeleteAllItems();
	m_uiActualTabMask = uiTabMask;

	// Now add all required tabs
	if( uiTabMask & CONFIG_ZONESRULES )
	{
		m_TabCtrl.AddTab( _T("Zones && Rules"), CZonesRulesConfigPage::IDD, &m_ZonesRulesPage );
	}

	if( uiTabMask & CONFIG_CALIBRATION )
	{
		m_TabCtrl.AddTab( _T("Calibration"), CCalibrationConfigPage::IDD, &m_CalibPage );
	}

	if( uiTabMask & CONFIG_ADVANCED )
	{
		m_TabCtrl.AddTab( _T("Advanced"), CAdvancedSettingsPage::IDD, &m_AdvancedPage );
	}

	if( uiTabMask & CONFIG_CLASSIFICATION )
	{
		m_TabCtrl.AddTab( _T("Classification"), CClassificationSettingPage::IDD, &m_ClassificationPage );
	}

	if( uiTabMask & CONFIG_TAMPER )
	{
		m_TabCtrl.AddTab( _T("Tamper Detection"), CTamperConfigPage::IDD, &m_TamperPage );
	}

	if( uiTabMask & CONFIG_SCENECHANGE )
	{
		m_TabCtrl.AddTab( _T("Scene Change Detection"), CSceneChangeConfigPage::IDD, &m_SceneChangePage );
	}

	if( uiTabMask & CONFIG_VIDEOSRC )
	{
		m_TabCtrl.AddTab( _T("Video Source"), CVideoConfigPage::IDD, &m_VideoPage );
	}

	if( uiTabMask & CONFIG_LICENSE )
	{
		m_TabCtrl.AddTab( _T("License Configuration"), CLicenseConfigPage::IDD, &m_LicensePage );
	}

	if( m_pVCADialog )
	{
		// Deactivate and reactivate
		m_TabCtrl.Activate( FALSE );
		m_TabCtrl.Activate( TRUE );
	}

}

