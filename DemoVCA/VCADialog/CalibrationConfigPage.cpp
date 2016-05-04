// ./VCADialog/CalibrationConfigPage.cpp : implementation file
//

#include "stdafx.h"
#include "CalibrationConfigPage.h"
#include "VCADialog.h"
#include "../Render/VCARender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// CCalibrationConfigPage dialog

IMPLEMENT_DYNAMIC(CCalibrationConfigPage, CConfigPage)

CCalibrationConfigPage::CCalibrationConfigPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CCalibrationConfigPage::IDD, pParent)
{
	m_bAlive = FALSE;
}

CCalibrationConfigPage::~CCalibrationConfigPage()
{
}

void CCalibrationConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCalibrationConfigPage, CConfigPage)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BN_CALIB_PAUSE, &CCalibrationConfigPage::OnBnClickedBnCalibPause)
	ON_BN_CLICKED(IDC_BUTTON_UNCALIBRATE, &CCalibrationConfigPage::OnBnClickedButtonUncalibrate)
END_MESSAGE_MAP()


BOOL CCalibrationConfigPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();
	m_CalibCtl.SetDataMgr( pDataMgr );
	m_OldDisplay	= pDataMgr->GetDisplayFlags();

	m_CalibCtl.Create( CCalibrationCtl::IDD, this );
	m_CalibCtl.ShowWindow( SW_SHOW );

	m_pOldParent = m_pVCADialog->SetParent( this );

	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_CALIBRATION );
	m_pVCADialog->ShowWindow( SW_SHOW );

	m_bAlive = TRUE;

	//
	
	return TRUE;
}

// CCalibrationConfigPage message handlers

void CCalibrationConfigPage::OnSize( UINT nType, int cx, int cy )
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
		CRect rc( 5, 5, (rcClient.Width() * 3)/4, rcClient.bottom - 5);
		m_pVCADialog->MoveWindow( rc );
	}

	if( m_CalibCtl.GetSafeHwnd() )
	{
		CRect rc( (rcClient.Width() * 3)/4, 5, rcClient.right - 5, rcClient.bottom - 30 );
		m_CalibCtl.MoveWindow( rc );
	}

	int ButtonWidth = (rcClient.right - (3*rcClient.Width())/4)/2 - 10 ;
	//---------------------------------------------------
	CButton *pBtn = (CButton *)GetDlgItem( IDC_BUTTON_UNCALIBRATE );
	if( pBtn )
	{
		CRect rc( (rcClient.Width() * 3)/4 + 5, rcClient.bottom - 25, rcClient.right - (ButtonWidth + 10)  , rcClient.bottom - 2 );
		pBtn->MoveWindow( rc );
	}

	pBtn = (CButton *)GetDlgItem( IDC_BN_CALIB_PAUSE );
	if( pBtn )
	{
		CRect rc( rcClient.right - (ButtonWidth + 10) +5, rcClient.bottom - 25, rcClient.right - 5, rcClient.bottom - 2 );
		pBtn->MoveWindow( rc );
	}
}


void CCalibrationConfigPage::OnDestroy( )
{
	// Put VCA Dialog parent back to what it was
	m_pVCADialog->ChangeViewMode( CVCADialog::VIEW_MODE_PREVIEW );
	m_pVCADialog->SetParent( m_pOldParent );
	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	CDialog::OnDestroy();

	m_pVCADialog->ShowWindow( SW_HIDE );

	m_CalibCtl.DestroyWindow();
	pDataMgr ->SetDisplayFlags(m_OldDisplay);

	//
	m_bAlive = FALSE;
}


void CCalibrationConfigPage::OnBnClickedBnCalibPause()
{
	// TODO: Add your control notification handler code here
	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	DWORD dwFlags = pDataMgr->GetDisplayFlags();
	if( dwFlags & IVCARender::CALIB_PAUSE )
	{
		dwFlags &= ~IVCARender::CALIB_PAUSE;
	}
	else
	{
		dwFlags |= IVCARender::CALIB_PAUSE;
	}

	pDataMgr->SetDisplayFlags( dwFlags );
}


void CCalibrationConfigPage::OnBnClickedButtonUncalibrate()
{
	if(IDYES != MessageBox(_T("This will remove all the calibration settings, All current settings will be lost. Are you sure you want to continue?"),  
		_T("WARNING"), MB_YESNO)){
		return ;
	}

	CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();

	VCA5_CALIB_INFO*	pCalibInfo	= pDataMgr->GetCalibInfo();
	pCalibInfo->calibrationStatus	= VCA5_CALIB_STATUS_UNCALIBRATED;
	m_pVCADialog->ApplyCalibInfoSetting();
}
