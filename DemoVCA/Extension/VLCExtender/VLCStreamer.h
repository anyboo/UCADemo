#pragma once
#include "..\..\VideoSource\AviDemux.h"
#include "VideoStreamer.h"

class VLCStreamer : public IVideoStreamer, public AviDemux, public AviDemuxCallback
{
public:
	VLCStreamer(void);
	~VLCStreamer(void);

protected:
	HRESULT Open( IVideoStreamCallback *pCallback );
	HRESULT Close();
	HRESULT StartVLCThread();
	HRESULT StopVLCThread();

	// AviDemuxCallback
	virtual void OnMainHeader( AVIMAINHEADER &avimh, void *pUserData );
	virtual void OnStreamHeader( AVISTREAMHEADER &avish, void *pUserData );
	virtual void OnStreamFormat( unsigned char *pFmtBuf, unsigned int uiLen, void *pUserData );
	virtual void OnNewFrame( DWORD dwFourCC, unsigned char *pBuf, unsigned int uiLen, void *pUserData );
	virtual void OnStreamEnd( void *pUserData);

	static UINT	StreamThreadStub( LPVOID lpThis );
	void StreamThread();

protected:
	BOOL	m_bGotMainHeader;
	BOOL	m_bOpen;
	BOOL	m_bRun;
	BOOL	m_bGotVideoFrame;
	BOOL	m_bVideoFmtComing;

	PROCESS_INFORMATION	m_piStreamer;
	
	HANDLE	m_hChildStdOut_R;
	HANDLE	m_hChildStdOut_W;
	HANDLE	m_hThreadEndedEvent;
	

	AVIMAINHEADER		*m_pAvimh;
	AVISTREAMHEADER		*m_pAvish;
	BITMAPINFOHEADER	*m_pAvibih;


	__int64				m_iTimestamp;

	IVideoStreamCallback	*m_pCallback;
};
