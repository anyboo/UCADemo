// AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "AppDlg.h"
#include "./VCADialog/VCADialog.h"
#include "./VCADialog/BlankDialog.h"
#include "./VCAEngine/VCAEngine.h"
#include "wm_user.h"
#include "VideoSource/VCAVideoSource.h"


// CAppDlg dialog

IMPLEMENT_DYNAMIC(CAppDlg, CDialog)

CAppDlg::CAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_eDisplayMode = DISPLAY_PREVIEW;

	memset(m_pVCADialogs, 0, sizeof(CVCADialog*)*VCA5_MAX_NUM_ENGINE);
	memset(m_pVideoSources, 0, sizeof(IVCAVideoSource*)*VCA5_MAX_NUM_ENGINE);
	memset(m_pBlankDialogs, 0, sizeof(CBlankDialog *) * VCA5_MAX_NUM_ENGINE);
}

CAppDlg::~CAppDlg()
{
	for (ULONG i = 0; i < VCA5_MAX_NUM_ENGINE; i++) {
		SAFE_DELETE(m_pVCADialogs[i]);
	}

	for (ULONG i = 0; i < VCA5_MAX_NUM_ENGINE; i++) {
		SAFE_DELETE(m_pBlankDialogs[i]);
	}

	for (ULONG i = 0; i < VCA5_MAX_NUM_ENGINE; i++) {
		if(m_pVideoSources[i])DestroyVCAVideoSource(m_pVideoSources[i]);
	}
}

void CAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAppDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE( WM_EXPORT_CONFIG, &CAppDlg::OnExportConfig )
	ON_MESSAGE( WM_IMPORT_CONFIG, &CAppDlg::OnImportConfig)
	ON_MESSAGE( WM_CONFIG_MODE, &CAppDlg::OnConfigMode)
	ON_MESSAGE( WM_VIDEO_SRC_CHANGED, &CAppDlg::OnVideoSrcChanged)
	ON_MESSAGE( WM_SETUP, &CAppDlg::OnSetup )
	ON_MESSAGE( WM_CONFIG_UPDATED, &CAppDlg::OnConfigUpdated )
END_MESSAGE_MAP()


// CAppDlg message handlers

