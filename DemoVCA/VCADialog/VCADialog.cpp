#include "stdafx.h"
#include "../resource.h"

#include "VCADialog.h"

#include "../DemoVCADlg.h"
#include "../VCAEngine/VCAEngine.h"
#include "../Render/D3DCalibrationRender.h"
//#include "../VideoSource/VideoSourceSettingDlg.h"
#include "../common/APPConfigure.h"
#include "../Common/wm_user.h"
#include "../utils/memdc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




IMPLEMENT_DYNAMIC(CVCADialog, CDialog)

BEGIN_MESSAGE_MAP(CVCADialog, CDialog)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()

	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CVCADialog::CVCADialog( int iEngId, CWnd* pParent /*=NULL*/)
	: CDialog(CVCADialog::IDD, pParent)
{
	
	m_iEngId			= iEngId;
	m_pEngine			= NULL;
	m_SrcId				= 0;
	m_pVCARender		= NULL;
	m_eViewMode			= VIEW_MODE_PREVIEW;
	m_pSimpleVCAAppDlg	= (CDemoVCADlg*)pParent ;

	m_ZoneCtrl.Reset();
}

CVCADialog::~CVCADialog()
{	
	if(m_pVCARender){
		m_pVCARender->Endup();
		delete m_pVCARender;
		m_pVCARender = NULL;
	}
}

void	CVCADialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CVCADialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect Rect;
	m_eViewMode		= VIEW_MODE_PREVIEW;
	m_pVCARender	= new CCalibrationD3DRender();
	
	if(!m_pVCARender->Setup(GetSafeHwnd(), &m_DataMgr)){
		MessageBox(_T("Can not Create D3D surface \n"));
		delete m_pVCARender;
		return FALSE;
	}


	m_ZoneCtrl.Setup(this);
	m_CalibrationCtrl.Setup(this);
		
	ChangeViewMode( VIEW_MODE_PREVIEW );

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CVCADialog::SetEngine(CEngine *pEngine)
{
	m_pEngine = pEngine;
}

void CVCADialog::OnDestroy()
{
	
	CDialog::OnDestroy();
}

void CVCADialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{

	DWORD eViewModePrev = m_eViewMode;
	switch(m_eViewMode)
	{
		case VIEW_MODE_PREVIEW:
		{
			if( GetParent() )
			{
				GetParent()->PostMessage( WM_CONFIG_MODE, m_iEngId, 1  );
			}
		}
		break;

		case VIEW_MODE_CONFIGURE:
		{
			m_ZoneCtrl.OnLButtonDblClk( nFlags, point );
		}
		break;

		default:
		// No action
		break;
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CVCADialog::OnLButtonDown(UINT nFlags, CPoint point)
{
//	if(!m_pEngine) return;

	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnLButtonDown(nFlags, point);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnLButtonDown(nFlags, point);
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CVCADialog::OnLButtonUp(UINT nFlags, CPoint point)
{
//	if(!m_pEngine) return;

	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnLButtonUp(nFlags, point);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnLButtonUp(nFlags, point);
	}

	CDialog::OnLButtonUp(nFlags, point);
}


void CVCADialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnRButtonUp(nFlags, point);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CVCADialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (VIEW_MODE_PREVIEW == m_eViewMode)
	{
		ClientToScreen(&point);
		HMENU hVCAMenu	= CreatePopupMenu();
		AppendMenu( hVCAMenu, MF_STRING,	ID_CONFIG_MODE, _T("Configure Settings..."));
		AppendMenu( hVCAMenu, MF_STRING,	ID_EXPORT_CONFIG, _T("Export Configuration..."));
		AppendMenu( hVCAMenu, MF_STRING,	ID_IMPORT_CONFIG, _T("Import Configuration..."));

		if(!m_pEngine){
			EnableMenuItem(hVCAMenu, ID_IMPORT_CONFIG, MF_DISABLED|MF_GRAYED);
			EnableMenuItem(hVCAMenu, ID_EXPORT_CONFIG, MF_DISABLED|MF_GRAYED);
		}
		
		TrackPopupMenu(hVCAMenu, TPM_LEFTALIGN | TPM_TOPALIGN, point.x, point.y, 0, m_hWnd, NULL);
	} else if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnRButtonDown(nFlags, point);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode){
		m_CalibrationCtrl.OnRButtonDown(nFlags, point);
	}

	CDialog::OnRButtonDown(nFlags, point);
}

void CVCADialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnMouseMove(nFlags, point);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnMouseMove(nFlags, point);
	}
	CDialog::OnMouseMove(nFlags, point);
}

BOOL CVCADialog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnMouseWheel(nFlags, zDelta, pt);
	}else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnMouseWheel(nFlags, zDelta, pt);
	}

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}

void CVCADialog::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);
	
	GetClientRect(&m_ClientRect);
	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnChangeClientRect(m_ClientRect);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_ClientRect.right -= m_ClientRect.right/4;
		m_CalibrationCtrl.OnChangeClientRect(m_ClientRect);
	} 

	if(m_pVCARender) m_pVCARender->OnChangeClientRect(m_ClientRect);
}
	


void CVCADialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	GetClientRect(&m_ClientRect);

	CRect rc = m_ClientRect;
	rc.right = min( rc.right, 1023);
	rc.bottom = min( rc.bottom, 1023);

	if (VIEW_MODE_CONFIGURE == m_eViewMode) {
		m_ZoneCtrl.OnChangeClientRect(rc);
	} else if (VIEW_MODE_CALIBRATION == m_eViewMode) {
		m_CalibrationCtrl.OnChangeClientRect(rc);
		SIZE videoSize = {rc.Width(), rc.Height()};
		m_CalibrationCtrl.SetVideoInputSize(videoSize);
		
	} 

	if(m_pVCARender) m_pVCARender->OnChangeClientRect(rc);
}


void CVCADialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

BOOL	CVCADialog::ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm)
{
	if(EngId != m_iEngId/*m_pEngine->GetEngId()*/) { ASSERT( FALSE ); return FALSE; }

	SIZE videoSize ={pbm->biWidth, pbm->biHeight};
	m_CalibrationCtrl.SetVideoInputSize(videoSize);

	BOOL bOk = m_pVCARender->ChangeVCASourceInfo(pbm);

	// Force a repaint
	Invalidate();

	return bOk;
}

BOOL	CVCADialog::ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib)
{
	if(EngId != m_iEngId/*m_pEngine->GetEngId()*/) return FALSE;
	//Update Counter. 

	VCA5_APP_COUNTERS* pVCA5Counters	= m_DataMgr.GetCounters();;
	BOOL bUpdateCounterTreeList			= FALSE;
	VCA5_PACKET_COUNTS*	pPacketCounters = pVCAMetaLib->GetCounts();

	for(ULONG i = 0 ; i < pVCA5Counters->ulTotalCounters ; i++){
		for(ULONG j = 0 ; j < pPacketCounters->ulTotalCounter ; j++){
			if((pVCA5Counters->pCounters[i].usCounterId == pPacketCounters->Counters[j].ulId) && 
				!(pVCA5Counters->pCounters[i].uiStatus&VCA5_APP_COUNTER_STATUS_NOT_USED)){
				if(pVCA5Counters->pCounters[i].iCounterResult != pPacketCounters->Counters[j].iVal){
					bUpdateCounterTreeList = TRUE;
					pVCA5Counters->pCounters[i].iCounterResult = pPacketCounters->Counters[j].iVal;
				}
			}
		}
	}
	
	if(bUpdateCounterTreeList && (VIEW_MODE_CONFIGURE == m_eViewMode)){
		m_DataMgr.FireEvent(IVCAConfigureObserver::VCA_COUNTEROUT_UPDATE, &m_ZoneCtrl);
	}

	ULONG	ulImageSize	= 0;
	RECT	rcImageROI	= {0,};
	RECT	rcVCAROI	= {0,};
	
	ULONG	ulRotate	= CAPPConfigure::Instance()->GetAPPEngineInfo( m_iEngId )->tSourceData.ulRotate;
	VCA5_RECT	rcROIEng= CAPPConfigure::Instance()->GetAPPEngineInfo( m_iEngId )->tSourceData.rcROI;
	m_pEngine->GetVideoSource()->Control(IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD)&ulImageSize, 0);

	if(VIEW_MODE_PREVIEW == m_eViewMode){
		rcImageROI.right	= VCA5_GETIMGWIDTH(ulImageSize);
		rcImageROI.bottom	= VCA5_GETIMGHEIGHT(ulImageSize);
		rcVCAROI.left	= rcROIEng.x;
		rcVCAROI.top	= rcROIEng.y;
		rcVCAROI.right	= rcROIEng.x + rcROIEng.w;
		rcVCAROI.bottom	= rcROIEng.y + rcROIEng.h;
	} else {
		rcImageROI.left	= rcROIEng.x;
		rcImageROI.top	= rcROIEng.y;
		rcImageROI.right	= rcROIEng.x + rcROIEng.w;
		rcImageROI.bottom	= rcROIEng.y + rcROIEng.h;
	}

	if( m_pEngine && (m_pEngine->GetFunction() & (VCA5_FEATURE_METADATA|VCA5_FEATURE_LINECOUNTER)) )
	{
		m_pVCARender->RenderVCAData(pImage, pbm, ulRotate, pVCAMetaLib, rcImageROI, rcVCAROI, pTimestamp);
	}
	else //If not support Metadata using VCA Meta Render
	{
		m_pVCARender->RenderVCAMetaData(pImage, pbm, ulRotate, pMetaData, iMataDataLen, rcImageROI, rcVCAROI, pTimestamp);
	}

	return TRUE;
}

