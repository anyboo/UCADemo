// ./VCADialog/CalibrationCtl.cpp : implementation file
//

#include "stdafx.h"
#include "CalibrationCtl.h"
#include <VCA5CoreLib.h>
#include "../Common/VCADataMgr.h"


// CCalibrationCtl dialog

IMPLEMENT_DYNAMIC(CCalibrationCtl, CDialog)

CCalibrationCtl::CCalibrationCtl(CWnd* pParent /*=NULL*/)
	: CDialog(CCalibrationCtl::IDD, pParent)
{
	m_bAlive = FALSE;
	m_pDataMgr = NULL;
	m_bApplyData = TRUE;
}

CCalibrationCtl::~CCalibrationCtl()
{
}

void CCalibrationCtl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CCalibrationCtl::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT( m_pDataMgr );

	m_pDataMgr->RegisterObserver( this );

	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_CALIB_UNITS_COMBO);
	pCombo->ResetContent();
	pCombo->InsertString( 0, _T("Metric (m)") );
	pCombo->InsertString( 1, _T("Imperial (ft/in)") );

	((CSliderCtrl *)GetDlgItem( IDC_FOV_SLIDER ))->SetRange( 5, 150 );
	((CSliderCtrl *)GetDlgItem( IDC_PAN_SLIDER ))->SetRange( -90, 90 );
	((CSliderCtrl *)GetDlgItem( IDC_ROLL_SLIDER ))->SetRange( -90, 90 );

	UpdateValues( );

	m_bAlive = TRUE;
	return TRUE;
}


BEGIN_MESSAGE_MAP(CCalibrationCtl, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_ED_HEIGHT, &CCalibrationCtl::OnEnChangeEdHeight)
	ON_EN_CHANGE(IDC_ED_HEIGHT2, &CCalibrationCtl::OnEnChangeEdHeight2)
	ON_EN_CHANGE(IDC_ED_TILT, &CCalibrationCtl::OnEnChangeEdTilt)
	ON_EN_CHANGE(IDC_ED_FOV, &CCalibrationCtl::OnEnChangeEdFov)
	ON_EN_CHANGE(IDC_ED_PAN, &CCalibrationCtl::OnEnChangeEdPan)
	ON_EN_CHANGE(IDC_ED_ROLL, &CCalibrationCtl::OnEnChangeEdRoll)
	ON_CBN_SELCHANGE(IDC_CALIB_UNITS_COMBO, &CCalibrationCtl::OnCbnSelchangeCalibUnitsCombo)
	ON_BN_CLICKED(IDC_BUTTON_RESTORE, &CCalibrationCtl::OnBnClickedButtonRestore)
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CCalibrationCtl message handlers

HBRUSH CCalibrationCtl::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
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

void CCalibrationCtl::OnDestroy()
{
	CDialog::OnDestroy();

	m_pDataMgr->UnregisterObserver( this );

	m_bAlive = FALSE;
}