BOOL CAppDlg::OnInitDialog()
{
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	TCHAR *szDllName		= CAPPConfigure::Instance()->GetCAP5DllPath();
	TCHAR *szModelXMLName	= CAPPConfigure::Instance()->GetCAP5ModelInfoPath();
	
	if(!m_CAP5System.Setup(szDllName, szModelXMLName)) {
		TRACE("Can not Setup CAP5System\n");
	}
	CAPPConfigure::Instance()->SetCapBoardsInfo(m_CAP5System.GetCapBoardInfo(), m_CAP5System.GetUSNs(), m_CAP5System.GetCapBoardsNum());


	HINSTANCE h = AfxGetInstanceHandle();

	// setup child dialogs and alarm list controls
	CRect Rect;
	GetClientRect(&Rect);

	m_AlarmListCtrl.Create(WS_CHILD | WS_BORDER | LVS_REPORT, Rect, this, IDC_ALARMLIST );
	m_AlarmListCtrl.SetExtendedStyle( m_AlarmListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
	m_AlarmListCtrl.Initialize();

	m_ConfigDlg.Create( CConfigDlg::IDD, this );
	m_ConfigDlg.ShowWindow( SW_HIDE );

	// Set up once all thw windows have been created
	PostMessage( WM_SETUP );

	return TRUE;
}

LRESULT CAppDlg::OnSetup( WPARAM wParam, LPARAM lParam )
{
	ULONG i;

	for (i = 0; i < GetNumEngine(); i++)
	{
		m_pVCADialogs[i]= new CVCADialog(i, this);

		m_pVCADialogs[i]->Create(CVCADialog::IDD, this);

		SetupEngine( i, TRUE );

		m_pVCADialogs[i]->ShowWindow( SW_SHOW );
	}

	// Now create some stub dialogs for those that don't have engines
	ULONG ulNumSplit = m_SplitConfig.cx * m_SplitConfig.cy;
	for(i = GetNumEngine(); i < ulNumSplit; i++ )
	{
		m_pBlankDialogs[i] = new CBlankDialog( this );
		m_pBlankDialogs[i]->Create( CBlankDialog::IDD, this );
	}

	Split(m_SplitConfig);

	m_CAP5System.Run();

	return 0;
}


IVCAVideoSource *CAppDlg::CreateVideoSource(int iEngId)
{
	BOOL bSourceLoad = FALSE;
	IVCAVideoSource *pSrc = NULL;
	
	VCA5_APP_VIDEOSRC_INFO *pInfo = GetSrcInfo( iEngId );

	// Set up the video source

	switch(pInfo->SourceType)
	{
		case IVCAVideoSource::COMPRESSEDFILESOURCE:
		case IVCAVideoSource::RAWFILESOURCE:
		{
			pSrc	= CreateVCAVideoSource((IVCAVideoSource::eSOURCETYPE) pInfo->SourceType, pInfo->szRawPath, 0);
		}
		break;
		case IVCAVideoSource::STREAMSOURCE:
		{
			pSrc	= CreateVCAVideoSource((IVCAVideoSource::eSOURCETYPE) pInfo->SourceType, pInfo->szVlcPath, 0);
		}
		break;

		case IVCAVideoSource::DSHOWSOURCE:
		{
			pSrc	= CreateVCAVideoSource((IVCAVideoSource::eSOURCETYPE) pInfo->SourceType, pInfo->szDShowURL, pInfo->ulDShowDeviceId );
		}
		break;

		case IVCAVideoSource::CAP5SOURCE:
		{
			pSrc	= CreateVCAVideoSource(IVCAVideoSource::CAP5SOURCE, (LPCTSTR)&m_CAP5System, 
			((pInfo->Bd)<<16|(pInfo->Ch)));
		}
		break;

		case IVCAVideoSource::NOTSETSOURCE:
		{
			// Unset
		}
		break;

		case IVCAVideoSource::VIRTUALSOURCE:
		{
			pSrc = CreateVCAVideoSource( (IVCAVideoSource::eSOURCETYPE)pInfo->SourceType, NULL, NULL );
		}
		break;

		default:
		{
			// Unknown source type
			ASSERT( FALSE );
		}
	}

	if( pSrc )
	{
		//Setup Video Source
		if(IVCAVideoSource::CAP5SOURCE == pInfo->SourceType)
		{
			pSrc->Control(IVCAVideoSource::CMD_SET_VIDEOFORMAT, pInfo->ulVideoFormat, 0);
			pSrc->Control(IVCAVideoSource::CMD_SET_COLORFORMAT, pInfo->ulColorFormat, 0);
			pSrc->Control(IVCAVideoSource::CMD_SET_IMGSIZE, pInfo->ulImageSize, 0);
			pSrc->Control(IVCAVideoSource::CMD_SET_FRAMERATE, pInfo->ulFrameRate, 0);
		}
		else
		{
			pSrc->Control(IVCAVideoSource::CMD_GET_VIDEOFORMAT, (DWORD)&(pInfo->ulVideoFormat), 0);
			pSrc->Control(IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD)&(pInfo->ulColorFormat), 0);
			pSrc->Control(IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD)&(pInfo->ulImageSize), 0);
			pSrc->Control(IVCAVideoSource::CMD_SET_FRAMERATE, pInfo->ulFrameRate, 0);
		}
	}

	return pSrc;
}