void	CVCADialog::SplitMove(int x, int y, int cx, int cy)
{
	if (VIEW_MODE_PREVIEW == m_eViewMode ||
		VIEW_MODE_DISABLED == m_eViewMode ) {
		MoveWindow(x, y, cx, cy);
	//	ShowWindow(SW_SHOW);
	}
}

void	CVCADialog::ChangeViewMode( VIEW_MODE viewMode, BOOL bForce )
{
	if( !m_pEngine && !bForce)
	{
		return;
	}

//	m_DataMgr.Reset();
//	m_ZoneCtrl.Reset();

	switch( viewMode )
	{
		case VIEW_MODE_PREVIEW:
		{
			m_pVCARender->SetRenderMode(IVCARender::RENDER_VCA);

			m_ZoneCtrl.Reset();

		}
		break;

		case VIEW_MODE_CONFIGURE:
		{
			m_pVCARender->SetRenderMode(IVCARender::RENDER_VCA);
		}
		break;

		case VIEW_MODE_TAMPER:
		{
			m_pVCARender->SetRenderMode(IVCARender::RENDER_TAMPER);
		}
		break;

		case VIEW_MODE_CALIBRATION:
		{
			m_pVCARender->SetRenderMode(IVCARender::RENDER_CALIBRATION);
			m_CalibrationCtrl.Reset();
		}
		break;
	} 

	m_eViewMode = viewMode;
}