void CCalibrationCtl::OnSize( UINT nType, int cx, int cy )
{
	if( !m_bAlive )
	{
		return;
	}

	CRect rc, rc2, rcClient;
	GetClientRect(&rcClient);

	int width = min( rcClient.Width(), 400 );

	// Explaining Picture about Calibration setup
	GetDlgItem(IDC_CAMERA_SETUP_INFO)->MoveWindow((rcClient.Width() - width)/2, 0, width, (width *4)/5);

	GetDlgItem( IDC_CAMERA_SETUP_INFO )->GetWindowRect( &rc );
	ScreenToClient( &rc );

	//-----------------------------------------------------
	// Camera height and tilt angle
	rc2.top = rc.bottom + 5;
	rc2.left = rcClient.left + 5;
	rc2.right = rcClient.right - 5;
	rc2.bottom = rc2.top + 70;

	GetDlgItem( IDC_CAM_SETUP )->MoveWindow( rc2 );

	rc = rc2;
	rc2.top += 18;
	rc2.left += 20;
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_HEIGHT )->MoveWindow( rc2 );

	rc2.OffsetRect( 50, 0 );
	rc2.bottom = rc2.top + 18;

	VCA5_APP_CALIB_INFO *pInfo = m_pDataMgr->GetCalibInfo();
	if( pInfo->ulSpeedUnits == VCA5_SPEED_UNITS_MPH )
	{
		rc2.right = rc2.left + 45;
		GetDlgItem( IDC_ED_HEIGHT )->MoveWindow( rc2 );

		rc2.OffsetRect( 50, 0);
		rc2.right -= 15;
		GetDlgItem( IDC_ED_HEIGHT2 )->MoveWindow( rc2 );
		rc2.OffsetRect( 35, 0 );
	}
	else
	{
		rc2.right = rc2.left + 50;
		GetDlgItem( IDC_ED_HEIGHT )->MoveWindow( rc2 );
		rc2.OffsetRect( 60, 0 );
	}
	rc2.bottom = rc2.top + 18;
	rc2.right = rc2.left + 40;

	GetDlgItem( IDC_METER )->MoveWindow( rc2 );

	GetDlgItem( IDC_HEIGHT )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, 25 );
	GetDlgItem( IDC_TILTANGLE )->MoveWindow( rc2 );
	
	rc2.OffsetRect( 50, 0 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;
	GetDlgItem( IDC_ED_TILT )->MoveWindow( rc2 );

	rc2.OffsetRect( 60, 0 );
	rc2.bottom = rc2.top + 18;
	GetDlgItem( IDC_DEG )->MoveWindow( rc2 );

	//----------------------------------------------------
	// Intrinsic params
	GetDlgItem( IDC_CAM_SETUP )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, rc2.Height() + 3 );
	GetDlgItem( IDC_INTRICSIC_PARAM )->MoveWindow( rc2 );

	rc2.OffsetRect( 20, 18 );
	rc2.right = rc2.left + 70;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_FOV )->MoveWindow( rc2 );

	rc2.OffsetRect( 70, 0 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_ED_FOV )->MoveWindow( rc2 );

	rc2.OffsetRect( 60, 0 );
	rc2.bottom = rc2.top + 18;
	rc2.right = rc2.left + 30;
	GetDlgItem( IDC_DEG1 )->MoveWindow( rc2 );

	GetDlgItem( IDC_FOV )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, 25 );
	rc2.right = rc2.left + 140;
	rc2.bottom = rc2.top + 20;

	GetDlgItem( IDC_FOV_SLIDER )->MoveWindow( rc2 );

	//--------------------------------------------------
	// Status
	GetDlgItem( IDC_INTRICSIC_PARAM )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, rc2.Height() + 7 );
	rc2.bottom = rc2.top + 35;

	GetDlgItem( IDC_CALIB_STATUS_GROUP )->MoveWindow( rc2 );

	rc2.OffsetRect( 10, 15 );
	//rc2.right = rc2.left + 150;
	rc2.bottom = rc2.top + 20;

	GetDlgItem( IDC_STATIC_CALI_STATUS )->MoveWindow( rc2 );

	//--------------------------------------------------
	// Measurement units
	GetDlgItem( IDC_CALIB_STATUS_GROUP )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, rc2.Height() + 7 );
	rc2.bottom = rc2.top + 45;

	GetDlgItem( IDC_MEASUREMENT_PARAM )->MoveWindow( rc2 );

	rc2.OffsetRect( 30, 15 );
	rc2.right = rc2.left + 100;
	rc2.bottom = rc2.top + 20;

	GetDlgItem( IDC_CALIB_UNITS_COMBO )->MoveWindow( rc2 );

	//---------------------------------------------------
	// Advanced params
	GetDlgItem( IDC_MEASUREMENT_PARAM )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, rc2.Height() + 7 );
	rc2.bottom = rc2.top + 110;

	GetDlgItem( IDC_ADV_PARAM )->MoveWindow( rc2 );

	// PAN
	rc2.OffsetRect( 20, 15 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_PAN )->MoveWindow( rc2 );

	rc2.OffsetRect( 70, 0 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_ED_PAN )->MoveWindow( rc2 );

	rc2.OffsetRect( 60, 0 );
	rc2.bottom = rc2.top + 18;
	rc2.right = rc2.left + 30;
	GetDlgItem( IDC_DEG2 )->MoveWindow( rc2 );

	GetDlgItem( IDC_PAN )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, 20 );
	rc2.right = rc2.left + 140;
	rc2.bottom = rc2.top + 20;

	GetDlgItem( IDC_PAN_SLIDER )->MoveWindow( rc2 );

	// ROLL
	rc2.OffsetRect( 0, 25 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_ROLL )->MoveWindow( rc2 );

	rc2.OffsetRect( 70, 0 );
	rc2.right = rc2.left + 50;
	rc2.bottom = rc2.top + 18;

	GetDlgItem( IDC_ED_ROLL )->MoveWindow( rc2 );

	rc2.OffsetRect( 60, 0 );
	rc2.bottom = rc2.top + 18;
	rc2.right = rc2.left + 30;
	GetDlgItem( IDC_DEG3 )->MoveWindow( rc2 );

	GetDlgItem( IDC_ROLL )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( 0, 20 );
	rc2.right = rc2.left + 140;
	rc2.bottom = rc2.top + 20;

	GetDlgItem( IDC_ROLL_SLIDER )->MoveWindow( rc2 );

	//---------------------------------------------------
	// Restore defaults
	GetDlgItem( IDC_ADV_PARAM )->GetWindowRect( &rc2 );
	ScreenToClient( &rc2 );

	rc2.OffsetRect( rc2.Width()/2 - 60, rc2.Height() + 5 );
	rc2.right = rc2.left + 150;
	rc2.bottom = rc2.top + 25;

	GetDlgItem( IDC_BUTTON_RESTORE )->MoveWindow( rc2 );
}