BOOL CAppDlg::SetupEngine( int iEngId, BOOL bUpdateDisplayFlagsOfDataMgr )
{
	BOOL bOk = FALSE;

	CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
	CVCADataMgr *pVCADataMgr;
	CEngine	*pEngine;

	pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(iEngId);
	pVCADataMgr->SetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
	pVCADataMgr->SetAdvancedInfo( &(pEngCfg->AdvInfo) );
	pVCADataMgr->SetTamperInfo(&(pEngCfg->TamperInfo));
	pVCADataMgr->SetSceneChangeInfo(&(pEngCfg->SceneChangeInfo));

	if( pAppCfg->GetAPPEngineInfo( iEngId ) )
	{
		pVCADataMgr->SetEngineInfo( pAppCfg->GetAPPEngineInfo( iEngId ) );

		if ( bUpdateDisplayFlagsOfDataMgr )
			pVCADataMgr->SetDisplayFlags(pAppCfg->GetAppEngineDisplayFlag(iEngId));
	}

	// Create the video source
	IVCAVideoSource *pSrc = CreateVideoSource( iEngId );
	if( pSrc )
	{
		m_pVideoSources[iEngId] = pSrc;

		// Create an engine
		CreateEngine( iEngId );

		pEngine		= GetEngine( iEngId );
		//it can possible to open after source setup because need to get widht, height information
		if(pEngine){
			pEngine->SetVideoSource( pSrc );
			bOk = pEngine->Open();
		}

		if(bOk)
		{
			// Sync config
			CVCADataMgr *pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
			VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(iEngId);
			pVCADataMgr->SetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
			pVCADataMgr->SetAdvancedInfo( &(pEngCfg->AdvInfo) );
			pVCADataMgr->SetTamperInfo( &(pEngCfg->TamperInfo) );
			pVCADataMgr->SetSceneChangeInfo(&(pEngCfg->SceneChangeInfo));

			VCA5_APP_ENGINE_INFO *pInfo = CAPPConfigure::Instance()->GetAPPEngineInfo( iEngId );
			if( pInfo ){
				pVCADataMgr->SetEngineInfo( pInfo );
			}

			pVCADataMgr->Reset();
	
			m_pVCADialogs[iEngId]->SetEngine(pEngine);
			m_pVCADialogs[iEngId]->SetSrcID(iEngId);
			m_pVCADialogs[iEngId]->SetSourceStatus( CVCADialog::SS_OK );

			pEngine->RegisterVCAEventSink(m_pVCADialogs[iEngId]);
			
			m_AlarmListCtrl.SetVCADataMgr(pEngine->GetEngId(), pVCADataMgr);
			pEngine->RegisterVCAEventSink(&m_AlarmListCtrl);
			
			pVCADataMgr->FireEvent(IVCAConfigureObserver::VCA_LOAD, NULL);

			// 
			if(bOk != VCA5OPEN_FAIL) m_pVCADialogs[iEngId]->ApplyAllSetting();

			// And run it
			pEngine->Run();
		}
	}
	else
	{
		m_pVCADialogs[iEngId]->SetEngine( NULL );

		VCA5_APP_VIDEOSRC_INFO *pInfo = GetSrcInfo( iEngId );

		if( IVCAVideoSource::NOTSETSOURCE != pInfo->SourceType )
		{
			m_pVCADialogs[iEngId]->SetSourceStatus( CVCADialog::SS_ERROR );
		}
		else
		{
			m_pVCADialogs[iEngId]->SetSourceStatus( CVCADialog::SS_NOTASSIGNED );
			m_pVCADialogs[iEngId]->Invalidate();
		}
	}

	return bOk;
}

BOOL CAppDlg::TeardownEngine( int iEngId )
{
	if( m_pVCADialogs[iEngId] )
	{
		CEngine *pEngine = NULL;
		pEngine = m_pVCADialogs[iEngId]->GetEngine();

		if( pEngine ) pEngine->Stop();
	}

	if(m_pVideoSources[iEngId])
	{
		DestroyVCAVideoSource(m_pVideoSources[iEngId]);
		m_pVideoSources[iEngId] = NULL;
	}

	return DestroyEngine( iEngId );
}


void CAppDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	WindowLayout( cx, cy );
}