BOOL CVCADialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	
	if((wmId >= ID_DISPLAY_BLOBS ) && (wmId <= ID_DISPLAY_TAMPER_ALARM)){ //Display settung
		DWORD SelectOption, CurDisplayOption = m_DataMgr.GetDisplayFlags();
		if(ID_DISPLAY_BLOBS == wmId){
			SelectOption = IVCARender::DISPLAY_BLOBS;
		}else if(ID_DISPLAY_NON_ALARMS == wmId){
			SelectOption = IVCARender::DISPLAY_NON_ALARMS;
		}else if(ID_DISPLAY_TARGETID == wmId){
			SelectOption = IVCARender::DISPLAY_TARGETID;
		}else if(ID_DISPLAY_OBJECTHEIGHT == wmId){
			SelectOption = IVCARender::DISPLAY_OBJECT_HEIGHT;
		}else if(ID_DISPLAY_OBJECTSPEED == wmId){
			SelectOption = IVCARender::DISPLAY_OBJECT_SPEED;
		}else if(ID_DISPLAY_OBJECTAREA == wmId){
			SelectOption = IVCARender::DISPLAY_OBJECT_AREA;
		}else if(ID_DISPLAY_OBJECTCLASSIFICATION == wmId){
			SelectOption = IVCARender::DISPLAY_OBJECT_CLASSIFICATION;
		}else if(ID_DISPLAY_OBJECTCOLORSIGNATURE == wmId){
			SelectOption = IVCARender::DISPLAY_OBJECT_COLOR_SIGNATURE;
//		}else if(ID_DISPLAY_TAMPER_ALARM == wmId){
//			SelectOption = IVCARender::DISPLAY_TAMPER_ALARM;
		}

		if(SelectOption&CurDisplayOption) 
			CurDisplayOption &= (~SelectOption);
		else
			CurDisplayOption |= SelectOption;
		m_DataMgr.SetDisplayFlags(CurDisplayOption);

		if(ID_DISPLAY_BLOBS == wmId){
			EnableBlob(CurDisplayOption&IVCARender::DISPLAY_BLOBS);
		}
		
		if(ID_DISPLAY_OBJECTCOLORSIGNATURE == wmId){
			EnableColorSignatute(CurDisplayOption&IVCARender::DISPLAY_OBJECT_COLOR_SIGNATURE);
		}

		if(VIEW_MODE_CONFIGURE == m_eViewMode){
			POINT point = m_ZoneCtrl.GetCurPoint();
			m_ZoneCtrl.ShowMenu(point, 0);
		}
				
	}else if(VIEW_MODE_CONFIGURE == m_eViewMode){
		m_ZoneCtrl.OnCommand(wParam, lParam);
		if(IDOK == wmId){
			return TRUE;
		}else if(IDCANCEL == wmId){
			m_pSimpleVCAAppDlg->SendMessage( WM_CONFIG_MODE, (WPARAM)GetEngId() , 0 );
			return TRUE;
		}
	}else if(VIEW_MODE_CALIBRATION == m_eViewMode){
		m_CalibrationCtrl.OnCommand(wParam, lParam);
		if(IDOK == wmId){
			return TRUE;
		}else if(IDCANCEL == wmId){
			m_pSimpleVCAAppDlg->SendMessage( WM_CONFIG_MODE, (WPARAM)GetEngId() , 0 );
			return TRUE;
		}
	}else{	//VIEW_MODE_PREVIEW Mode
		// Popup Dialog

		switch( wmId )
		{
			case ID_CONFIG_MODE:
			{
				if( GetParent() )
				{
					GetParent()->PostMessage( WM_CONFIG_MODE, m_iEngId, 1  );
				}
			}
			break;

			case ID_IMPORT_CONFIG:
			{
				if( GetParent() )
				{
					GetParent()->PostMessage( WM_IMPORT_CONFIG, m_iEngId, NULL );
				}
			}
			break;

			case ID_EXPORT_CONFIG:
			{
				if( GetParent() )
				{
					GetParent()->PostMessage( WM_EXPORT_CONFIG, m_iEngId, NULL );
				}
			}
			break;

			case IDOK:
			case IDCANCEL:
				return TRUE;
		}
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void	CVCADialog::ApplyZoneSetting()
{
	if(!m_pEngine) return;

	VCA5_APP_ZONES zones;
	VCA5_APP_COUNTERS counters;
	VCA5_APP_RULES rules;
	VCA5_APP_CLSOBJECTS clsObjs;
	VCA5_APP_CALIB_INFO calibInfo;

	m_DataMgr.GetData( &zones, &rules, &counters, &calibInfo, &clsObjs );

	//Counterline rule setting
	for(ULONG i = 0 ; i < rules.ulTotalRules ; ++i){
		if(VCA5_RULE_TYPE_LINECOUNTER_A == rules.pRules[i].usRuleType || 
			VCA5_RULE_TYPE_LINECOUNTER_B == rules.pRules[i].usRuleType){
				if(rules.pRules[i].ucWidthCalibrationEnabled == 0){
					rules.pRules[i].tRuleDataEx.tLineCounter.ulCalibrationWidth = 0;
				}
				rules.pRules[i].tRuleDataEx.tLineCounter.ulShadowFilterEnabled = rules.pRules[i].ucShadowFilterEnabled;
		}
	}

	//2. Apply Engine
	if( m_pEngine )
	{
		m_pEngine->SetConfig(&zones, &rules, &counters );//(m_DataMgr.GetZones(), &TempRules, &TempCounters);
	}

	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf( m_iEngId );

	VCA5_APP_COUNTERS *pCounters = m_DataMgr.GetCounters();
	for (ULONG i = 0; i < pCounters->ulTotalCounters; ++i) 
	{
		if(pCounters->pCounters[i].uiStatus&VCA5_APP_COUNTER_STATUS_NOT_USED)
		{
			ClearAreaStatusBit(&(pCounters->pCounters[i]), VCA5_APP_COUNTER_STATUS_NOT_USED);
		}else{
			//Update counter result 
			m_pEngine->SetControl(VCA5_CMD_SETCOUNTERRES, pCounters->pCounters[i].usCounterId, pCounters->pCounters[i].iCounterResult);
		}
	}

	m_DataMgr.GetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
	m_DataMgr.ClearCountingLineEvents();
	
}


void CVCADialog::ApplyCalibInfoSetting()
{
	//Apply Engine setting.
	VCA5_APP_CALIB_INFO* pClibInfo	= m_DataMgr.GetCalibInfo();
	VCA5_ENGINE_PARAMS* pParams = m_pEngine->GetEngineParams();

	USHORT	InputWidth, InputHeight;
	InputWidth	= (USHORT)VCA5_GETIMGWIDTH(pParams->ulImageSize);
	InputHeight	= (USHORT)VCA5_GETIMGHEIGHT(pParams->ulImageSize);

	//as calibrationStatus parameter so set this value 
	if(m_DataMgr.IsChangeCalibInfo()){
		pClibInfo->calibrationStatus = VCA5_CALIB_STATUS_CALIBRATED;
		m_DataMgr.SetChangeCalibInfo(FALSE);
	}

	m_pEngine->SetClibInfo(pClibInfo, InputWidth, InputHeight);
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);
	memcpy(&(pEngCfg->CalibInfo), pClibInfo, sizeof(VCA5_APP_CALIB_INFO));

	m_DataMgr.FireEvent(IVCAConfigureObserver::VCA_CALIB_UPDATE, NULL);
	m_DataMgr.m_bResetObjectModel = TRUE;

	DecideDrawTrailMode();
}