void CCalibrationCtl::FireOnEvent(DWORD uiEvent, DWORD uiContext)
{
	if( uiEvent & IVCAConfigureObserver::VCA_CALIB_UPDATE )
	{
		UpdateValues( );
	}
}

void CCalibrationCtl::UpdateValues( )
{
	// Flag prevents us setting the values from inside causing windows messages
	// to be sent as if it were a GUI user

	m_bApplyData = FALSE;

	VCA5_APP_CALIB_INFO *pInfo = m_pDataMgr->GetCalibInfo();

	//1.Update Edit control 
	TCHAR szfValue[10];

	// Check for imperial/metric
	if( VCA5_HEIGHT_UNITS_METERS == pInfo->ulHeightUnits )
	{
		_stprintf_s(szfValue, _countof(szfValue), _T("%5.2f"), pInfo->fHeight );
		GetDlgItem( IDC_ED_HEIGHT )->SetWindowText( szfValue);

		((CComboBox *)GetDlgItem( IDC_CALIB_UNITS_COMBO ))->SetCurSel( 0 );

		GetDlgItem( IDC_ED_HEIGHT2 )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_METER )->SetWindowText( _T("meters") );
	}
	else
	{
		float fFt = pInfo->fHeight;// * 3.2808399f;
		int ft = (int)floor( fFt );
		int in = (int)((float)(fFt - (float)ft) * 12);
		_stprintf_s(szfValue, _countof(szfValue), _T("%d"), ft);
		GetDlgItem( IDC_ED_HEIGHT )->SetWindowText( szfValue);

		GetDlgItem( IDC_ED_HEIGHT2 )->ShowWindow( SW_SHOW );

		GetDlgItem( IDC_METER )->SetWindowText( _T("ft/in"));

		_stprintf_s(szfValue, _countof(szfValue), _T("%d"), in);
		GetDlgItem( IDC_ED_HEIGHT2 )->SetWindowText( szfValue );

		((CComboBox *)GetDlgItem( IDC_CALIB_UNITS_COMBO ))->SetCurSel( 1 );

	}

	_stprintf_s(szfValue, _countof(szfValue), _T("%5.2f"), pInfo->fTilt);
	GetDlgItem( IDC_ED_TILT )->SetWindowText( szfValue );	
	_stprintf_s(szfValue, _countof(szfValue), _T("%5.2f"), pInfo->fFOV );
	GetDlgItem( IDC_ED_FOV )->SetWindowText( szfValue );
	((CSliderCtrl *)GetDlgItem( IDC_FOV_SLIDER ))->SetPos( (int) pInfo->fFOV );

	TCHAR strStatus[1024];

	switch(pInfo->calibrationStatus){
		case VCA5_CALIB_STATUS_CALIBRATED_OVERHEAD: _tcscpy_s(strStatus, _T("CALIBRATED_OVERHEAD")); break;
		case VCA5_CALIB_STATUS_CALIBRATED_SIDEON:	_tcscpy_s(strStatus, _T("CALIBRATED_SIDEON")); break;
		case VCA5_CALIB_STATUS_CALIBRATED:			_tcscpy_s(strStatus, _T("CALIBRATED")); break;
		case VCA5_CALIB_STATUS_UNCALIBRATED:		_tcscpy_s(strStatus, _T("UNCALIBRATED")); break;
		case VCA5_CALIB_STATUS_INVALIDSETUP:		_tcscpy_s(strStatus, _T("INVALIDSETUP")); break;
		case VCA5_CALIB_STATUS_FAILED_TOOFEW_MARKERS: _tcscpy_s(strStatus, _T("FAILED_TOOFEW_MARKERS")); break;
		case VCA5_CALIB_STATUS_FAILED_TILT_OUTOFRANGE: _tcscpy_s(strStatus, _T("FAILED_TILT_OUTOFRANGE")); break;
		case VCA5_CALIB_STATUS_FAILED_HEIGHT_OUTOFRANGE: _tcscpy_s(strStatus, _T("FAILED_HEIGHT_OUTOFRANGE	")); break;
		case VCA5_CALIB_STATUS_FAILED_FOV_OUTOFRANGE: _tcscpy_s(strStatus, _T("FOV_OUTOFRANGE")); break;
		default: strStatus[0] = NULL;
	}
	GetDlgItem( IDC_STATIC_CALI_STATUS )->SetWindowText( strStatus );

	_stprintf_s(szfValue, _countof(szfValue), _T("%5.2f"), pInfo->fPan );
	GetDlgItem( IDC_ED_PAN )->SetWindowText( szfValue );
	((CSliderCtrl *)GetDlgItem( IDC_PAN_SLIDER ))->SetPos( (int) pInfo->fPan );

	_stprintf_s(szfValue, _countof(szfValue), _T("%5.2f"), pInfo->fRoll );
	GetDlgItem( IDC_ED_ROLL )->SetWindowText( szfValue );
	((CSliderCtrl *)GetDlgItem( IDC_ROLL_SLIDER ))->SetPos( (int) pInfo->fRoll );

	m_bApplyData = TRUE;

}