void CAppDlg::Split(SIZE &size)
{
	SIZE Unit;
	CRect Rect;
	ULONG x, y, height;
	
	// place VCA dialogs
	GetDlgItem(IDC_STATIC_DIALOG)->GetClientRect(&Rect);
	GetDlgItem(IDC_STATIC_DIALOG)->MapWindowPoints(this, (POINT *) &Rect.left, 2);
	Unit.cx = Rect.Width() / size.cx;
	Unit.cy = Rect.Height() / size.cy;

	for (ULONG i = 0; i < GetNumEngine(); ++i) {
		x = i % size.cx;
		y = i / size.cy;
		m_pVCADialogs[i]->SplitMove((int)Rect.left+x*Unit.cx, (int)Rect.top+y*Unit.cy, (int) Unit.cx, (int) Unit.cy);
		m_pVCADialogs[i]->Invalidate();
	}

	for( ULONG i = GetNumEngine(); i < (ULONG)(m_SplitConfig.cx * m_SplitConfig.cy); ++i ) {
		x = i % size.cx;
		y = i / size.cy;
		m_pBlankDialogs[i]->MoveWindow((int)Rect.left+(x*Unit.cx), (int)Rect.top+(y*Unit.cy), (int) Unit.cx, (int) Unit.cy);
		m_pBlankDialogs[i]->Invalidate();
	}

	// place alarm list controls
	GetDlgItem(IDC_STATIC_ALARM)->GetClientRect(&Rect);
	GetDlgItem(IDC_STATIC_ALARM)->MapWindowPoints(this, (POINT *) &Rect.left, 2);
	height = Rect.Height() / (size.cx*size.cy);

	//for (ULONG i = 0; i < m_VCASystem.GetNumEngine(); ++i) {
	m_AlarmListCtrl.MoveWindow(&Rect);
	m_AlarmListCtrl.ShowWindow(SW_SHOW);
	Rect.OffsetRect(0, height);
	//}

}

void CAppDlg::WindowLayout( int cx, int cy )
{
	CRect rcClient;
	GetClientRect(&rcClient);

	if( DISPLAY_PREVIEW == m_eDisplayMode )
	{
		BOOL bVCA_Dlg_enable=FALSE;

		// VCA Dlg Stretch
		if(GetDlgItem(IDC_STATIC_DIALOG)->GetSafeHwnd())
		{
			bVCA_Dlg_enable = TRUE;

			GetDlgItem(IDC_STATIC_DIALOG)->MoveWindow(rcClient.left + 5, rcClient.top + 5, cx - 8, int(cy*0.7)); 
			Split(m_SplitConfig);		
		}	

		// AlarmListCtrl Stretch
		if( GetDlgItem(IDC_STATIC_ALARM)->GetSafeHwnd() && bVCA_Dlg_enable)
		{
			CRect rect;
			GetDlgItem(IDC_STATIC_DIALOG)->GetClientRect(&rect);

			GetDlgItem(IDC_STATIC_ALARM)->MoveWindow(rcClient.left + 5, rect.bottom + 17, cx - 9, int(cy*0.27)); 
			m_AlarmListCtrl.MoveWindow(rcClient.left + 5, rect.bottom + 17, cx - 9, int(cy*0.27)); 
			m_AlarmListCtrl.Invalidate();	
		}
	}
	else
	{
		ASSERT( DISPLAY_CONFIG == m_eDisplayMode );

		m_ConfigDlg.MoveWindow( 0, 0, cx, cy );
	}

}

void CAppDlg::OnClose()
{
	m_CAP5System.Stop();

	for (ULONG i = 0; i < GetNumEngine(); i++ )
	{		
		DestroyEngine( i );
		m_pVCADialogs[i]->SetEngine( NULL );		
	}

	m_CAP5System.Endup();

	m_AlarmListCtrl.SetImageList(NULL, LVSIL_SMALL); 
	DestroyWindow();//PostMessage( WM_DESTROY );
}

