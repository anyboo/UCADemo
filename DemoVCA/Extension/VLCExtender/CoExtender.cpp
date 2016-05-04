#include "StdAfx.h"
#include "CoExtender.h"

#include "../VCAMediaBundle.h"
#include "VLCStreamer.h"
#include "DShowStreamer.h"
#include "CAP5Streamer.h"


#define METABUF_SIZE 1024*1024
#define CONFIGBUF_SIZE 1024*1024

#define SETTINGS_FILE _T("extender.xml")


//-----------------------------------------------------------------------------

extern UINT g_objCount;

CoExtender::CoExtender(void)
{
	// COM
	m_refCount = 0;
	g_objCount++;

	m_pVCAConfig		= NULL;

	m_eVideoType		= VIDEO_NULL;
	m_pStreamer			= NULL;

	m_pMetaBuf = new unsigned char[METABUF_SIZE];
	m_pConfigBuf = new char[ CONFIGBUF_SIZE ];

	memset( m_pMetaBuf, 0, METABUF_SIZE );
	memset( m_pConfigBuf, 0, CONFIGBUF_SIZE );
	memset( m_sLicense, 0, MAX_LICENSE_LEN );

	m_bVideo = FALSE;
	m_bMeta = FALSE;

	m_uiConfigLen = 0;

	// Set up the DOM
	HRESULT hr;
	hr = m_pDOMDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if(S_OK != hr)
	{
		MessageBox(NULL, _T("Can not Create XMLDOMDocument Install MSXML4.0"), _T("ERROR") ,MB_OK | MB_ICONERROR);
	}

	ParseSettings();

	LoadConfig();

	switch( m_eVideoType )
	{
		case VIDEO_DSHOW:	m_pStreamer = new DShowStreamer( m_sVideoData );	break;
		case VIDEO_VLC:		m_pStreamer = new VLCStreamer( );					break;
		case VIDEO_CAP5:	m_pStreamer = new CAP5Streamer( m_sVideoData );		break;
		default: ASSERT( FALSE );
	}

	if( VIDEO_CAP5 == m_eVideoType && !m_bVideo )
	{
		AfxMessageBox( _T("Since CAP5 cannot be accessed from Extender DLL and ConfigApp simultaneously, both video and metadata must be generated in Extender DLL.\nThis setting will be updated internally (both video and metadata will be generated inside Extender DLL and not in ConfigApp).") );
		m_bVideo = TRUE;
	}
}

CoExtender::~CoExtender(void)
{
	g_objCount--;

	delete m_pStreamer;
}

STDMETHODIMP_(ULONG) CoExtender::AddRef()
{
	return ++m_refCount;
}

STDMETHODIMP_(ULONG) CoExtender::Release()
{
	if( 0 == --m_refCount )
	{
		delete this;
		return 0;
	}

	return m_refCount;
}

