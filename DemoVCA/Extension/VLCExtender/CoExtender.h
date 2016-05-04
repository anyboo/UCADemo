#pragma once

#import "msxml3.dll"


#include "VCA5.h"

#include <initguid.h>
#include "VideoStreamer.h"
#include "..\vcaconfig.h"

#define MAX_LICENSE_LEN 1024




class CoExtender :
	public IVCADataSource, public IVideoStreamCallback
{
public:
	CoExtender(void);
	~CoExtender(void);

	// IUnknown
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface( const IID &riid, void **ppvObject );

	// IVCADataSource
	STDMETHODIMP OnConnected( IVCAConfig *pConfig );
	STDMETHODIMP OnDisconnected( IVCAConfig *pConfig );
	STDMETHODIMP GetNumChannels( int &iNumChannels );
	STDMETHODIMP GetChannelSource( int iChannel, VCACONFIG_CHANNEL_SRC *pSrc );
	STDMETHODIMP StartStream( int iChannel );
	STDMETHODIMP StopStream( int iChannel );
	STDMETHODIMP GetConfig( int iChannel, char *pszConfig, int iMaxSize, int &iRetSize );
	STDMETHODIMP OnConfigUpdated( int iChannel, char *pszConfig, int iLen );

	// IVideoStreamCallback
	STDMETHODIMP OnNewFrame( BITMAPINFOHEADER *pBih, unsigned char *pBuf, unsigned int uiLen, __int64 iTimestamp );
	STDMETHODIMP OnStreamEnd( );

protected:
	BOOL LoadConfig();
	BOOL SaveConfig();
	BOOL ParseSettings();

protected:

	enum VIDEO_TYPE
	{
		VIDEO_NULL,
		VIDEO_DSHOW,
		VIDEO_VLC,
		VIDEO_CAP5
	};

	IVCAConfig	*m_pVCAConfig;

	BOOL				m_bVideo;
	BOOL				m_bMeta;

	VCA5				m_VCA5;
	unsigned char		*m_pMetaBuf;
	char				*m_pConfigBuf;
	unsigned int		m_uiConfigLen;

	IVideoStreamer		*m_pStreamer;

	CString				m_sConfigFile;
	CString				m_sVideoData;
	char				m_sLicense[MAX_LICENSE_LEN];
	VIDEO_TYPE			m_eVideoType;

	MSXML2::IXMLDOMDocument2Ptr	m_pDOMDoc;

private:
	ULONG	m_refCount;
};