void CVCADialog::DecideDrawTrailMode()
{
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);
	const int FORCED_CENTROID = 1, FORCED_MIDBOTTOM = 2;

	if ( FORCED_CENTROID == pEngCfg->AdvInfo.TrackerParams.ulDetectionPoint )
		m_DataMgr.SetDrawTrailMode(TRUE);
	else if ( FORCED_MIDBOTTOM == pEngCfg->AdvInfo.TrackerParams.ulDetectionPoint )
		m_DataMgr.SetDrawTrailMode(FALSE);
	else if ( VCA5_CALIB_STATUS_CALIBRATED_OVERHEAD == m_DataMgr.GetCalibInfo()->calibrationStatus )
		m_DataMgr.SetDrawTrailMode(TRUE);
	else
		m_DataMgr.SetDrawTrailMode(FALSE);
}


void CVCADialog::ApplyClassObjsSetting()
{
	//Apply Engine setting.
	VCA5_APP_CLSOBJECTS* pClsObjs = m_DataMgr.GetClsObjects();
	m_pEngine->SetClassObjs(pClsObjs);
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);
	memcpy(&(pEngCfg->ClsObjects), pClsObjs, sizeof(VCA5_APP_CLSOBJECTS));
}

void CVCADialog::ApplyAdvancedSetting()
{
	VCA5_APP_ADVANCED_INFO *pInfo = m_DataMgr.GetAdvancedInfo();

	m_pEngine->SetControl( VCA5_CMD_SETSTABENABLE, (ULONG) pInfo->bEnableStab );
	m_pEngine->SetControl( VCA5_CMD_SETRETRIGTIME, pInfo->ulRetrigTime );

	m_pEngine->SetControl( VCA5_CMD_SETTRACKERPARAMS, (ULONG) &(pInfo->TrackerParams));
	
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);

	memcpy( &(pEngCfg->AdvInfo), pInfo, sizeof( VCA5_APP_ADVANCED_INFO ) );

	DecideDrawTrailMode();
}

void CVCADialog::ApplyTamperInfo()
{
	VCA5_TAMPER_INFO*	pTamperInfo = m_DataMgr.GetTamperInfo();
	m_pEngine->SetControl( VCA5_CMD_SETTAMPERPARAMS, (ULONG)pTamperInfo );
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);
	memcpy(&(pEngCfg->TamperInfo), pTamperInfo, sizeof(VCA5_TAMPER_INFO));
}

void CVCADialog::ApplySceneChangeInfo()
{
	VCA5_SCENECHANGE_INFO*	pSceneChangeInfo = m_DataMgr.GetSceneChangeInfo();
	m_pEngine->SetControl( VCA5_CMD_SETSCENECHANGEPARAMS, (ULONG)pSceneChangeInfo );
	DWORD eng = m_pEngine->GetEngId();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(eng);
	memcpy(&(pEngCfg->SceneChangeInfo), pSceneChangeInfo, sizeof(VCA5_SCENECHANGE_INFO));
}

