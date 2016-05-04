#pragma once
#include "videostreamer.h"
#include <Cap5BoardLibEx.h>

typedef struct {
	DWORD hwId;
	BYTE activationCode[16];
} DVRAUTHINFO;

class CAP5Streamer :
	public IVideoStreamer
{
public:
	CAP5Streamer( CString sVideoData );
	~CAP5Streamer(void);

protected:
	HRESULT Open( IVideoStreamCallback *pCallback );
	HRESULT Close();
	BOOL Uda5CreateInstance(HMODULE hLib,REFIID riid,void ** ppInterface);
	BOOL InitCapApi(LPCTSTR lpszDllName, DVRAUTHINFO AuthCode[], int nAuthCount);
	BOOL SetProperty(void);
	BOOL SetAdjust(void);
	static UINT MediaThreadStub( LPVOID lpThis );
	void MediaThread();
	BOOL ProcessCapData(CAP5_DATA_INFO* pCap5ImageInfo);


protected:
	IVideoStreamCallback *m_pCallback;

	ICap5		*m_pCapApi;

	HANDLE		m_hStopThreadEvent;
	HANDLE		m_hThreadStoppedEvent;

	CAP5_BOARD_INFO*	m_pCapBoardInfo;
	int					m_nCapBoards;

	int			m_bd, m_ch;
	int			m_videoStd;
	int			m_iFrameRate;
	int			m_iVideoWidth, m_iVideoHeight;
};
