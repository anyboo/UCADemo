#include "StdAfx.h"
#include "VCA5.h"

#define VCA5_LIB		"VCA5Lib.dll"
#define VCA5_LICENSE	"license.lic"

#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

VCA5::VCA5(void)
{
	m_pVCA5 = 0;
	m_fVCAOpened = FALSE;

	memset( &m_bih, 0, sizeof( BITMAPINFOHEADER ) );
	memset( &m_systemInfo, 0, sizeof( VCA5_SYSTEM_INFO ) );

	m_pConfig = NULL;
	m_uiConfigLen = 0;

	m_fConfigUpdated = FALSE;
}

VCA5::~VCA5(void)
{
}

BOOL VCA5::Open( char *pszLicense )
{
	USES_CONVERSION;
	HMODULE hLib;
	BOOL fOk = FALSE;
	CString sErr = _T("All OK, thanks");

	do
	{
		hLib = LoadLibrary( _T(VCA5_LIB) );

		if( !hLib )
		{
			sErr.Format( _T("Unable to load library %s"), _T(VCA5_LIB) );
			break;
		}

		// Create an instance...
		BOOL rs;
		IUnknown* pUnknown;
		BOOL (FAR WINAPI*_CreateInstance)(IUnknown ** ppInterface);
		FARPROC test_proc=GetProcAddress(hLib,"VCA5CreateInstance");
		if( test_proc )
		{
			*(FARPROC*)&_CreateInstance = test_proc;
			rs = (*_CreateInstance)((IUnknown **)&pUnknown);
			if( rs )
			{
				HRESULT hr;
				hr = pUnknown->QueryInterface( IID_IVCA5, (void **)&m_pVCA5 );
				pUnknown->Release();
				if( FAILED(hr) )
				{
					sErr = _T("Unable to create VCA5 instance");
					break;
				}
			}
		}

		/*
		// Read the license
		char pszLicense[1024];
		memset( pszLicense, 0, sizeof( pszLicense ) );
		unsigned int uiLicenseLen = 1024;

		if( !ReadLicense( VCA5_LICENSE, pszLicense, &uiLicenseLen ) )
		{
			sErr.Format(_T("Unable to load license %s"), _T(VCA5_LICENSE) );
			break;
		}
	*/
		// Activate the VCA library
		VCA5_LICENSE_INFO	licenseInfo;
		licenseInfo.szLicense	= pszLicense;

		if( !m_pVCA5->VCA5Activate( 1, &licenseInfo ) )
		{
			sErr = _T("Unable to activate VCA5");
			break;
		}

		// Check we have at least one supported channel
		if( !m_pVCA5->VCA5GetSystemInfo( &m_systemInfo ) )
		{
			sErr = _T("Unable to get system info");
			break;
		}

		if( m_systemInfo.ulNumOfEngine[0] < 1 )
		{
			sErr = _T("No engines available in this license");
			break;
		}

		// Open with some nominal parameters
		VCA5_ENGINE_PARAMS vcaParams;
		vcaParams.ulColorFormat		= VCA5_COLOR_FORMAT_YUY2;
		vcaParams.ulVideoFormat		= VCA5_VIDEO_FORMAT_PAL_B;
		vcaParams.ulFrameRate100	= 3000;
		vcaParams.ulImageSize		= VCA5_MAKEIMGSIZE( 704, 576 );

		if( !m_pVCA5->VCA5Open( 0, &vcaParams ) )
		{
			// Failed!!
			ASSERT( FALSE );
			break;
		}

		fOk = TRUE;
	}
	while( 0 );

	if( !fOk )
	{
		CString sMsg;
		sMsg.Format( _T("Unable to open a local instance of VCA5.\n\nA local instance of VCA5 is required in order to generate metadata to overlay on the video and simulate a metadata source from elsewhere.\n\nReason: %s."),
			sErr );
		::MessageBox(AfxGetMainWnd()->m_hWnd, sMsg, _T("CoVLCExtender"), MB_ICONERROR | MB_OK );
	}

	return fOk;
}

BOOL VCA5::Close()
{
	if( m_fVCAOpened )
	{
		m_pVCA5->VCA5Close( 0 );

		m_uiConfigLen = 0;
		m_pConfig = NULL;

		m_fConfigUpdated = FALSE;
	}

	return TRUE;
}