void CAppDlg::OnDestroy()
{
	PostQuitMessage(0);
}

HCURSOR CAppDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAppDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

LRESULT CAppDlg::OnConfigMode( WPARAM wParam, LPARAM lParam )
{
	// One of the windows has requested config mode.
	// Switch off all others and enable the config dialog
	int iEngId = wParam;
	int iEnable = lParam;

	// If disabling, hide config window now
	if( !iEnable )
	{
		// Check if it's dirty
		if( m_ConfigDlg.IsDirty() )
		{
			if( IDNO == AfxMessageBox(_T("WARNING!\nThe configuration has been changed but not applied.\nDo you really want to exit without applying the configuration?"),
				MB_ICONQUESTION | MB_YESNO ) )
			{
				// Not ready yet
				return 0;
			}
		}

		m_ConfigDlg.ShowWindow( SW_HIDE );
		m_ConfigDlg.SetVCADialog( NULL );
	}

	// Hide or show all open dialogs, switch to config dialog
	for( int i = 0; i < VCA5_MAX_NUM_ENGINE; i++ )
	{
		if( m_pVCADialogs[i] )
		{
			if( !iEnable )
			{
				m_pVCADialogs[i]->SetParent( this );
			}

			m_pVCADialogs[i]->ShowWindow( iEnable ? SW_HIDE : SW_SHOW );
		}

		if( m_pBlankDialogs[i] )
		{
			m_pBlankDialogs[i]->ShowWindow( iEnable ? SW_HIDE : SW_SHOW );
		}
	}

	m_AlarmListCtrl.ShowWindow( iEnable ? SW_HIDE : SW_SHOW );

	CVCADataMgr *pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();

	// Reset all config to that stored by the app before we do this
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(iEngId);
	pVCADataMgr->SetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
	pVCADataMgr->SetAdvancedInfo( &(pEngCfg->AdvInfo) );
	pVCADataMgr->SetTamperInfo( &(pEngCfg->TamperInfo) );
	pVCADataMgr->SetSceneChangeInfo(&(pEngCfg->SceneChangeInfo));

	VCA5_APP_ENGINE_INFO *pInfo = CAPPConfigure::Instance()->GetAPPEngineInfo( iEngId );
	if( pInfo )
	{
		pVCADataMgr->SetEngineInfo( pInfo );
	}


	if( iEnable )
	{
		// Assign the right VCA dialog to the config dialog, and then show it
		m_ConfigDlg.SetVCADialog( m_pVCADialogs[iEngId] );

		// We allow all to be displayed - delegate the decision to CConfigDlg
//		m_ConfigDlg.SetTabMask( CConfigDlg::CONFIG_ALL );
		m_ConfigDlg.ShowWindow( SW_SHOW );
	}

	pVCADataMgr->Reset();
	m_eDisplayMode = iEnable ? DISPLAY_CONFIG : DISPLAY_PREVIEW;

	// Update sizing
	CRect rcClient;
	GetClientRect( &rcClient );
	WindowLayout( rcClient.Width(), rcClient.Height() );

	return 0;
}

LRESULT CAppDlg::OnConfigUpdated( WPARAM wParam, LPARAM lParam )
{
	// wParam = engine id
	int iEngId = (int)wParam;

	CVCADataMgr *pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
	VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(iEngId);
	pVCADataMgr->SetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
	pVCADataMgr->SetAdvancedInfo( &(pEngCfg->AdvInfo) );
	pVCADataMgr->SetTamperInfo( &(pEngCfg->TamperInfo) );
	pVCADataMgr->SetSceneChangeInfo(&(pEngCfg->SceneChangeInfo));

	VCA5_APP_ENGINE_INFO *pInfo = CAPPConfigure::Instance()->GetAPPEngineInfo( iEngId );
	if( pInfo )
	{
		pVCADataMgr->SetEngineInfo( pInfo );
	}

	OnConfigUpdated( (int)wParam );

	return 0;
}