void CCalibrationCtl::ApplyChanges()
{
	if( m_bApplyData )
	{
		// Things haven't changed due to an update from elsewhere, so go ahead and apply the changes back

		VCA5_APP_CALIB_INFO*	pCalibInfo = m_pDataMgr->GetCalibInfo();
		CRect rc1;
		TCHAR szTemp[128];

		CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_CALIB_UNITS_COMBO );
		int iUnits = pCombo->GetCurSel();

		if( 0 == iUnits )	// Metric
		{
			GetDlgItem( IDC_ED_HEIGHT )->GetWindowText( szTemp, 128 );
			_stscanf_s(szTemp, _T("%f"), &pCalibInfo->fHeight);

			pCalibInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_METERS;
			pCalibInfo->ulSpeedUnits	= VCA5_SPEED_UNITS_KPH;
		}
		else				// Imperial
		{
			int ft, in;
			GetDlgItem( IDC_ED_HEIGHT )->GetWindowText( szTemp, 128 );
			_stscanf_s(szTemp, _T("%d"), &ft);

			GetDlgItem( IDC_ED_HEIGHT2 )->GetWindowText( szTemp, 128);
			_stscanf_s(szTemp, _T("%d"), &in);

			pCalibInfo->fHeight = (float)ft + (((float)in) / 12.0f);

			pCalibInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_FEET;
			pCalibInfo->ulSpeedUnits	= VCA5_SPEED_UNITS_MPH;
		}

		GetDlgItem( IDC_ED_TILT )->GetWindowText( szTemp, 128);
		_stscanf_s(szTemp, _T("%f"), &pCalibInfo->fTilt );	

		GetDlgItem( IDC_ED_FOV )->GetWindowText( szTemp, 128);
		_stscanf_s(szTemp, _T("%f"), &pCalibInfo->fFOV);

		GetDlgItem( IDC_ED_PAN )->GetWindowText( szTemp, 128);
		_stscanf_s(szTemp, _T("%f"), &pCalibInfo->fPan);

		GetDlgItem( IDC_ED_ROLL )->GetWindowText( szTemp, 128);
		_stscanf_s(szTemp, _T("%f"), &pCalibInfo->fRoll);

		// For now, always set status to calibrated. We don't have an "uncalibrate" button yet, but when we do
		// we'll need to set the status accordingly if that's clicked.
		//pCalibInfo->calibrationStatus	= VCA5_CALIB_STATUS_CALIBRATED;
		m_pDataMgr->SetChangeCalibInfo(TRUE);

		m_pDataMgr->FireEvent( IVCAConfigureObserver::VCA_CALIB_UPDATE, this );
	}
}

