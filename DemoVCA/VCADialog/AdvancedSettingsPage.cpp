// AdvancedSettingsPage.cpp : implementation file
//

#include "stdafx.h"
#include "AdvancedSettingsPage.h"
#include "../resource.h"
#include "VCADialog.h"
#include "../Common/VCADataMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// CAdvancedSettingsPage dialog

IMPLEMENT_DYNAMIC(CAdvancedSettingsPage, CConfigPage)

CAdvancedSettingsPage::CAdvancedSettingsPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CAdvancedSettingsPage::IDD, pParent)
{
	m_pVCADialog = NULL;
	m_bPopulating = FALSE;
}

CAdvancedSettingsPage::~CAdvancedSettingsPage()
{
}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAdvancedSettingsPage, CConfigPage)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_OBTRACKING_ENABLE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_CHECK_ABRM_ENABLE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_CHECK_CNTLINE_ENABLE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_CHECK_CAMSTAB_ENABLE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_EN_CHANGE(IDC_RETRIGGER_TIME_EDIT, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_EN_CHANGE(IDC_EDIT_OBJSIZE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_EN_CHANGE(IDC_EDIT_OBJHOLDTIME, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_CHECK_SMOKEFIRE_ENABLE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_RADIO_DETECTPOINT1, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_RADIO_DETECTPOINT2, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_RADIO_DETECTPOINT3, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_RADIO_SURVEILLANCE, &CAdvancedSettingsPage::OnEnChangeValue)
	ON_BN_CLICKED(IDC_RADIO_PEOPLE, &CAdvancedSettingsPage::OnEnChangeValue)
END_MESSAGE_MAP()


// CAdvancedSettingsPage message handlers
BOOL CAdvancedSettingsPage::OnInitDialog()
{
	m_bPopulating = TRUE;
	// Forgot to set me??
	ASSERT( m_pVCADialog );
	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();
	VCA5_APP_ADVANCED_INFO *pInfo = pDataMgr->GetAdvancedInfo();

	// Set data
	BOOL bObjTracking = pInfo->TrackerParams.bMovObjEnabled;
	BOOL bIsPeopleTrackMode = pInfo->TrackerParams.bAdvTraEnabled;
	BOOL bIsPeopleTrackMenuEnabled = bObjTracking && pDataMgr->CheckFeature(VCA5_FEATURE_PEOPLETRACKING);
	CheckDlgButton(IDC_CHECK_OBTRACKING_ENABLE, bObjTracking ? BST_CHECKED : BST_UNCHECKED);

	GetDlgItem(IDC_RADIO_SURVEILLANCE)->EnableWindow(bIsPeopleTrackMenuEnabled);
	GetDlgItem(IDC_RADIO_PEOPLE)->EnableWindow(bIsPeopleTrackMenuEnabled);

	if(!pDataMgr->CheckFeature(VCA5_FEATURE_PRESENCE)){
		GetDlgItem(IDC_CHECK_OBTRACKING_ENABLE)->EnableWindow(FALSE);
		CheckDlgButton(IDC_CHECK_OBTRACKING_ENABLE, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_SURVEILLANCE, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_PEOPLE, BST_UNCHECKED);
	}

	if ( bIsPeopleTrackMenuEnabled )
		CheckRadioButton(IDC_RADIO_SURVEILLANCE, IDC_RADIO_PEOPLE, pInfo->TrackerParams.bAdvTraEnabled + IDC_RADIO_SURVEILLANCE);
	else
		CheckRadioButton(IDC_RADIO_SURVEILLANCE, IDC_RADIO_PEOPLE, IDC_RADIO_SURVEILLANCE);

	CheckDlgButton(IDC_CHECK_ABRM_ENABLE, pInfo->TrackerParams.bAbObjEnabled ? BST_CHECKED : BST_UNCHECKED);
	if(!pDataMgr->CheckFeature(VCA5_FEATURE_ABOBJ)){
		GetDlgItem(IDC_CHECK_ABRM_ENABLE)->EnableWindow(FALSE);
		CheckDlgButton(IDC_CHECK_ABRM_ENABLE, BST_UNCHECKED);
	}

	CheckDlgButton(IDC_CHECK_CNTLINE_ENABLE, pInfo->TrackerParams.bCntLineEnabled ? BST_CHECKED : BST_UNCHECKED);
	if(!pDataMgr->CheckFeature(VCA5_FEATURE_LINECOUNTER)){
		GetDlgItem(IDC_CHECK_CNTLINE_ENABLE)->EnableWindow(FALSE);
		CheckDlgButton(IDC_CHECK_CNTLINE_ENABLE, BST_UNCHECKED);
	}

	CheckDlgButton(IDC_CHECK_SMOKEFIRE_ENABLE, pInfo->TrackerParams.bSmokeFireEnabled ? BST_CHECKED : BST_UNCHECKED);
	if(!pDataMgr->CheckFeature(VCA5_FEATURE_SMOKE|VCA5_FEATURE_FIRE)){
		GetDlgItem(IDC_CHECK_SMOKEFIRE_ENABLE)->EnableWindow(FALSE);
		CheckDlgButton(IDC_CHECK_SMOKEFIRE_ENABLE, BST_UNCHECKED);
	}

	CheckDlgButton(IDC_CHECK_CAMSTAB_ENABLE, pInfo->bEnableStab ? BST_CHECKED : BST_UNCHECKED);

	
	SetDlgItemInt(IDC_RETRIGGER_TIME_EDIT, pInfo->ulRetrigTime);

	SetDlgItemInt(IDC_EDIT_OBJSIZE, pInfo->TrackerParams.ulMinObjAreaPix);
	SetDlgItemInt(IDC_EDIT_OBJHOLDTIME, pInfo->TrackerParams.ulSecsToHoldOn);

	CheckRadioButton(IDC_RADIO_DETECTPOINT1, IDC_RADIO_DETECTPOINT3, pInfo->TrackerParams.ulDetectionPoint+IDC_RADIO_DETECTPOINT1); 
	
	m_bPopulating = FALSE;
	return FALSE;
}


void CAdvancedSettingsPage::SyncToDataMgr()
{
	if( m_bPopulating )
	{
		return;
	}
	// TODO: Add your control notification handler code here
	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_APP_ADVANCED_INFO *pInfo = pDataMgr->GetAdvancedInfo();

	pInfo->bEnableStab		= IsDlgButtonChecked(IDC_CHECK_CAMSTAB_ENABLE) == BST_CHECKED ? TRUE : FALSE;
	pInfo->ulRetrigTime		= GetDlgItemInt(IDC_RETRIGGER_TIME_EDIT);
	pInfo->TrackerParams.ulMinObjAreaPix	= GetDlgItemInt(IDC_EDIT_OBJSIZE); 
	pInfo->TrackerParams.ulSecsToHoldOn		= GetDlgItemInt(IDC_EDIT_OBJHOLDTIME);
	pInfo->TrackerParams.bAbObjEnabled		= IsDlgButtonChecked(IDC_CHECK_ABRM_ENABLE) == BST_CHECKED ? TRUE : FALSE;
	pInfo->TrackerParams.bMovObjEnabled		= IsDlgButtonChecked(IDC_CHECK_OBTRACKING_ENABLE) == BST_CHECKED ? TRUE : FALSE;
	pInfo->TrackerParams.bAdvTraEnabled		= IsDlgButtonChecked(IDC_RADIO_PEOPLE) == BST_CHECKED ? TRUE : FALSE;
	pInfo->TrackerParams.bCntLineEnabled	= IsDlgButtonChecked(IDC_CHECK_CNTLINE_ENABLE) == BST_CHECKED ? TRUE : FALSE;
	pInfo->TrackerParams.bSmokeFireEnabled	= IsDlgButtonChecked(IDC_CHECK_SMOKEFIRE_ENABLE) == BST_CHECKED ? TRUE : FALSE;;

	BOOL bObjTracking = pInfo->TrackerParams.bMovObjEnabled;
	BOOL bIsPeopleTrackMenuEnabled = bObjTracking && pDataMgr->CheckFeature(VCA5_FEATURE_PEOPLETRACKING);

	GetDlgItem(IDC_RADIO_SURVEILLANCE)->EnableWindow(bIsPeopleTrackMenuEnabled);
	GetDlgItem(IDC_RADIO_PEOPLE)->EnableWindow(bIsPeopleTrackMenuEnabled);

	if ( bIsPeopleTrackMenuEnabled )
		CheckRadioButton(IDC_RADIO_SURVEILLANCE, IDC_RADIO_PEOPLE, pInfo->TrackerParams.bAdvTraEnabled + IDC_RADIO_SURVEILLANCE);
	else
		CheckRadioButton(IDC_RADIO_SURVEILLANCE, IDC_RADIO_PEOPLE, IDC_RADIO_SURVEILLANCE);

	if(IsDlgButtonChecked(IDC_RADIO_DETECTPOINT1))pInfo->TrackerParams.ulDetectionPoint	= 0;
	else if(IsDlgButtonChecked(IDC_RADIO_DETECTPOINT2))pInfo->TrackerParams.ulDetectionPoint = 1;
	else if(IsDlgButtonChecked(IDC_RADIO_DETECTPOINT3))pInfo->TrackerParams.ulDetectionPoint = 2;

	pDataMgr->FireEvent( IVCAConfigureObserver::VCA_ADVANCED_UPDATE, NULL );
}

void CAdvancedSettingsPage::OnEnChangeValue()
{
	SyncToDataMgr();
}

