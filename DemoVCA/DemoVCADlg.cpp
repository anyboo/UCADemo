// DemoVCADlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "DemoVCADlg.h"
#include "ProgressDlg.h"
#include "PrjVersion.h"

#include "./VCADialog/BlankDialog.h"
#include "./VCADialog/VCADialog.h"
#include "./VideoSource/VCAVideoSource.h"
#include "./Recoder/AlarmOptDlg.h"
#include "./VCAEngine/VCAEngine.h"
#include "../Common/wm_user.h"
#include "../Common/EventFilter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemoVCADlg dialog


CDemoVCADlg::CDemoVCADlg(CWnd* pParent /*=NULL*/)
	: CAppDlg(pParent)
{
	m_pStartupThread = 0;
}

CDemoVCADlg::~CDemoVCADlg()
{
	Endup();

}

void CDemoVCADlg::DoDataExchange(CDataExchange* pDX)
{
	CAppDlg::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CDemoVCADlg, CAppDlg)
	//{{AFX_MSG_MAP(CDemoVCADlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_CONTROL_EXIT, &CDemoVCADlg::OnMenuControlExit)
	ON_COMMAND(ID_OPTION_RECORDING, &CDemoVCADlg::OnMenuOptionRecording)
	ON_COMMAND(ID_OPTION_SAVE_CURRENT_SETTING , &CDemoVCADlg::OnMenuOptionSaveCurSetting)
	ON_MESSAGE(WM_SHOW_WAIT_WND, &CDemoVCADlg::OnShowWaitWnd)

	// USER MESSAGES


	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void	CDemoVCADlg::UpdateMenuControl(BOOL bStart)
{
	CMenu *pMenu = GetMenu();
	if (!pMenu) return;

	pMenu->EnableMenuItem(ID_OPTION_RECORDING, MF_ENABLED);
	pMenu->EnableMenuItem(ID_OPTION_SAVE_CURRENT_SETTING, MF_ENABLED);
}

/////////////////////////////////////////////////////////////////////////////
// CDemoVCADlg message handlers

BOOL CDemoVCADlg::InitInstance()
{
	HRESULT hr;

	CoInitialize( NULL );

	CAPPConfigure *pApp = CAPPConfigure::Instance();

	if ('\0' == AfxGetApp()->m_lpCmdLine[0])
	{
		hr = pApp->Load(_T("DemoVCA.xml"));
	}
	else
	{
		hr = pApp->Load(AfxGetApp()->m_lpCmdLine);
	}

	if (FAILED(hr))
	{
		MessageBox(_T("Failed to load APP configuration"));
		return FALSE;
	}

	if(pApp->IsAlarmSaveEnabled() && pApp->IsEventExportEnabled())
	{
		CEventFilter *pEventFilter = CEventFilter::Instance();
		hr = pEventFilter->Load(pApp->GetEventFilterFile());
		if (FAILED(hr))
		{
			CString errMsg;
			errMsg.Format(_T("Failed to load %s"), pApp->GetEventFilterFile());
			MessageBox(errMsg);
			return FALSE;
		}
	}

	CVCAConfigure *pCfg = CVCAConfigure::Instance();
	
	pCfg->SetEngCnt( pApp->GetEngineCnt() );

	for( unsigned int i = 0; i < pApp->GetEngineCnt(); i++ )
	{
		VCA5_APP_ENGINE_INFO *pInfo = pApp->GetAPPEngineInfo(i);

		// Always call, even if we have invalid filename - it initializes defaults too
		if( S_OK != pCfg->LoadEngine( i, pInfo->szConfPath ) )
		{

			// If filename was 0, ignore error
			if( _tcslen( pInfo->szConfPath ) > 0 )
			{
				CString s;
				s.Format( _T("Unable to load configuration for engine %d from file %s"),
							 i, pInfo->szConfPath );
				AfxMessageBox( s );
			}
		}
	}

	return TRUE;
}

BOOL CDemoVCADlg::OnInitDialog()
{
	TCHAR	szWinodwsTitle[256];
	BOOL bSetupOk = FALSE;

	m_SplitConfig.cx = 2;
	m_SplitConfig.cy = 2;

	SendMessage( WM_SHOW_WAIT_WND, 1 );

	CAPPConfigure *pAppCfg = CAPPConfigure::Instance();

	bSetupOk = Setup();
	if( bSetupOk )
	{
		m_AlarmRecorder.SetOption(pAppCfg->IsAlarmSaveEnabled(),
							pAppCfg->GetAlarmSavePath(),
							pAppCfg->GetAlarmSavePeriod());

		CAppDlg::OnInitDialog();
		USES_CONVERSION;
		_stprintf_s(szWinodwsTitle, _countof(szWinodwsTitle), _T("VCA Demo APP Ver[%s]"), A2T(APP_RC_VERSION_STR));
		SetWindowText(szWinodwsTitle);

		UpdateMenuControl(TRUE);
		SendMessage( WM_SHOW_WAIT_WND, 0 );

		ShowWindow( SW_SHOW );
	}
	else
	{
		// Jump out straight away
		PostQuitMessage( 0 );
	}


	return TRUE;
}