BOOL VCA5::CheckFormatChange( BITMAPINFOHEADER *pBih )
{
	BOOL fChanged = FALSE;
	// Check to see if the new BIH is different from the one we've saved

	if( pBih->biWidth != m_bih.biWidth || pBih->biHeight != m_bih.biHeight ||
		pBih->biCompression != m_bih.biCompression || pBih->biBitCount != m_bih.biBitCount )
	{
		// Changed
		fChanged = TRUE;

		if( m_fVCAOpened )
		{
			m_pVCA5->VCA5Close( 0 );
		}

		unsigned long vcaColorType = 0;

		switch( pBih->biCompression )
		{
			case BI_RGB:
			{
			switch( pBih->biBitCount )
				case 24:
					vcaColorType = VCA5_COLOR_FORMAT_RGB24;
					break;
				case 16:
					vcaColorType = VCA5_COLOR_FORMAT_RGB16;
					break;
			}
			break;

			case FCC('YUY2'):
				vcaColorType = VCA5_COLOR_FORMAT_YUY2;
				break;
			case FCC('YV12'):
				vcaColorType = VCA5_COLOR_FORMAT_YV12;
				break;
			case FCC('UYVY'):
				vcaColorType = VCA5_COLOR_FORMAT_UYVY;
				break;

			default:
				ASSERT( FALSE );	// Not supported!
			break;
		}


		VCA5_ENGINE_PARAMS vcaParams;
		vcaParams.ulColorFormat		= vcaColorType;
		vcaParams.ulVideoFormat		= VCA5_VIDEO_FORMAT_PAL_B;
		vcaParams.ulFrameRate100	= 3000;
		vcaParams.ulImageSize		= VCA5_MAKEIMGSIZE( pBih->biWidth, pBih->biHeight );

		if( !m_pVCA5->VCA5Open( 0, &vcaParams ) )
		{
			// Failed!!
			ASSERT( FALSE );
		}

		// Also need to reapply the config
		m_fConfigUpdated = TRUE;
		m_fVCAOpened = TRUE;

		memcpy( &m_bih, pBih, sizeof( BITMAPINFOHEADER ) );
	}

	return fChanged;
}

BOOL VCA5::Process( unsigned char *pImage, BITMAPINFOHEADER *pBih, unsigned char *pResult, unsigned int *puiResultLen )
{
	CheckFormatChange( pBih );

	// Process the frame
	if( !m_fVCAOpened )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	// See if we need to update the config (keep the updates in the same thread as the process
	if( m_fConfigUpdated )
	{
		// Set the config if we have some
		if( m_pConfig && m_uiConfigLen )
		{
			VCA5_XMLCFG_PARAMS params;
			memset( &params, 0, sizeof( VCA5_XMLCFG_PARAMS ) );

			params.ulMedia			= VCA5_XMLCFG_BUFFER;
			params.ulCfgFlags		= VCA5_CFGFLAG_ALL;
			params.pszBufOrFilename	= m_pConfig;
			params.ulBufLen			= m_uiConfigLen;


			VERIFY( m_pVCA5->VCA5Control( 0, VCA5_CMD_LOADCFGXML, (ULONG)&params ) );
		}

		m_fConfigUpdated = FALSE;
	}

	VCA5_TIMESTAMP ts;
	GetSystemTimeAsFileTime( (LPFILETIME)&ts );
	FileTimeToLocalFileTime( (LPFILETIME)&ts, (LPFILETIME)&ts );

	BOOL fOk = m_pVCA5->VCA5Process( 0, pImage, &ts, (ULONG *)puiResultLen, pResult );

	return fOk;
}

BOOL VCA5::SetConfig( char *pszBuf, unsigned int uiBufLen )
{
	// Load the configuration from the specified buffer
	m_pConfig		= pszBuf;
	m_uiConfigLen	= uiBufLen;

	m_fConfigUpdated = TRUE;

	return TRUE;
}

BOOL VCA5::GetConfig( char *pszBuf, unsigned int *puiBufLen )
{
	// Get the config from VCA lib into the supplied bufs
	VCA5_XMLCFG_PARAMS params;
	memset( &params, 0, sizeof( VCA5_XMLCFG_PARAMS ) );

	params.ulMedia			= VCA5_XMLCFG_BUFFER;
	params.ulCfgFlags		= VCA5_CFGFLAG_ALL;
	params.pszBufOrFilename	= pszBuf;
	params.ulBufLen			= *puiBufLen;

	BOOL bOk = m_pVCA5->VCA5Control( 0, VCA5_CMD_SAVECFGXML, (ULONG)&params );

	*puiBufLen = params.ulBufLen;

	ASSERT( bOk );

	return bOk;
}