void CCalibrationCtl::OnEnChangeEdHeight()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	VCA5_APP_CALIB_INFO *pInfo = m_pDataMgr->GetCalibInfo();
	TCHAR	szTemp[128];
	float 	fHeight;
	if( pInfo->ulSpeedUnits == VCA5_SPEED_UNITS_KPH ){
		GetDlgItem( IDC_ED_HEIGHT )->GetWindowText( szTemp, 128 );
		_stscanf_s(szTemp, _T("%f"), &fHeight);
		if(fHeight > 200){
			GetDlgItem( IDC_ED_HEIGHT )->SetWindowText( _T("200.00") );
			return;
		}

	}else{
		GetDlgItem( IDC_ED_HEIGHT )->GetWindowText( szTemp, 128 );
		_stscanf_s(szTemp, _T("%f"), &fHeight);
		if(fHeight > 660){
			GetDlgItem( IDC_ED_HEIGHT )->SetWindowText( _T("660.00") );
			GetDlgItem( IDC_ED_HEIGHT2 )->SetWindowText( _T("00") );

			return;
		}
	}
	ApplyChanges( );
}

void CCalibrationCtl::OnEnChangeEdHeight2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	VCA5_APP_CALIB_INFO *pInfo = m_pDataMgr->GetCalibInfo();
	TCHAR	szTemp[128];
	float 	fHeight;
	if( pInfo->ulSpeedUnits == VCA5_SPEED_UNITS_MPH ){
		GetDlgItem( IDC_ED_HEIGHT2 )->GetWindowText( szTemp, 128 );
		_stscanf_s(szTemp, _T("%f"), &fHeight);
		if(fHeight > 99){
			GetDlgItem( IDC_ED_HEIGHT2 )->SetWindowText( _T("99") );
			return;
		}
	}

	ApplyChanges( );
}

void CCalibrationCtl::OnEnChangeEdTilt()
{
	TCHAR	szTemp[128];
	float 	fTilt;
	GetDlgItem( IDC_ED_TILT )->GetWindowText( szTemp, 128 );
	_stscanf_s(szTemp, _T("%f"), &fTilt);
	if(fTilt > 120){
		GetDlgItem( IDC_ED_TILT )->SetWindowText( _T("120.00") );
		return;
	}

	ApplyChanges( );
}

void CCalibrationCtl::OnEnChangeEdFov()
{
	TCHAR	szTemp[128];
	float 	fFov;
	GetDlgItem( IDC_ED_FOV )->GetWindowText( szTemp, 128 );
	_stscanf_s(szTemp, _T("%f"), &fFov);
	if(fFov > 150){
		GetDlgItem( IDC_ED_FOV )->SetWindowText( _T("150.00") );
		return;
	}
	ApplyChanges( );
}

void CCalibrationCtl::OnEnChangeEdPan()
{
	TCHAR	szTemp[128];
	float 	fPan;
	GetDlgItem( IDC_ED_PAN )->GetWindowText( szTemp, 128 );
	_stscanf_s(szTemp, _T("%f"), &fPan);
	if(fPan > 90){
		GetDlgItem( IDC_ED_PAN )->SetWindowText( _T("90.00") );
		return;
	}

	if(fPan < -90){
		GetDlgItem( IDC_ED_PAN )->SetWindowText( _T("-90.00") );
		return;
	}

	ApplyChanges( );
}