LRESULT CDemoVCADlg::OnShowWaitWnd( WPARAM wParam, LPARAM lParam )
{
	int iShow = wParam;

	if( iShow )
	{
		m_pStartupThread = AfxBeginThread( RUNTIME_CLASS( CStartupWnd ) );
	}
	else
	{
		if( m_pStartupThread )
		{
			// Kill this to allow the other window to be shown properly
			m_pStartupThread->PostThreadMessageW( WM_QUIT, 0, 0 );
			m_pStartupThread = 0;
		}
	}

	SetForegroundWindow();

	return 0;
}

CEngine *CDemoVCADlg::GetEngine( int iEngId )
{
	return m_VCASystem.GetEngine( iEngId );
}

BOOL CDemoVCADlg::Setup()
{
	//Setup VCASystem
	if (!m_VCASystem.Setup()) {
		TRACE("Can not Setup VCASystem\n");
		return FALSE;
	}

	return TRUE;
}

VCA5_APP_VIDEOSRC_INFO *CDemoVCADlg::GetSrcInfo( int iEngId )
{
	VCA5_APP_ENGINE_INFO* pAppEngineInfo = CAPPConfigure::Instance()->GetAPPEngineInfo( iEngId );

	return &(pAppEngineInfo->tSourceData);
}

BOOL CDemoVCADlg::CreateEngine( int iEngId )
{
	BOOL bOk = FALSE;

	// Set up the engine
	CVCAEngine *pEngine;
	m_VCASystem.Open( iEngId );
	pEngine = m_VCASystem.GetEngine(iEngId);

	if(pEngine)
	{
		pEngine->RegisterVCAEventSink(&m_AlarmRecorder);
		CVCADataMgr *pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
		m_AlarmRecorder.SetVCADataMgr(pEngine->GetEngId(), pVCADataMgr);
		bOk = TRUE;
	}

	return bOk;
}

BOOL CDemoVCADlg::DestroyEngine( int iEngId )
{
	m_VCASystem.Stop( iEngId );
	m_VCASystem.Close( iEngId );
	return TRUE;
}

void CDemoVCADlg::Endup()
{
	SaveCurSetting();
	m_VCASystem.Endup();
}

BOOL CDemoVCADlg::SaveEngSetting( int iEngId, LPTSTR lpszFilename /*=NULL*/ )
{
	CVCADataMgr		*pVCADataMgr;
	VCA5_APP_ENGINE_CONFIG *pEngCfg;
	CVCAConfigure *pVcaCfg = CVCAConfigure::Instance();
	CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
	
	if(NULL == m_pVCADialogs[iEngId]) return FALSE;

	pVCADataMgr = m_pVCADialogs[iEngId]->GetVCADataMgr();
	pEngCfg		= pVcaCfg->GetEngineConf( iEngId );
	pVCADataMgr->GetData(&(pEngCfg->Zones), &(pEngCfg->Rules), &(pEngCfg->Counters), &(pEngCfg->CalibInfo), &(pEngCfg->ClsObjects));
	memcpy( &(pEngCfg->AdvInfo), pVCADataMgr->GetAdvancedInfo(), sizeof( VCA5_APP_ADVANCED_INFO ) );
	memcpy(&(pEngCfg->TamperInfo), pVCADataMgr->GetTamperInfo(), sizeof(VCA5_TAMPER_INFO));
	memcpy(&(pEngCfg->SceneChangeInfo), pVCADataMgr->GetSceneChangeInfo(), sizeof(VCA5_SCENECHANGE_INFO));

	VCA5_APP_ENGINE_INFO *pAppInfo = pAppCfg->GetAPPEngineInfo( iEngId );
	if( NULL == lpszFilename )
	{
		lpszFilename = pAppInfo->szConfPath;
	}

	pVcaCfg->SaveEngine( iEngId, lpszFilename );

	return TRUE;
}

BOOL CDemoVCADlg::SaveCurSetting()
{
	//1. Get VCAConfigure
	CVCADataMgr		*pVCADataMgr;
	CVCAConfigure *pVcaCfg = CVCAConfigure::Instance();
	CAPPConfigure *pAppCfg = CAPPConfigure::Instance();

	//DO not use VCASystem ENgine Number, becasue new VCASystem hold current opened vca engine count
	DWORD	nEngCount = pAppCfg->GetEngineCnt();
	for (ULONG eng = 0; eng < nEngCount ; eng++)
	{
		SaveEngSetting( eng );
	}


	//3. Save APPConfigure
	for (ULONG eng = 0; eng < nEngCount ; eng++)
	{
		if(m_pVCADialogs[eng]){
			pVCADataMgr = m_pVCADialogs[eng]->GetVCADataMgr();
			pAppCfg->SetAppEngineDisplayFlag(eng, pVCADataMgr->GetDisplayFlags());
			pVCADataMgr->FireEvent(IVCAConfigureObserver::VCA_SAVE, NULL);
		}
	}
	pAppCfg->Save();

	return TRUE;
}

void CDemoVCADlg::OnMenuControlExit()
{
	OnClose();
}

void CDemoVCADlg::OnMenuOptionRecording()
{
	CAlaramOptDlg Dlg;
	if (IDOK == Dlg.DoModal()) {
		m_AlarmRecorder.SetOption(CAPPConfigure::Instance()->IsAlarmSaveEnabled(),
			CAPPConfigure::Instance()->GetAlarmSavePath(), CAPPConfigure::Instance()->GetAlarmSavePeriod());
	}
}

void CDemoVCADlg::OnMenuOptionSaveCurSetting()
{
	SaveCurSetting();
}