void CVCADialog::ApplyEngineSetting()
{
	// Copy from datamgr to appconfig to commit the changes
	VCA5_APP_ENGINE_INFO *pInfo = m_DataMgr.GetEngineInfo();

	VCA5_APP_ENGINE_INFO *pAppInfo = CAPPConfigure::Instance()->GetAPPEngineInfo( m_iEngId );

	if( pAppInfo )
	{
		memcpy( pAppInfo, pInfo, sizeof( VCA5_APP_ENGINE_INFO ) );
	}
}


void CVCADialog::EnableBlob( BOOL bDisplay )
{
	ULONG MetaFmt;
	if(m_pEngine->SetControl( VCA5_CMD_GETMETAFMT, (ULONG)&MetaFmt)){
		if(bDisplay) {
			MetaFmt = MetaFmt|VCA5_METAFMT_BLOB;
			MetaFmt = MetaFmt|VCA5_METAFMT_SMOKEFIRE;
		} else {
			MetaFmt = MetaFmt&(~VCA5_METAFMT_BLOB);
			MetaFmt = MetaFmt&(~VCA5_METAFMT_SMOKEFIRE);
		}
		m_pEngine->SetControl( VCA5_CMD_SETMETAFMT,  MetaFmt);
	}
}


void CVCADialog::EnableColorSignatute( BOOL bDisplay )
{
	ULONG MetaFmt;
	if(m_pEngine->SetControl( VCA5_CMD_GETMETAFMT, (ULONG)&MetaFmt)){
		if(bDisplay) {
			MetaFmt = MetaFmt|VCA5_METAFMT_COLORSIG;
		} else {
			MetaFmt = MetaFmt&(~VCA5_METAFMT_COLORSIG);
		}
		m_pEngine->SetControl( VCA5_CMD_SETMETAFMT,  MetaFmt);
	}
}


BOOL CVCADialog::OnEraseBkgnd( CDC *pDC )
{
	if( VIEW_MODE_PREVIEW == m_eViewMode )
	{
		return TRUE;
	}

	return CDialog::OnEraseBkgnd( pDC );
}

void CVCADialog::OnPaint()
{
	
	CPaintDC dc(this); // device context for painting

	CRect rc;
	GetClientRect( &rc );

	if(m_pEngine)
	{
		if( CEngine::VCA_ENGINE_STREAMING != m_pEngine->GetStatus() )
		{
			CMemDC dcMem( &dc );

			dcMem.FillSolidRect( rc, RGB(0,0,0));

			CBitmap bmp;
			bmp.LoadBitmap( IDB_NOSIGNAL );

			CDC dcBmp;
			dcBmp.CreateCompatibleDC( &dcMem );
			CBitmap *pOldBmp = dcBmp.SelectObject( &bmp );

			BITMAP bm;
			bmp.GetBitmap( &bm );

			int sml = min( rc.Width(), rc.Height() );
			int x, y;
			x = sml == rc.Width() ? 0 : (rc.Width() - sml)/2;
			y = sml == rc.Height() ? 0 : (rc.Height() - sml)/2;

			dcMem.StretchBlt( x, y, sml, sml, &dcBmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

			dcBmp.SelectObject( pOldBmp );

			bmp.DeleteObject();
		}
	}
	else
	{
		dc.FillSolidRect(rc,  RGB(0,0,0) );
		dc.SetTextColor( RGB( 255, 255, 255) );
		CString s = _T("");
		switch( m_eSourceStatus )
		{
			case SS_OK:	
			{
				s = _T("OK");
				dc.DrawText( s, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
			}
			break;
			case SS_NOTASSIGNED:
			{
				s = _T("VIDEO SOURCE NOT ASSIGNED");
				dc.DrawText( s, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
			}
			break;
			case SS_ERROR:	
			{
				s = _T("ERROR OPENING VIDEO SOURCE");
				dc.DrawText( s, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );

				rc.OffsetRect( 0, 30 );
				s.Format( _T("ENGINE: %d TYPE: %s"), m_iEngId, 
					IVCAVideoSource::GetSourceTypeStr( (IVCAVideoSource::eSOURCETYPE)CAPPConfigure::Instance()->GetAPPEngineInfo(m_iEngId)->tSourceData.SourceType) );
				dc.DrawText( s, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
			}
			break;
		}
	}
}


void CVCADialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if ( VK_DELETE == nChar && VIEW_MODE_CONFIGURE == m_eViewMode)
	{
		m_ZoneCtrl.RemoveNode();
		m_ZoneCtrl.RemoveZone();
		m_ZoneCtrl.RemoveCounter();
	}

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}