void CCalibrationCtl::OnEnChangeEdRoll()
{
	TCHAR	szTemp[128];
	float 	fRoll;
	GetDlgItem( IDC_ED_ROLL )->GetWindowText( szTemp, 128 );
	_stscanf_s(szTemp, _T("%f"), &fRoll);
	if(fRoll > 90){
		GetDlgItem( IDC_ED_ROLL )->SetWindowText( _T("90.00") );
		return;
	}

	if(fRoll < -90){
		GetDlgItem( IDC_ED_ROLL )->SetWindowText( _T("-90.00") );
		return;
	}

	ApplyChanges( );
}

void CCalibrationCtl::OnCbnSelchangeCalibUnitsCombo()
{
	// TODO: Add your control notification handler code here

	VCA5_APP_CALIB_INFO *pInfo = m_pDataMgr->GetCalibInfo();

	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_CALIB_UNITS_COMBO );

	if( pCombo->GetCurSel() == 0 )
	{
		// Meters
		if( pInfo->ulHeightUnits == VCA5_HEIGHT_UNITS_FEET )
		{
			// Was ft, so change
			pInfo->fHeight *= 0.3048f;

			pInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_METERS;
			pInfo->ulSpeedUnits		= VCA5_SPEED_UNITS_KPH;
		}
	}
	else
	{
		// Ft/in

		if( pInfo->ulHeightUnits == VCA5_HEIGHT_UNITS_METERS )
		{
			// Was meters, so change
			pInfo->fHeight *= 3.2808399f;

			pInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_FEET;
			pInfo->ulSpeedUnits		= VCA5_SPEED_UNITS_MPH;
		}
	}

	// Update local values
	UpdateValues( );

	// Apply Changes
	ApplyChanges();

	// Resize
	PostMessage( WM_SIZE );
}


void CCalibrationCtl::OnBnClickedButtonRestore()
{
	// TODO: Add your control notification handler code here

	VCA5_APP_CALIB_INFO*	pCalibInfo = m_pDataMgr->GetCalibInfo();
	pCalibInfo->fHeight		= 10;
	pCalibInfo->fTilt		= 30;
	pCalibInfo->fFOV		= 40;
	pCalibInfo->fRoll		= 0;
	pCalibInfo->fPan		= 0;
	pCalibInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_METERS;
	pCalibInfo->ulSpeedUnits	= VCA5_SPEED_UNITS_KPH;
	//pCalibInfo->calibrationStatus	= VCA5_CALIB_STATUS_CALIBRATED;
	m_pDataMgr->SetChangeCalibInfo(TRUE);
	m_pDataMgr->ResetObjectModel();

	// Update our own view
	UpdateValues();

	// Apply Changes
	ApplyChanges();
}

void CCalibrationCtl::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	if( pScrollBar )
	{
		if( IDC_FOV_SLIDER == pScrollBar->GetDlgCtrlID() )
		{
			int pos = ((CSliderCtrl *)pScrollBar)->GetPos();
			CString sPos;
			sPos.Format( _T("%d"), pos );
			GetDlgItem( IDC_ED_FOV )->SetWindowText( sPos );
		}

		if( IDC_PAN_SLIDER == pScrollBar->GetDlgCtrlID() )
		{
			int pos = ((CSliderCtrl *)pScrollBar)->GetPos();
			CString sPos;
			sPos.Format( _T("%d"), pos );
			GetDlgItem( IDC_ED_PAN )->SetWindowText( sPos );
		}

		if( IDC_ROLL_SLIDER == pScrollBar->GetDlgCtrlID() )
		{
			int pos = ((CSliderCtrl *)pScrollBar)->GetPos();
			CString sPos;
			sPos.Format( _T("%d"), pos );
			GetDlgItem( IDC_ED_ROLL )->SetWindowText( sPos );
		}
	}
}

BOOL CCalibrationCtl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	if(IDOK == wmId){
		return TRUE;
	}

	if(IDCANCEL  == wmId){
		return TRUE;
	}

	return CDialog::OnCommand(wParam, lParam);

}
