// ConfigAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoVCA.h"
#include "ConfigAppDlg.h"
#include "./VCAEngine/ConfigEngine.h"
#include "CustomCmdLineInfo.h"

#define MAX_CONFIG_SIZE 1024 * 1024


// CConfigAppDlg dialog

IMPLEMENT_DYNAMIC( CConfigAppDlg, CAppDlg)

CConfigAppDlg::CConfigAppDlg( CCustomCmdLineInfo *pInfo, CWnd* pParent /*=NULL*/)
	: CAppDlg( pParent)
{
	m_pVCADlg = NULL;
	m_pCmdLineInfo	= pInfo;
	m_uiRefCount = 0;
	m_pVCADataSrc = NULL;

	m_pszConfigBuf = new char[ MAX_CONFIG_SIZE ];
}

CConfigAppDlg::~CConfigAppDlg()
{
	// Disconnecting
	if( m_pVCADataSrc )
	{
		m_pVCADataSrc->OnDisconnected( this );

		m_pVCADataSrc->Release();
	}

	CoUninitialize();

	delete [] m_pszConfigBuf;
}

void CConfigAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CAppDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConfigAppDlg, CAppDlg)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------------
// COM bits

ULONG CConfigAppDlg::AddRef()
{
	return ++m_uiRefCount;
}

ULONG CConfigAppDlg::Release()
{
	--m_uiRefCount;

	if( 0 == m_uiRefCount )
	{
		// Time to go...
	}

	return m_uiRefCount;
}