LRESULT CAppDlg::OnExportConfig( WPARAM wParam, LPARAM lParam )
{
	int iEngId = wParam;

	CFileDialog dlg( FALSE, 0, 0, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 
		_T("XML Files (*.xml)|*.xml|All Files(*.*)|*.*||"), this );

	if( IDOK == dlg.DoModal() )
	{
		CString sPath = dlg.GetPathName();
		CVCAConfigure *pVcaCfg = CVCAConfigure::Instance();
		
		sPath.MakeLower();

		if(sPath.Find(_T(".xml")) == -1){
			sPath.Append(_T(".xml"));
		}
		pVcaCfg->SaveEngine( iEngId, sPath.GetBuffer() );
	}

	return 0;
}

LRESULT CAppDlg::OnImportConfig( WPARAM wParam, LPARAM lParam )
{
	CVCAConfigure *pVcaCfg = CVCAConfigure::Instance();
	int iEngId = wParam;

	CFileDialog dlg( TRUE, 0, 0, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 
		_T("XML Files (*.xml)|*.xml|All Files(*.*)|*.*||"), this );

	if( IDOK == dlg.DoModal() )
	{
		CString sPath = dlg.GetPathName();

		if( SUCCEEDED( pVcaCfg->LoadEngine( iEngId, sPath.GetBuffer() ) ) )
		{
			CVCADataMgr *pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
			VCA5_APP_ENGINE_CONFIG *pEngCfg = CVCAConfigure::Instance()->GetEngineConf(iEngId);
			pVCADataMgr->SetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
			
			pVCADataMgr->SetAdvancedInfo( &(pEngCfg->AdvInfo) );
			pVCADataMgr->SetTamperInfo(&(pEngCfg->TamperInfo));
			pVCADataMgr->SetSceneChangeInfo(&(pEngCfg->SceneChangeInfo));

			m_pVCADialogs[iEngId]->ApplyAllSetting();

			OnConfigUpdated( iEngId );
		}
		else
		{
			CString sMsg;
			sMsg.Format( _T("Unable to load configuration file %s"), sPath );
			AfxMessageBox( sMsg );
		}
	}

	return 0;
}

LRESULT CAppDlg::OnVideoSrcChanged( WPARAM wParam, LPARAM lParam )
{
	// Video source changed
	// wParam - engine id
	int iEngId = wParam;

	// Just restart - config will have changed and this will get picked up next time
	// around

	m_CAP5System.Stop();
//	m_VCASystem.Stop( iEngId );

	TeardownEngine( iEngId );
	SetupEngine( iEngId, FALSE );

	//if source video format change, other source format be changed.
	VCA5_APP_VIDEOSRC_INFO *pInfo = GetSrcInfo( iEngId );

	if(IVCAVideoSource::CAP5SOURCE == pInfo->SourceType)
	{
		VCA5_APP_VIDEOSRC_INFO* pTempInfo;
		for(DWORD i = 0 ; i < GetNumEngine() ; i++)
		{
			pTempInfo = GetSrcInfo( i );
			if((IVCAVideoSource::CAP5SOURCE == pTempInfo->SourceType) && (i != iEngId))
			{
				if(pTempInfo->ulVideoFormat != pInfo->ulVideoFormat)
				{
					pTempInfo->ulVideoFormat = pInfo->ulVideoFormat;
					pTempInfo->ulImageSize = (pInfo->ulVideoFormat == VCA5_VIDEO_FORMAT_NTSC_M)?
						VCA5_MAKEIMGSIZE(720,480):VCA5_MAKEIMGSIZE(720,576);
					
//					m_VCASystem.Stop( i );
					TeardownEngine( i );
					SetupEngine( i, FALSE );
//					m_VCASystem.Run( i );
				}
			}
		}
	}

//	m_VCASystem.Run( iEngId );

	m_CAP5System.Run();

	return 0;
}

