#include "StdAfx.h"
#include "DShowStreamer.h"
#include "../../VideoSource/DShowCap.h"


DShowStreamer::DShowStreamer( LPCTSTR lpszURI )
{
	m_sURI = CString( lpszURI );

	m_pCap = new DShowCap();

	m_pCap->RegisterCallback( DShowStreamer::CapCallback, (void*) this );
}

DShowStreamer::~DShowStreamer(void)
{
	delete m_pCap;
}

HRESULT DShowStreamer::Open(IVideoStreamCallback *pCallback)
{
	m_pCallback = pCallback;

	HRESULT hr = m_pCap->StartURI( m_sURI );

	return hr;
}

HRESULT DShowStreamer::Close()
{
	HRESULT hr = m_pCap->StopCap();

	return hr;
}

// static
BOOL __stdcall DShowStreamer::CapCallback( BITMAPINFOHEADER &bih, unsigned char *pData, unsigned int uiLen, __int64 timeStamp, void *pUserData )
{
	((DShowStreamer *)pUserData)->Callback( &bih, pData, uiLen, timeStamp );

	return TRUE;
}


void DShowStreamer::Callback(BITMAPINFOHEADER *pBih, unsigned char *pData, unsigned int uiLen, __int64 timeStamp )
{
	ASSERT( m_pCallback );

	// TBD - timestamp
	m_pCallback->OnNewFrame( pBih, pData, uiLen, timeStamp );
}