STDMETHODIMP CoExtender::QueryInterface(const IID &riid, void **ppvObject)
{
	if( IID_IUnknown == riid )
	{
		*ppvObject = (IUnknown *)this;
	}
	else
	if( IID_IVCADataSource == riid )
	{
		*ppvObject = (IVCADataSource *)this;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	if( *ppvObject )
	{
		((IUnknown *)*ppvObject)->AddRef();
	}

	return S_OK;
}

BOOL CoExtender::ParseSettings()
{
	BOOL bOk = FALSE;
	VARIANT_BOOL vb;
	USES_CONVERSION;
	struct _stat64i32 statInfo;
	

	if( 0 == _tstat( SETTINGS_FILE, &statInfo ) )
	{
		vb = m_pDOMDoc->load( SETTINGS_FILE );
	}
	else
	{
		CString s;
		s.Format( _T("Unable to load settings file: %s"), SETTINGS_FILE );
		AfxMessageBox( s );
		return FALSE;
	}

	if( VARIANT_TRUE == vb )
	{
		// Parse config file location
		MSXML2::IXMLDOMNodePtr	pNode, pSubNode;

		BSTR bsTag = CComBSTR( "root/config_file" );

		pNode = m_pDOMDoc->selectSingleNode( (BSTR) bsTag );

		if( pNode )
		{
			m_sConfigFile = (char *)pNode->text;
		}
		else
		{
			AfxMessageBox( _T("Config file not specified in extender config XML file") );
		}

		// Video settings
		bsTag = CComBSTR( "root/video_src" );
			pNode = m_pDOMDoc->selectSingleNode( (BSTR) bsTag );
		if( pNode )
		{
			bsTag = CComBSTR( "type" );
			pSubNode = pNode->selectSingleNode( (BSTR) bsTag );

			if( pSubNode )
			{
				if( 0 == strcmp( pSubNode->text, "dshow" ) )	m_eVideoType = VIDEO_DSHOW;
				else if( 0 == strcmp( pSubNode->text, "vlc" ) )	m_eVideoType = VIDEO_VLC;
				else if( 0 == strcmp( pSubNode->text, "cap5" ) )m_eVideoType = VIDEO_CAP5;
				else m_eVideoType = VIDEO_NULL;
			}

			bsTag = CComBSTR( "data" );
			pSubNode = pNode->selectSingleNode( (BSTR) bsTag );
			if( pSubNode )
			{
				m_sVideoData = (char *) pSubNode->text;
			}


			bsTag = CComBSTR( "stream_video" );
			pSubNode = pNode->selectSingleNode( (BSTR) bsTag );
			if( pSubNode )
			{
				int i = atoi( pSubNode->text );
				m_bVideo = i > 0 ? TRUE : FALSE;
			}

			bsTag = CComBSTR( "stream_meta" );
			pSubNode = pNode->selectSingleNode( (BSTR) bsTag );
			if( pSubNode )
			{
				int i = atoi( pSubNode->text );
				m_bMeta = i > 0 ? TRUE : FALSE;
			}
		}

		// local VCA settings
		bsTag = CComBSTR( "root/local_vca" );
		pNode = m_pDOMDoc->selectSingleNode( (BSTR) bsTag );
		if( pNode )
		{
			bsTag = CComBSTR( "license" );
			pSubNode = pNode->selectSingleNode( (BSTR) bsTag );

			if( pSubNode )
			{
				strcpy_s( m_sLicense, _countof( m_sLicense ), T2A( pSubNode->text ) );
			}
		}

		bOk = TRUE;

	}
	else
	{
		// Loading the doc failed, find out why...
		MSXML2::IXMLDOMParseErrorPtr pErr = m_pDOMDoc->parseError;

		CString strLine, sResult;
		strLine.Format(_T(" ( line %u, column %u )"), pErr->Getline(), pErr->Getlinepos());
		// Return validation results in message to the user.
		if (pErr->errorCode != S_OK)
		{
			sResult = CString("Validation failed on ") +
			 CString ("\n=====================") +
			 CString("\nReason: ") + CString( (char*)(pErr->Getreason())) +
			 CString("\nSource: ") + CString( (char*)(pErr->GetsrcText())) +
			 strLine + CString("\n");

			ATLTRACE(sResult);

			AfxMessageBox( sResult );


		}
	}

	return bOk;

}

STDMETHODIMP CoExtender::OnConnected( IVCAConfig *pConfig )
{
	HRESULT hr = E_FAIL;

	m_pVCAConfig = pConfig;

	// Open up VCA5 engine
	if( !m_VCA5.Open( m_sLicense ) )
	{
		return hr;
	}

	if( m_uiConfigLen )
	{
		// Apply the configuration
		m_VCA5.SetConfig( m_pConfigBuf, m_uiConfigLen );
	}
	else
	{
		// There is no config - this must be a fresh install
		// So query local VCA5 for its config - it will provide defaults
		// and details of what license is loaded - this is important since the license
		// dictates which features are available for configuration in the config GUI.
		unsigned int uiLen = CONFIGBUF_SIZE;
		BOOL fOk = m_VCA5.GetConfig( m_pConfigBuf, &uiLen );
		if( fOk )
		{
			// TBD - fix this so it comes back from VCA5
			m_uiConfigLen = uiLen;
		}
	}

	hr = S_OK;

	return hr;
}

STDMETHODIMP CoExtender::OnDisconnected( IVCAConfig *pConfig )
{

	m_pVCAConfig = NULL;

	// Close VCA
	m_VCA5.Close();

	return S_OK;
}

STDMETHODIMP CoExtender::GetNumChannels( int &iNumChannels )
{
	// We only support 1 channel
	if( m_pStreamer )
	{
		iNumChannels = 1;
	}
	else
	{
		iNumChannels = 0;
	}

	return S_OK;
}

STDMETHODIMP CoExtender::GetChannelSource( int iChannel, VCACONFIG_CHANNEL_SRC *pSrc)
{
	// Tell them about the sources we support
	USES_CONVERSION;
	memset( pSrc, 0, sizeof( VCACONFIG_CHANNEL_SRC ) );

	ASSERT( iChannel == 0 );

	pSrc->iChannelId		= 0;

	if( m_bVideo )
	{
		// We provide video, so we tell them we'll stream through the IF
		pSrc->videoSrcType	= VCACONFIG_SRC_IF;
	}
	else
	{
		// We want them to provide the video...

		switch( m_eVideoType )
		{
			case VIDEO_DSHOW:	pSrc->videoSrcType	= VCACONFIG_SRC_DSHOW;	break;
			case VIDEO_VLC:		pSrc->videoSrcType	= VCACONFIG_SRC_VLC;	break;
			default:			pSrc->videoSrcType	= VCACONFIG_SRC_NULL;	break;
		}
	}

	strcpy_s( pSrc->szVideoData, _countof( pSrc->szVideoData ), T2A(m_sVideoData.GetBuffer(0)) );
	pSrc->fMediaTypeFlags	= (m_bMeta & VCACONFIG_MEDIA_METADATA) | (m_bVideo & VCACONFIG_MEDIA_VIDEO);	// Provides just meta

	return S_OK;
}

STDMETHODIMP CoExtender::StartStream(int iChannel)
{
	HRESULT hr = E_FAIL;

	if( m_pStreamer )
	{
		hr = m_pStreamer->Open( this );
	}

	return hr;
}

STDMETHODIMP CoExtender::StopStream(int iChannel)
{
	HRESULT hr = E_FAIL;

	hr = m_pStreamer->Close();



	return hr;
}

STDMETHODIMP CoExtender::GetConfig(int iChannel, char *pszConfig, int iMaxSize, int &iRetSize)
{
	HRESULT hr = S_OK;

	// Load the config from fixed location
	memcpy( pszConfig, m_pConfigBuf, min( (unsigned int)iMaxSize, m_uiConfigLen ) );
	iRetSize = min( (unsigned int)iMaxSize, m_uiConfigLen );

	return hr;
}

STDMETHODIMP CoExtender::OnConfigUpdated(int iChannel, char *pszConfig, int iLen)
{
	// Update local buffer

	m_uiConfigLen = min( iLen, CONFIGBUF_SIZE );

	memcpy( m_pConfigBuf, pszConfig, m_uiConfigLen);

	// Save it
	SaveConfig();

	// Re-apply to local VCA5

	m_VCA5.SetConfig( m_pConfigBuf, m_uiConfigLen );
	return S_OK;
}

BOOL CoExtender::LoadConfig()
{
	BOOL fOk = FALSE;

	// Load the config from fixed location
	FILE *pFile;

	if( 0 == _tfopen_s( &pFile, m_sConfigFile.GetBuffer(0), _T("r") ) )
	{
		unsigned int uiFileSize, uiReadSize;

		fseek( pFile, 0, SEEK_END );
		uiFileSize = ftell( pFile );
		fseek( pFile, 0, SEEK_SET );

		if( uiFileSize <= (unsigned int )CONFIGBUF_SIZE )
		{
			uiReadSize = (unsigned int)fread( m_pConfigBuf, 1, uiFileSize, pFile );

			m_uiConfigLen = uiReadSize;

			m_pConfigBuf[uiReadSize] = '\0';
			uiReadSize++;

			fOk = TRUE;
		}

		fclose( pFile );
	}

	return fOk;
}

BOOL CoExtender::SaveConfig()
{
	BOOL fOk = FALSE;
	FILE *pFile;

	if( 0 == _tfopen_s( &pFile, m_sConfigFile.GetBuffer(0), _T("w") ) )
	{
		if( 1 == fwrite( m_pConfigBuf, m_uiConfigLen, 1, pFile ) )
		{
			fOk = TRUE;
		}
		else
		{
			ASSERT( FALSE );
		}

		fclose( pFile );
	}

	return fOk;
}


//-----------------------------------------------------------------------------
// IVideoStreamCallback

STDMETHODIMP CoExtender::OnNewFrame(BITMAPINFOHEADER *pBih, unsigned char *pBuf, unsigned int uiLen, __int64 iTimestamp )
{
	// Create a MediaSample to store this
	IVCAMediaSample *pSamp = 0;
	IVCAMediaSample	*pMetaSamp = 0;
	IVCAMediaBundle *pBun = 0;

	pSamp = VCAMediaSample::CreateMediaSample( );

	HRESULT hr = pSamp->Create( uiLen );
	unsigned char *pData = 0;
	int iDataLen = 0;

	if( SUCCEEDED( hr ) )
	{
		// Get the dataptr
		hr = pSamp->GetDataPtr( (void **)&pData, iDataLen );
	}

	if( SUCCEEDED( hr ) )
	{
		if( m_bVideo )
		{
			// copy the data in
			memcpy( pData, pBuf, uiLen );

			// Set up the header and set that
			VCACONFIG_MEDIASAMPLEHDR hdr;
			memset( &hdr, 0, sizeof( VCACONFIG_MEDIASAMPLEHDR ) );

			hdr.mediaType = VCACONFIG_MEDIA_VIDEO;
			hdr.mediaHdr.videoHdr.bmih	= *pBih;

			hdr.timeStamp = iTimestamp;

			hr = pSamp->SetHeader( hdr );
		}
	}

	if( SUCCEEDED( hr ) )
	{
		if( m_bMeta )
		{
			// Generate some metadata (we do this by passing through a local VCA5 library). In practice it's likely
			// the data would come from elsewhere (e.g. archived).
			unsigned int uiLen = METABUF_SIZE;
			if( m_VCA5.Process( pBuf, pBih, m_pMetaBuf, &uiLen ) )
			{
				pMetaSamp = VCAMediaSample::CreateMediaSample();
				pMetaSamp->Create( uiLen );

				if( SUCCEEDED( pMetaSamp->GetDataPtr( (void**)&pData, iDataLen ) ) )
				{
					memcpy( pData, m_pMetaBuf, min( uiLen, (unsigned int)iDataLen ) );
				}

				// Set the header
				VCACONFIG_MEDIASAMPLEHDR hdr;
				memset( &hdr, 0, sizeof( VCACONFIG_MEDIASAMPLEHDR ) );

				hdr.mediaType = VCACONFIG_MEDIA_METADATA;

				hdr.timeStamp = iTimestamp;
				
				hr = pMetaSamp->SetHeader( hdr );

				ASSERT( SUCCEEDED( hr ) );
			}
		}
	}

	if( SUCCEEDED( hr ) )
	{
		// Create the bundle
		pBun = VCAMediaBundle::CreateBundle();

		if( m_bVideo )
		{
			hr = pBun->AddSample( pSamp );
		}

		if( m_bMeta )
		{
			hr = pBun->AddSample( pMetaSamp );
		}
	}

	if( SUCCEEDED( hr ) )
	{
		// Pass the bundle in...
		m_pVCAConfig->DeliverMedia( (IVCADataSource *)this, 0, pBun );
	}

	if( pBun )
	{
		pBun->Release();
	}

	if( pSamp )
	{
		pSamp->Release();
	}

	if( pMetaSamp )
	{
		pMetaSamp->Release();
	}

	return S_OK;
}

STDMETHODIMP CoExtender::OnStreamEnd()
{
	return S_OK;
}
