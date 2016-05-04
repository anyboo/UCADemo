#pragma once

#include "VideoStreamer.h"

class DShowCap;
class DShowStreamer : public IVideoStreamer
{
public:
	DShowStreamer(LPCTSTR lpszURI);
	~DShowStreamer(void);

	HRESULT Open( IVideoStreamCallback *pCallback );
	HRESULT Close();

protected:
	static BOOL __stdcall CapCallback( BITMAPINFOHEADER &bih, unsigned char *pData, unsigned int uiLen, __int64 timeStamp, void *pUserData );
	void Callback( BITMAPINFOHEADER *pBih, unsigned char *pData, unsigned int uiLen, __int64 timeStamp );

protected:
	CString	m_sURI;

	IVideoStreamCallback *m_pCallback;

	DShowCap	*m_pCap;

};