HRESULT CConfigAppDlg::QueryInterface( const IID &riid, void **ppvObject )
{
	if( IID_IUnknown == riid )
	{
		*ppvObject = (IUnknown *)this;
	}
	else
	if( IID_IVCAConfig == riid )
	{
		*ppvObject = (IVCAConfig *)this;
	}

	if( *ppvObject )
	{
		((IUnknown *)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

// CConfigAppDlg message handlers

//---------------------------------------------------------------------------------

BOOL CConfigAppDlg::OnInitDialog()
{
	USES_CONVERSION;

	CoInitialize( NULL );

	m_SplitConfig.cx = 1;
	m_SplitConfig.cy = 1;


	//-----------------------------------------------
	// Now create client COM object
	UUID uuid;
	if( RPC_S_OK == UuidFromString((RPC_WSTR) m_pCmdLineInfo->GetGuid().GetBuffer(0), &uuid ) )
	{
		// Go ahead and cocreate it
		HRESULT hr = CoCreateInstance( uuid, NULL, CLSCTX_INPROC_SERVER, IID_IVCADataSource, (void**)&m_pVCADataSrc );

		if( SUCCEEDED( hr ) )
		{
			// Connect
			m_pVCADataSrc->OnConnected( (IVCAConfig *)this );

			// Find out what it supports
			int iNumChannels = 0;
			m_pVCADataSrc->GetNumChannels( iNumChannels );

			for( int i = 0; i < iNumChannels; i++ )
			{
				VCACONFIG_CHANNEL_SRC cSrc;
				memset( &cSrc, 0, sizeof( VCACONFIG_CHANNEL_SRC ) );

				m_pVCADataSrc->GetChannelSource( i, &cSrc );

				VCA5_APP_VIDEOSRC_INFO info;
				memset( &info, 0, sizeof( info ) );

				info.SourceType = IVCAVideoSource::NOTSETSOURCE;

				// Check the source, see what kinds of stuff it provides
//				if( cSrc.fMediaTypeFlags & VCACONFIG_MEDIA_VIDEO )
				{
					// Populate

					// Provides video data
					// See what kind of video source
					switch( cSrc.videoSrcType )
					{
						case VCACONFIG_SRC_IF:
						{
							info.SourceType	= IVCAVideoSource::VIRTUALSOURCE;
						}
						break;

						case VCACONFIG_SRC_DSHOW:
						{
							info.SourceType = IVCAVideoSource::DSHOWSOURCE;
							_tcscpy( info.szDShowURL, A2T( cSrc.szVideoData ) );
						}
						break;

						case VCACONFIG_SRC_CAP5:
						{
							// for now just use bd 0 and ch 0
							info.SourceType = IVCAVideoSource::CAP5SOURCE;
						}
						break;

						case VCACONFIG_SRC_VLC:
						{
							info.SourceType	= IVCAVideoSource::STREAMSOURCE;
						}
						break;

						default:
						{
							// not supported
							ASSERT( FALSE );
						}
					}
				}

				// Assume for now that it will always provide metadata

				m_srcInfo.insert( std::pair< int, VCA5_APP_VIDEOSRC_INFO > ((int)i, info ) );
			}

			// And start the hooked in client
		//	m_pVCADataSrc->StartStream( 0 );
			CAppDlg::OnInitDialog();
		}
		else
		{
			CString sErr;
			sErr.Format( _T("Unable to create 3PP module [%s]. Reason: 0x%x."), 
				m_pCmdLineInfo->GetGuid().GetBuffer(0), hr );
			AfxMessageBox( sErr, MB_OK | MB_ICONERROR );

			PostQuitMessage(0);
		}
	}

	// Update config dialog to not display video source tab
	m_ConfigDlg.SetTabMask( CConfigDlg::CONFIG_ALL & ~CConfigDlg::CONFIG_VIDEOSRC );

	SetWindowText( _T("Configuration Client") );


	return TRUE;
}

//---------------------------------------------------------------------------------

UINT CConfigAppDlg::GetNumEngine( )
{
	int iEngine = 0;

	if( m_pVCADataSrc )
	{
		m_pVCADataSrc->GetNumChannels( iEngine );
	}

	return (UINT)iEngine;
}

//---------------------------------------------------------------------------------

BOOL CConfigAppDlg::CreateEngine( int iEngId )
{
	BOOL fRet = FALSE;
	std::map< int, ConfigEngine *>::iterator it;

	it = m_engines.find( iEngId );

	if( it != m_engines.end() )
	{
		// Already created!
		ASSERT( FALSE );
		return FALSE;
	}
	
	fRet = LoadConfig( iEngId );

	// Create the engine anyway, even if we can't get the config
	ConfigEngine *pEngine = new ConfigEngine( iEngId );
	pEngine->SetDataSrc( m_pVCADataSrc );

	m_engines.insert( std::pair< int, ConfigEngine *>( iEngId, pEngine ) );

	if( !fRet )
	{
		CString sErr;
		sErr.Format( _T("Unable to load configuration for engine %d"), iEngId );
		AfxMessageBox( sErr, MB_OK | MB_ICONERROR );
	}

	return fRet;
}

//--------------------------------------------------------------------------------------

void CConfigAppDlg::OnConfigUpdated( int iEngId )
{
	// Something in the app has changes the config

	// Tell the data source
	CVCAConfigure *pCfg = CVCAConfigure::Instance();

	unsigned int uiBufLen = MAX_CONFIG_SIZE;
	if( SUCCEEDED( pCfg->SaveEngine( iEngId, m_pszConfigBuf, uiBufLen ) ) )
	{
		// Pass it through
		m_pVCADataSrc->OnConfigUpdated( iEngId, m_pszConfigBuf, uiBufLen );
	}
}

//--------------------------------------------------------------------------------------

BOOL CConfigAppDlg::LoadConfig( int iEngId )
{
	BOOL fRet = FALSE;

	// Before this engine is created, let's get the dude to load the config
	int iActualSize = 0;
	if( SUCCEEDED( m_pVCADataSrc->GetConfig( iEngId, m_pszConfigBuf, MAX_CONFIG_SIZE, iActualSize ) ) )
	{
		m_pszConfigBuf[iActualSize] = '\0';

		// Load it into the VCAConfigure
		if( SUCCEEDED( CVCAConfigure::Instance()->LoadEngine( iEngId, m_pszConfigBuf, iActualSize ) ) )
		{
			fRet = TRUE;
		}
	}

	return fRet;
}

//---------------------------------------------------------------------------------

BOOL CConfigAppDlg::DestroyEngine( int iEndId )
{
	std::map< int, ConfigEngine *>::iterator it = m_engines.find( iEndId );

	if( it != m_engines.end() )
	{
		it->second->Stop();
		delete it->second;

		m_engines.erase( it );

		return TRUE;
	}
	else
	{
		// Not there!
		ASSERT( FALSE );

		return FALSE;
	}
}

//---------------------------------------------------------------------------------

CEngine *CConfigAppDlg::GetEngine( int iEngId )
{
	std::map< int, ConfigEngine *>::iterator it = m_engines.find( iEngId );

	if( it != m_engines.end() )
	{
		return it->second;
	}

	return NULL;
}

//---------------------------------------------------------------------------------

VCA5_APP_VIDEOSRC_INFO *CConfigAppDlg::GetSrcInfo(int iEngId)
{
	std::map< int, VCA5_APP_VIDEOSRC_INFO >::iterator it = m_srcInfo.find( iEngId );

	if( it != m_srcInfo.end() )
	{
		return &(it->second);
	}

	ASSERT( FALSE );
	return NULL;
}

//----------------------------------------------------------------------------------

STDMETHODIMP CConfigAppDlg::OnConfigUpdated(IVCADataSource *pSrc, int iChannel )
{
	LoadConfig( iChannel );

	return S_OK;
}

//----------------------------------------------------------------------------------

STDMETHODIMP CConfigAppDlg::DeliverMedia(IVCADataSource *pSrc, int iChannel, IVCAMediaBundle *pBun)
{
	// Got some media delivered - pass it to VCA Dialog
	pBun->AddRef();

	// Go through each sample and deliver it to the right place
	ConfigEngine *pEngine = NULL;
	IVCAVideoSource *pVideoSource = NULL;

	if( (UINT)iChannel < m_engines.size() )
	{
		pEngine = m_engines[iChannel];
	}

	pVideoSource = pEngine->GetVideoSource();

	int iNumSamples = 0;
	pBun->GetNumSamples( iNumSamples );

	for( int i = 0; i < iNumSamples; i++ )
	{
		IVCAMediaSample *pSample = NULL;
		VCACONFIG_MEDIASAMPLEHDR hdr;
		HRESULT hr;
		
		hr = pBun->GetSamplePtr( i, &pSample );

		if( SUCCEEDED( hr ) )
		{
			hr = pSample->GetHeader( hdr );
		}

		if( SUCCEEDED( hr ) )
		{
			switch( hdr.mediaType )
			{
				case VCACONFIG_MEDIA_VIDEO:
				{
					// Video sample - give it to the virtual video source if there is one...
					if( pVideoSource )
					{
						pVideoSource->Control( IVCAVideoSource::CMD_ADD_MEDIASAMPLE, (DWORD)pSample, 0 );
					}
				}
				break;

				case VCACONFIG_MEDIA_METADATA:
				{
					// Metadata sample - give it directly to the engine if there is one...
					if( pEngine )
					{
						pEngine->AddMetadata( pSample );
					}
				}
				break;

				default:
				{
					// Erk??
					ASSERT( FALSE );
				}
			}
		}

		if( pSample )
		{
			// Done
			pSample->Release();
			pSample = NULL;
		}
	}

	// Done...
	pBun->Release();

	return S_OK;
}