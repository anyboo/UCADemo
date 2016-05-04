#pragma once

class IVideoStreamCallback
{
public:
	STDMETHOD( OnNewFrame( BITMAPINFOHEADER *pBih, unsigned char *pBuf, unsigned int uiLen, __int64 iTimestamp ) ) PURE;
	STDMETHOD( OnStreamEnd( ) ) PURE;
};

class IVideoStreamer
{
public: 
	virtual HRESULT Open( IVideoStreamCallback *pCallback ) = 0;
	virtual HRESULT Close() = 0;

	virtual ~IVideoStreamer(){;}
};

